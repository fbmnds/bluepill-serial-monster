/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */

#include <stm32f4xx.h>
/*
#include "system_clock.h"
#include "system_interrupts.h"
#include "status_led.h"
#include "device_config.h"
#include "usb.h"
*/

#define LED_PIN 13

int main() {
  //system_clock_init();


        // Enable clock at GPIO port C
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN_Msk;
    // Execute dsb instruction after enabling the peripheral clock, as per the errata
    __DSB();

    // Set pin 13 connected to blue led of port C as output
    GPIOC->MODER |= (1 << GPIO_MODER_MODER13_Pos);

    while(1)
    {
        GPIOC->ODR ^= (1 << LED_PIN);
        for (uint32_t i = 0; i < 1000000; i++);
    }

    
    /*
    system_interrupts_init();
    device_config_init();
    status_led_init();
    usb_init();
    
    while (1) {
      usb_poll();
      ;
    }
    */
}
