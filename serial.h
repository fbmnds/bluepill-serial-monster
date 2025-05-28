#ifndef SERIAL_H
#define SERIAL_H

#include "stm32f4xx.h"

#define USB_CDC_NUM_PORTS 3

typedef struct {
    GPIO_TypeDef *port;
    uint32_t pin;
    uint32_t mode;
    uint32_t speed;
} pin_config_t;

typedef struct {
    USART_TypeDef *usart;
    pin_config_t tx_pin;
    pin_config_t rx_pin;
    pin_config_t rts_pin;
    pin_config_t cts_pin;
} cdc_port_t;

void GPIO_Init(void);
void UART_Init(void);
void DMA_Init(void);

void serial_rx_handler(USART_TypeDef *USARTx, uint8_t data);
void serial_cts_handler(USART_TypeDef *USARTx);
void serial_error_handler(USART_TypeDef *USARTx, uint32_t error);
void serial_dma_tx_complete(DMA_Stream_TypeDef *stream);
void serial_dma_rx_complete(DMA_Stream_TypeDef *stream);
void serial_dma_error(DMA_Stream_TypeDef *stream);

#endif /* SERIAL_H */
