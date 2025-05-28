/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */

#include <stm32f4xx.h>
#include "system_clock.h"


void system_clock_init() {

  
    /*
     * https://github.com/e-pirate/blackpill-gcc-cmsis-blink
     *
     * After reset MCU clock source is set to internal 16 MHz RC oscillator
     * and flash wait states (WS) set to 0. To switch clock source and increase
     * CPU (HCLK) frequency special procedure must be performed.
     */

    // 1. Enable HSE (High speed external clock signal) @ 25 MHz and wait for it to become ready
    RCC->CR |= RCC_CR_HSEON_Msk;
    while (! (RCC->CR & RCC_CR_HSERDY_Msk));

    // 2a. Enable power interface clock
    RCC->APB1ENR |= RCC_APB1ENR_PWREN_Msk;
    // Execute dsb instruction after enabling the peripheral clock, as per the errata
    __DSB();

    // 2b. Set regulator voltage scale mode 3 (HCLK <= 64MHz)
    PWR->CR &= ~(PWR_CR_VOS_Msk);
    PWR->CR |= (0b01 << PWR_CR_VOS_Pos);

    // 3. Configure flash controller for 2.7-3.3V supply and 30 MHz < HCLK <= 64 < MHz -> 1 WS,
    // enable Instruction prefetch (PRFTEN), Instruction cache (ICEN), Data cache (DCEN)
    FLASH->ACR |= (FLASH_ACR_DCEN_Msk | FLASH_ACR_ICEN_Msk | FLASH_ACR_PRFTEN_Msk | FLASH_ACR_LATENCY_1WS);

    /*
     * 4. Main PLL configuration:
     * PLL input should be withing 1-2 MHz range to achieve that with HSE @ 25 MHz choosing PLLM=25 that gives 25 MHz / 25 = 1 MHz;
     * The output of the PLL fVCO should be withing 100-432 MHz range, choosing PLLN=192 gives fVCO = 1 MHz * 192 = 192 MHz;
     * Compromise SYSCLK will be 48 MHz. Choosing PLLP=4 giving PLLCLK and SYSCLK = 192 MHz / 4 = 48 MHz;
     * The USB OTG FS clock must be exactly 48 MHz, to achieve that choosing PLLQ=4 giving PLL48CLK = 192 MHz / 4 = 48 MHz;
     * Final PLL configuration is: PLLM=25, PLLN=192, PLLP=4, PLLQ=3
     * SYSCLK=48 MHz, PLL48CLK=48 MHz
     */

    // Clear PLLM, PLLN, PLLP, PLLQ bits
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM_Msk | RCC_PLLCFGR_PLLN_Msk | RCC_PLLCFGR_PLLP_Msk | RCC_PLLCFGR_PLLQ_Msk);
    // Set PLLM, PLLN, PLLP, PLLQ  and select HSE as PLL source
    RCC->PLLCFGR |= ((25 << RCC_PLLCFGR_PLLM_Pos) | (192 << RCC_PLLCFGR_PLLN_Pos) | (0b01 << RCC_PLLCFGR_PLLP_Pos) | (4 << RCC_PLLCFGR_PLLQ_Pos) | (1 << RCC_PLLCFGR_PLLSRC_Pos));

    /*
     * 5. Prescalers configuration:
     * HCLK (CPU) clock derived from SYSCLK and must not exceed 100 MHz, choosing AHB prescaler HPRE = 1 giving HCLK = SYSCLK;
     * APB1 low speed peripheral clock PCLK1 derived from HCLK and must not exceed 50 MHz, choosing PPRE1 = 1, giving APB1 = HCLK
     * APB2 high speed peripheral clock PCLK2 derived from HCLK and must no exceed 100 MHz, choosing PPRE2 = 1, giving APB2 = HCLK
     */

    // Clear PPRE2, PPRE1, HPRE bits
    RCC->CFGR &= ~(RCC_CFGR_PPRE2_Msk | RCC_CFGR_PPRE1_Msk | RCC_CFGR_HPRE_Msk);
    // Set  PPRE2, PPRE1, HPRE bit (this actually senseless as all bits are zeros after clearing)
    RCC->CFGR |= ((0b000 << RCC_CFGR_PPRE2_Pos) | (0b000 << RCC_CFGR_PPRE1_Pos) | (0b0000 << RCC_CFGR_HPRE_Pos));

    // 6. Enable PLL and wait for it to become ready
    RCC->CR |= RCC_CR_PLLON_Msk;
    while (! (RCC->CR & RCC_CR_PLLRDY_Msk));

    // 7. Wait for the Regulator voltage scaling mode to be actually set after enabling PLL
    while (! (PWR->CSR & PWR_CSR_VOSRDY_Msk));

    // 8. Select PLL output as system clock source
    RCC->CFGR |= (RCC_CFGR_SW_PLL << RCC_CFGR_SW_Pos);
    // Wait for PLL to become system clock source
    while (! (RCC->CFGR & RCC_CFGR_SWS_PLL));

    // 9. Disable internal 16 MHz RC oscillator (HSI) to reduce power consumption as it is not needed anymore
    RCC->CR &= ~(RCC_CR_HSION_Msk);

    // Update the internal clock frequency variable
    SystemCoreClockUpdate();

}

void enable_periph_clocks () {

  /* Enable GPIOA clock */
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN_Msk;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN_Msk;
    
  /* Enable UART clocks */
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk | RCC_APB2ENR_USART6EN_Msk;
  //RCC->APB1ENR |= RCC_APB1ENR_USART2EN_Msk;
  RCC->APB1ENR |= (1U<<17);

  // ES0287 - Rev 6, 2.2.7
  __DSB();
}
