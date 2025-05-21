/**
  * @file    device_config.c
  * @brief   UART pin configuration for STM32F401CCU6
  */

/* Includes */
#include <stddef.h>
#include <limits.h>
#include "stm32f4xx.h"
#include "device_config.h"




/* UART pin configuration */
/*
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
*/


static device_config_t current_device_config;

static uint32_t device_config_calc_crc(const device_config_t *device_config) {
    uint32_t *word_p = (uint32_t*)device_config;
    size_t bytes_left = offsetof(device_config_t, crc);
    CRC->CR |= CRC_CR_RESET;
    while (bytes_left > sizeof(*word_p)) {
        CRC->DR = *word_p++;
        bytes_left -= sizeof(*word_p);
    }
    if (bytes_left) {
        uint32_t shift = 0;
        uint32_t tail = 0;
        uint8_t *byte_p = (uint8_t*)word_p;
        for (int i = 0; i < bytes_left; i++) {
            tail |= (uint32_t)(*byte_p++) << (shift);
            shift += CHAR_BIT;
        }
        CRC->DR = tail;
    }
    return CRC->DR;
}

device_config_t *device_config_get() {
    return &current_device_config;
}
