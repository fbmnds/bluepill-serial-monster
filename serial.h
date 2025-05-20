/**
  * @file    serial.h
  * @brief   UART handling function declarations
  */

#ifndef SERIAL_H
#define SERIAL_H

#include "stm32f4xx.h"

void serial_rx_handler(USART_TypeDef *USARTx, uint8_t data);
void serial_cts_handler(USART_TypeDef *USARTx);
void serial_error_handler(USART_TypeDef *USARTx, uint32_t error);
void serial_dma_tx_complete(DMA_Stream_TypeDef *hdma);
void serial_dma_rx_complete(DMA_Stream_TypeDef *hdma);
void serial_dma_error(DMA_Stream_TypeDef *hdma);

#endif /* SERIAL_H */
