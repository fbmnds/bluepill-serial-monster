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
#include "serial.h" // UART handling functions
#include "usb_cdc.h" // USB CDC interface
#include "device_config.h" // Port configuration

/* External variables */
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart6_tx;
extern DMA_HandleTypeDef hdma_usart6_rx;

/* Private function prototypes */
void UART_IRQHandler(USART_TypeDef *USARTx);
void DMA_IRQHandler(DMA_HandleTypeDef *hdma);

/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
    while (1) {
        // Infinite loop for debugging
    }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
    while (1) {
        // Infinite loop for debugging
    }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
    while (1) {
        // Infinite loop for debugging
    }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
    while (1) {
        // Infinite loop for debugging
    }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
    while (1) {
        // Infinite loop for debugging
    }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
    UART_IRQHandler(USART1);
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
    UART_IRQHandler(USART2);
}

/**
  * @brief This function handles USART6 global interrupt.
  */
void USART6_IRQHandler(void)
{
    UART_IRQHandler(USART6);
}

/**
  * @brief This function handles DMA2 Stream7 interrupt (USART1 TX).
  */
void DMA2_Stream7_IRQHandler(void)
{
    DMA_IRQHandler(&hdma_usart1_tx);
}

/**
  * @brief This function handles DMA2 Stream2 interrupt (USART1 RX).
  */
void DMA2_Stream2_IRQHandler(void)
{
    DMA_IRQHandler(&hdma_usart1_rx);
}

/**
  * @brief This function handles DMA1 Stream6 interrupt (USART2 TX).
  */
void DMA1_Stream6_IRQHandler(void)
{
    DMA_IRQHandler(&hdma_usart2_tx);
}

/**
  * @brief This function handles DMA1 Stream5 interrupt (USART2 RX).
  */
void DMA1_Stream5_IRQHandler(void)
{
    DMA_IRQHandler(&hdma_usart2_rx);
}

/**
  * @brief This function handles DMA2 Stream6 interrupt (USART6 TX).
  */
void DMA2_Stream6_IRQHandler(void)
{
    DMA_IRQHandler(&hdma_usart6_tx);
}

/**
  * @brief This function handles DMA2 Stream1 interrupt (USART6 RX).
  */
void DMA2_Stream1_IRQHandler(void)
{
    DMA_IRQHandler(&hdma_usart6_rx);
}

/**
  * @brief Generic UART interrupt handler for RX/TX and flow control.
  * @param USARTx Pointer to USART peripheral (USART1, USART2, or USART6).
  */
void UART_IRQHandler(USART_TypeDef *USARTx)
{
    uint32_t isrflags = USARTx->SR;
    uint32_t cr1its = USARTx->CR1;

    // Handle RXNE (Receive Data Register Not Empty)
    if ((isrflags & USART_SR_RXNE) && (cr1its & USART_CR1_RXNEIE)) {
        uint8_t data = USARTx->DR; // Read data, clears RXNE
        serial_rx_handler(USARTx, data); // Process received data
    }

    // Handle TXE (Transmit Data Register Empty)
    if ((isrflags & USART_SR_TXE) && (cr1its & USART_CR1_TXEIE)) {
        // Handled by DMA in bluepill-serial-monster; disable TXE interrupt if active
        USARTx->CR1 &= ~USART_CR1_TXEIE;
    }

    // Handle CTS interrupt (if enabled)
    if ((isrflags & USART_SR_CTS) && (USARTx->CR3 & USART_CR3_CTSIE)) {
        USARTx->SR &= ~USART_SR_CTS; // Clear CTS flag
        serial_cts_handler(USARTx); // Update flow control
    }

    // Handle errors (ORE, NE, FE, PE)
    if (isrflags & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) {
        USARTx->DR; // Read DR to clear error flags
        serial_error_handler(USARTx, isrflags); // Handle errors
    }
}

/**
  * @brief Generic DMA interrupt handler for UART TX/RX.
  * @param hdma Pointer to DMA handle.
  */
void DMA_IRQHandler(DMA_HandleTypeDef *hdma)
{
    uint32_t flag_it = hdma->Instance->ISR;
    uint32_t source_it = hdma->Instance->CR;

    // Transfer Complete
    if ((flag_it & (DMA_FLAG_TCIF0 << (hdma->StreamIndex & 0x1F))) &&
        (source_it & DMA_SxCR_TCIE)) {
        hdma->Instance->IFCR = DMA_FLAG_TCIF0 << (hdma->StreamIndex & 0x1F);
        if (hdma->Direction == DMA_MEMORY_TO_PERIPH) {
            serial_dma_tx_complete(hdma); // Handle TX completion
        } else {
            serial_dma_rx_complete(hdma); // Handle RX completion
        }
    }

    // Transfer Error
    if ((flag_it & (DMA_FLAG_TEIF0 << (hdma->StreamIndex & 0x1F))) &&
        (source_it & DMA_SxCR_TEIE)) {
        hdma->Instance->IFCR = DMA_FLAG_TEIF0 << (hdma->StreamIndex & 0x1F);
        serial_dma_error(hdma); // Handle DMA error
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
