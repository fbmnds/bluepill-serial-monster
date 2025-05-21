/**
  * @file    device_config.h
  * @brief   UART configuration definitions
  */

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include "stm32f4xx.h"

#define USB_CDC_NUM_PORTS 3


typedef struct {
    GPIO_TypeDef *port;
    uint32_t pin;
    uint32_t mode; // GPIO_MODER_MODE*
    uint32_t speed; // GPIO_OSPEEDR_OSPEED*
} pin_config_t;

typedef struct {
    USART_TypeDef *usart;
    pin_config_t tx_pin;
    pin_config_t rx_pin;
    pin_config_t rts_pin;
    pin_config_t cts_pin;
} cdc_port_t;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t moder;
    uint32_t otyper;
    uint32_t ospeedr;
    uint32_t afr;
} usart_pin_t;

extern const cdc_port_t port_config[USB_CDC_NUM_PORTS];

#endif /* DEVICE_CONFIG_H */
