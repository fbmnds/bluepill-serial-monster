/**
  * @file    serial.c
  * @brief   UART and DMA handling for STM32F401CCU6
  */

/* Includes */
#include "stm32f4xx.h"
#include "serial.h"

/* External variables */
DMA_TypeDef hdma_usart1_tx, hdma_usart1_rx;
DMA_TypeDef hdma_usart2_tx, hdma_usart2_rx;
DMA_TypeDef hdma_usart6_tx, hdma_usart6_rx;

/* UART initialization (called by main.c) */
void UART_Init(void); // Defined in main.c for test program

/* DMA initialization (called by main.c) */
void DMA_Init(void); // Defined in main.c for test program

/* Function definitions for interrupt handlers */
void serial_rx_handler(USART_TypeDef *USARTx, uint8_t data);
void serial_cts_handler(USART_TypeDef *USARTx);
void serial_error_handler(USART_TypeDef *USARTx, uint32_t error);
void serial_dma_tx_complete(DMA_TypeDef *hdma);
void serial_dma_rx_complete(DMA_TypeDef *hdma);
void serial_dma_error(DMA_TypeDef *hdma);
