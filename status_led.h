/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */

#ifndef STATUS_LED
#define STATUS_LED

#define LED_PIN 13
#define LED_DELAY 800

#include "stm32f4xx_it.h"

void status_led_init();
//void status_led_set(int on);
void status_led_toggle();

#endif /* STATUS_LED */
