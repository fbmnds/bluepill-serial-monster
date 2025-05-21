/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */


#include <stm32f4xx.h>
#include "device_config.h"
#include "gpio.h"



#include "stm32f4xx.h"
#include "device_config.h"

static void _gpio_enable_port(uint8_t portnum) {
    // Enable clock for GPIO port (STM32F4 uses AHB1 bus)
    RCC->AHB1ENR |= (1 << portnum); // e.g., GPIOA=0, GPIOB=1, GPIOC=2
}

void usart_pin_init(usart_pin_t *pin) {
    if (!pin || !pin->port) return;

    // Enable GPIO port clock
    if (pin->port == GPIOA) _gpio_enable_port(0);
    else if (pin->port == GPIOB) _gpio_enable_port(1);
    else if (pin->port == GPIOC) _gpio_enable_port(2);
    // Add other ports (GPIOD, GPIOE, etc.) if needed

    GPIO_TypeDef *gpio = pin->port;
    uint16_t p = pin->pin;

    // Configure GPIO: mode, output type, speed, alternate function
    gpio->MODER = (gpio->MODER & ~(3 << (p * 2))) | (pin->moder << (p * 2));
    gpio->OTYPER = (gpio->OTYPER & ~(1 << p)) | (pin->otyper << p);
    gpio->OSPEEDR = (gpio->OSPEEDR & ~(3 << (p * 2))) | (pin->ospeedr << (p * 2));
    if (p < 8) {
        gpio->AFR[0] = (gpio->AFR[0] & ~(15 << (p * 4))) | (pin->afr << (p * 4));
    } else {
        gpio->AFR[1] = (gpio->AFR[1] & ~(15 << ((p - 8) * 4))) | (pin->afr << ((p - 8) * 4));
    }
}


void gpio_pin_set(const gpio_pin_t *pin, int is_active) {
    if (pin->port) {
        pin->port->BSRR = (GPIO_BSRR_BS0 << pin->pin) 
            << (!!is_active != (pin->polarity == gpio_polarity_low) ? 0 : GPIO_BSRR_BR0_Pos);
    }
}

int gpio_pin_get(const gpio_pin_t *pin) {
    if (pin->port) {
        return (!!(pin->port->IDR & (GPIO_IDR_IDR_0 << pin->pin))) != (pin->polarity == gpio_polarity_low);
    }
    return 0;
}

volatile uint32_t *gpio_pin_get_bitband_clear_addr(const gpio_pin_t *pin) {
    volatile uint32_t result = 0;
    if (pin->port) {
        result = PERIPH_BB_BASE;
        result += ((uint32_t)(&pin->port->BSRR) - PERIPH_BASE) << 5;
        result += pin->pin << 2;
        if (pin->polarity == gpio_polarity_high) {
            result += GPIO_BSRR_BR0_Pos << 2;
        }
    }
    return (volatile uint32_t*)result;
}
