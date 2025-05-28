#include "stm32f4xx.h"
#include "serial.h"

void serial_rx_handler(USART_TypeDef *USARTx, uint8_t data) {}
void serial_cts_handler(USART_TypeDef *USARTx) {}
void serial_error_handler(USART_TypeDef *USARTx, uint32_t error) {}
void serial_dma_tx_complete(DMA_Stream_TypeDef *stream) {}
void serial_dma_rx_complete(DMA_Stream_TypeDef *stream) {}
void serial_dma_error(DMA_Stream_TypeDef *stream) {}

