/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */

#include <stm32f4xx.h>
#include "status_led.h"

/*
#include "gpio.h"
#include "device_config.h"
*/

extern volatile uint32_t ticks;

void status_led_init() {
  
    // Set pin 13 connected to blue led of port C as output
    GPIOC->MODER |= (1 << GPIO_MODER_MODER13_Pos);

    // Configure SysTick interrupt to fire every x ms
    SysTick_Config(SystemCoreClock / LED_DELAY);
}


void delay_ms (uint32_t ms)
{
    uint32_t start = ticks;
    uint32_t end = start + ms;

    // Overflow condition
    if (end < start)
        while (ticks > start); // Wait for ticks to wrap

    while (ticks < end);
}

void status_led_toggle() {
        GPIOC->ODR ^= (1 << LED_PIN);
        delay_ms(1500);
}
