/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */

#include <stm32f4xx.h>
#include "system_clock.h"


void system_clock_init() {
    /* Enable HSE */
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    /* Configure PLL: 25MHz * 336 / 25 / 4 = 84MHz */
    RCC->PLLCFGR = (25 << RCC_PLLCFGR_PLLM_Pos) |
                   (336 << RCC_PLLCFGR_PLLN_Pos) |
                   (0 << RCC_PLLCFGR_PLLP_Pos) | // PLLP = 4
                   (7 << RCC_PLLCFGR_PLLQ_Pos) | // PLLQ = 7 (48MHz USB)
                   RCC_PLLCFGR_PLLSRC_HSE;
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    /* Set flash latency (2WS for 84MHz at 3.3V) */
    FLASH->ACR = FLASH_ACR_LATENCY_2WS;

    /* Select PLL as system clock */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    /* Configure APB1/APB2 prescalers */
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2 | // APB1 = 42MHz
                 RCC_CFGR_PPRE2_DIV1;  // APB2 = 84MHz

}
