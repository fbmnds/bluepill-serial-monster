/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines for STM32F401CCU6 (bluepill-serial-monster port)
  * @author  Adapted for STM32F401CCU6 port
  ******************************************************************************
  * @attention
  * This file is based on the bluepill-serial-monster firmware requirements.
  * It handles UART (USART1, USART2, USART6) and DMA interrupts for the 3-port
  * USB-to-serial functionality.
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "serial.h"

volatile uint32_t ticks;

void system_interrupts_init() {
    NVIC_SetPriorityGrouping(SYSTEM_INTERRUPTS_PRIORITY_GROUPING);
}

/* Cortex-M4 Exception Handlers */
void NMI_Handler(void) { while (1) {} }
void HardFault_Handler(void) { while (1) {} }
void MemManage_Handler(void) { while (1) {} }
void BusFault_Handler(void) { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }
void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}
void SysTick_Handler(void) { ticks++; }

void UART_IRQHandler(USART_TypeDef *USARTx);

/* UART Handlers */
void USART1_IRQHandler(void) { UART_IRQHandler(USART1); }
void USART2_IRQHandler(void) { UART_IRQHandler(USART2); }
void USART6_IRQHandler(void) { UART_IRQHandler(USART6); }

void DMA_IRQHandler(DMA_Stream_TypeDef *stream);

/* DMA Handlers */
void DMA2_Stream7_IRQHandler(void) { DMA_IRQHandler(DMA2_Stream7); }
void DMA2_Stream2_IRQHandler(void) { DMA_IRQHandler(DMA2_Stream2); }
void DMA1_Stream6_IRQHandler(void) { DMA_IRQHandler(DMA1_Stream6); }
void DMA1_Stream5_IRQHandler(void) { DMA_IRQHandler(DMA1_Stream5); }
void DMA2_Stream6_IRQHandler(void) { DMA_IRQHandler(DMA2_Stream6); }
void DMA2_Stream1_IRQHandler(void) { DMA_IRQHandler(DMA2_Stream1); }


/**
  * @brief Generic UART interrupt handler
  */
void UART_IRQHandler(USART_TypeDef *USARTx) {
    uint32_t isrflags = USARTx->SR;
    uint32_t cr1its = USARTx->CR1;

    if ((isrflags & USART_SR_RXNE) && (cr1its & USART_CR1_RXNEIE)) {
        uint8_t data = USARTx->DR;
        serial_rx_handler(USARTx, data);
    }

    if ((isrflags & USART_SR_TXE) && (cr1its & USART_CR1_TXEIE)) {
        USARTx->CR1 &= ~USART_CR1_TXEIE;
    }

    if ((isrflags & USART_SR_CTS) && (USARTx->CR3 & USART_CR3_CTSIE)) {
        USARTx->SR &= ~USART_SR_CTS;
        serial_cts_handler(USARTx);
    }

    if (isrflags & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) {
        USARTx->DR;
        serial_error_handler(USARTx, isrflags);
    }
}

/**
  * @brief Generic DMA interrupt handler
  */
void DMA_IRQHandler(DMA_Stream_TypeDef *stream) {
    uint32_t flag_it = stream == DMA2_Stream7 || stream == DMA2_Stream2 || stream == DMA2_Stream6 || stream == DMA2_Stream1 ?
                       DMA2->LISR : DMA1->LISR;
    uint32_t source_it = stream->CR;
    uint32_t stream_idx = (stream == DMA2_Stream7 || stream == DMA1_Stream7) ? 24 :
                         (stream == DMA2_Stream6 || stream == DMA1_Stream6) ? 16 :
                         (stream == DMA2_Stream5 || stream == DMA1_Stream5) ? 8 : 0;

    if ((flag_it & (DMA_LISR_TCIF0 << stream_idx)) && (source_it & DMA_SxCR_TCIE)) {
        if (stream == DMA2_Stream7 || stream == DMA1_Stream6 || stream == DMA2_Stream6) {
            (stream->CR & DMA_SxCR_DIR_1) ? serial_dma_tx_complete(stream) : serial_dma_rx_complete(stream);
        } else {
            serial_dma_rx_complete(stream);
        }
        stream == DMA2_Stream7 || stream == DMA2_Stream2 || stream == DMA2_Stream6 || stream == DMA2_Stream1 ?
            (DMA2->LIFCR = DMA_LIFCR_CTCIF0 << stream_idx) :
            (DMA1->LIFCR = DMA_LIFCR_CTCIF0 << stream_idx);
    }

    if ((flag_it & (DMA_LISR_TEIF0 << stream_idx)) && (source_it & DMA_SxCR_TEIE)) {
        stream == DMA2_Stream7 || stream == DMA2_Stream2 || stream == DMA2_Stream6 || stream == DMA2_Stream1 ?
            (DMA2->LIFCR = DMA_LIFCR_CTEIF0 << stream_idx) :
            (DMA1->LIFCR = DMA_LIFCR_CTEIF0 << stream_idx);
        serial_dma_error(stream);
    }
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
