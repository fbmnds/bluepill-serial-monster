/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */

#include <stm32f4xx.h>
#include "system_clock.h"
/*
#include "system_interrupts.h"
#include "status_led.h"
#include "device_config.h"
#include "usb.h"
*/

#define LED_PIN 13

volatile uint32_t ticks;

void status_led_init() {
  
    // Enable clock at GPIO port C
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN_Msk;
    // Execute dsb instruction after enabling the peripheral clock, as per the errata
    __DSB();

    // Set pin 13 connected to blue led of port C as output
    GPIOC->MODER |= (1 << GPIO_MODER_MODER13_Pos);

    // Configure SysTick interrupt to fire every 1ms
    SysTick_Config(SystemCoreClock / 1000);

    __enable_irq();
}

void SysTick_Handler ()
{
    ticks++;
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


int main() {
  system_clock_init();
  status_led_init();

  /*
    system_interrupts_init();
    device_config_init();
    usb_init();

   */
  

    while(1)
    {
      status_led_toggle();
      // usb_poll();
    }

}
