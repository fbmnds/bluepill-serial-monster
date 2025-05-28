#include <stddef.h>
#include "stm32f4xx.h"
#include "serial.h"

#define BUFFER_SIZE 100

volatile uint8_t usart1_rx_buf[BUFFER_SIZE];
volatile uint8_t usart2_rx_buf[BUFFER_SIZE];
volatile uint8_t usart6_rx_buf[BUFFER_SIZE];
volatile uint32_t usart1_rx_idx = 0;
volatile uint32_t usart2_rx_idx = 0;
volatile uint32_t usart6_rx_idx = 0;


const cdc_port_t port_config[USB_CDC_NUM_PORTS] = {
    { // USART1
        .usart = USART1,
        .tx_pin = {GPIOA, 9, GPIO_MODER_MODE2, GPIO_OSPEEDR_OSPEED3}, // AF, very high speed
        .rx_pin = {GPIOA, 10, GPIO_MODER_MODE2, GPIO_OSPEEDR_OSPEED3}, // AF
        .rts_pin = {GPIOA, 8, GPIO_MODER_MODE1, GPIO_OSPEEDR_OSPEED3}, // Output
        .cts_pin = {NULL, 0, 0, 0} // No CTS (USB conflict)
    },
    { // USART2
        .usart = USART2,
        .tx_pin = {GPIOA, 2, GPIO_MODER_MODE2, GPIO_OSPEEDR_OSPEED3},
        .rx_pin = {GPIOA, 3, GPIO_MODER_MODE2, GPIO_OSPEEDR_OSPEED3},
        .rts_pin = {GPIOA, 1, GPIO_MODER_MODE1, GPIO_OSPEEDR_OSPEED3},
        .cts_pin = {GPIOA, 0, GPIO_MODER_MODE0, GPIO_OSPEEDR_OSPEED3} // Input
    },
    { // USART6
        .usart = USART6,
        .tx_pin = {GPIOC, 6, GPIO_MODER_MODE2, GPIO_OSPEEDR_OSPEED3},
        .rx_pin = {GPIOC, 7, GPIO_MODER_MODE2, GPIO_OSPEEDR_OSPEED3},
        .rts_pin = {GPIOC, 8, GPIO_MODER_MODE1, GPIO_OSPEEDR_OSPEED3},
        .cts_pin = {GPIOC, 9, GPIO_MODER_MODE0, GPIO_OSPEEDR_OSPEED3}
    }
};



/**
  * @brief  Initialize GPIO for PA5 (config shell)
  */
void GPIO_Init(void) {
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* PA5: Input, pull-down */
    GPIOA->MODER &= ~GPIO_MODER_MODE5;
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD5_1;
}


/**
  * @brief  Initialize UARTs (USART1, USART2, USART6)
  */
void UART_Init(void) {
    /* Enable UART clocks */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_USART6EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* Configure UART pins */
    for (int i = 0; i < USB_CDC_NUM_PORTS; i++) {
        const cdc_port_t *port = &port_config[i];
        GPIO_TypeDef *gpio = port->tx_pin.port;
        uint32_t pin = port->tx_pin.pin;

        /* Enable GPIO clock */
        if (gpio == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
        else if (gpio == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

        /* TX: AF push-pull */
        gpio->MODER &= ~GPIO_MODER_MODE0 << (pin * 2);
        gpio->MODER |= GPIO_MODER_MODE2 << (pin * 2);
        gpio->OSPEEDR |= GPIO_OSPEEDR_OSPEED3 << (pin * 2);
        gpio->AFR[pin >> 3] |= (port->usart == USART6 ? 8 : 7) << ((pin % 8) * 4);

        /* RX: Input, AF */
        pin = port->rx_pin.pin;
        gpio->MODER &= ~GPIO_MODER_MODE0 << (pin * 2);
        gpio->MODER |= GPIO_MODER_MODE2 << (pin * 2);
        gpio->AFR[pin >> 3] |= (port->usart == USART6 ? 8 : 7) << ((pin % 8) * 4);

        /* RTS: Output */
        if (port->rts_pin.port) {
            pin = port->rts_pin.pin;
            gpio->MODER &= ~GPIO_MODER_MODE0 << (pin * 2);
            gpio->MODER |= GPIO_MODER_MODE1 << (pin * 2);
            gpio->OSPEEDR |= GPIO_OSPEEDR_OSPEED3 << (pin * 2);
        }
        /* CTS: Input */
        if (port->cts_pin.port) {
            pin = port->cts_pin.pin;
            gpio->MODER &= ~GPIO_MODER_MODE0 << (pin * 2);
        }
    }

    /* Configure UARTs: 115200 baud, 8N1, DMA, interrupts */
    USART_TypeDef *uarts[] = {USART1, USART2, USART6};
    uint32_t apb_freq[] = {84000000, 42000000, 84000000};
    for (int i = 0; i < 3; i++) {
        USART_TypeDef *USARTx = uarts[i];
        uint32_t brr = apb_freq[i] / 115200;

        USARTx->BRR = brr;
        USARTx->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
        USARTx->CR3 = USART_CR3_DMAT | USART_CR3_DMAR | USART_CR3_CTSE | USART_CR3_CTSIE;
        NVIC_EnableIRQ(i == 0 ? USART1_IRQn : i == 1 ? USART2_IRQn : USART6_IRQn);
    }
}

/**
  * @brief  Initialize DMA for UART TX/RX
  */
void DMA_Init(void) {
    /* Enable DMA clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMA2EN;

    /* USART1 TX: DMA2 Stream7 Channel4 */
    DMA2_Stream7->CR = (4 << DMA_SxCR_CHSEL_Pos) | // Channel 4
                       DMA_SxCR_MSIZE_0 | // 8-bit memory
                       DMA_SxCR_PSIZE_0 | // 8-bit peripheral
                       DMA_SxCR_MINC | // Memory increment
                       DMA_SxCR_DIR_1 | // Memory to peripheral
                       DMA_SxCR_TCIE; // Transfer complete interrupt
    NVIC_EnableIRQ(DMA2_Stream7_IRQn);

    /* USART1 RX: DMA2 Stream2 Channel4 */
    DMA2_Stream2->CR = (4 << DMA_SxCR_CHSEL_Pos) |
                       DMA_SxCR_MSIZE_0 |
                       DMA_SxCR_PSIZE_0 |
                       DMA_SxCR_MINC |
                       DMA_SxCR_DIR_0 | // Peripheral to memory
                       DMA_SxCR_TCIE;
    NVIC_EnableIRQ(DMA2_Stream2_IRQn);

    /* USART2 TX: DMA1 Stream6 Channel4 */
    DMA1_Stream6->CR = (4 << DMA_SxCR_CHSEL_Pos) |
                       DMA_SxCR_MSIZE_0 |
                       DMA_SxCR_PSIZE_0 |
                       DMA_SxCR_MINC |
                       DMA_SxCR_DIR_1 |
                       DMA_SxCR_TCIE;
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);

    /* USART2 RX: DMA1 Stream5 Channel4 */
    DMA1_Stream5->CR = (4 << DMA_SxCR_CHSEL_Pos) |
                       DMA_SxCR_MSIZE_0 |
                       DMA_SxCR_PSIZE_0 |
                       DMA_SxCR_MINC |
                       DMA_SxCR_DIR_0 |
                       DMA_SxCR_TCIE;
    NVIC_EnableIRQ(DMA1_Stream5_IRQn);

    /* USART6 TX: DMA2 Stream6 Channel5 */
    DMA2_Stream6->CR = (5 << DMA_SxCR_CHSEL_Pos) |
                       DMA_SxCR_MSIZE_0 |
                       DMA_SxCR_PSIZE_0 |
                       DMA_SxCR_MINC |
                       DMA_SxCR_DIR_1 |
                       DMA_SxCR_TCIE;
    NVIC_EnableIRQ(DMA2_Stream6_IRQn);

    /* USART6 RX: DMA2 Stream1 Channel5 */
    DMA2_Stream1->CR = (5 << DMA_SxCR_CHSEL_Pos) |
                       DMA_SxCR_MSIZE_0 |
                       DMA_SxCR_PSIZE_0 |
                       DMA_SxCR_MINC |
                       DMA_SxCR_DIR_0 |
                       DMA_SxCR_TCIE;
    NVIC_EnableIRQ(DMA2_Stream1_IRQn);
}




/**
  * @brief  Send data via UART using DMA
  */
void UART_Send(USART_TypeDef *USARTx, uint8_t *data, uint32_t len) {
    DMA_Stream_TypeDef *stream = (USARTx == USART1) ? DMA2_Stream7 :
                                (USARTx == USART2) ? DMA1_Stream6 : DMA2_Stream6;

    stream->CR &= ~DMA_SxCR_EN;
    stream->PAR = (uint32_t)&USARTx->DR;
    stream->M0AR = (uint32_t)data;
    stream->NDTR = len;
    stream->CR |= DMA_SxCR_EN;

    USARTx->CR3 |= USART_CR3_DMAT;
}

/**
  * @brief  UART RX handler
  */
void serial_rx_handler(USART_TypeDef *USARTx, uint8_t data) {
    volatile uint8_t *buf = (USARTx == USART1) ? usart1_rx_buf :
                           (USARTx == USART2) ? usart2_rx_buf : usart6_rx_buf;
    volatile uint32_t *idx = (USARTx == USART1) ? &usart1_rx_idx :
                            (USARTx == USART2) ? &usart2_rx_idx : &usart6_rx_idx;

    if (*idx < BUFFER_SIZE) {
        buf[(*idx)++] = data;
        if (data == '\n' || *idx >= BUFFER_SIZE) {
            UART_Send(USARTx, (uint8_t *)buf, *idx);
            *idx = 0;
        }
    }
}


void serial_dma_tx_complete(DMA_Stream_TypeDef *stream) {
    USART_TypeDef *USARTx = (stream == DMA2_Stream7) ? USART1 :
                           (stream == DMA1_Stream6) ? USART2 : USART6;
    USARTx->CR3 &= ~USART_CR3_DMAT;
}

void serial_dma_rx_complete(DMA_Stream_TypeDef *stream) {}
void serial_cts_handler(USART_TypeDef *USARTx) {}  

void serial_error_handler(USART_TypeDef *USARTx, uint32_t error) {}
void serial_dma_error(DMA_Stream_TypeDef *stream) {}

