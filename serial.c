#include "stm32f4xx.h"
#include "serial.h"

#define BUFFER_SIZE 100

volatile uint8_t usart1_rx_buf[BUFFER_SIZE];
volatile uint8_t usart2_rx_buf[BUFFER_SIZE];
volatile uint8_t usart6_rx_buf[BUFFER_SIZE];
volatile uint32_t usart1_rx_idx = 0;
volatile uint32_t usart2_rx_idx = 0;
volatile uint32_t usart6_rx_idx = 0;




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

