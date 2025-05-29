/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */
#include <string.h>

#include "version.h"

#include <stm32f4xx.h>
#include "system_clock.h"
#include "status_led.h"
#include "stm32f4xx_it.h"
#include "serial.h"

/*
#include "device_config.h"
#include "usb.h"
*/


int main() {
  char *msg = "UART/DMA Test Started: Echo mode enabled\r\n";
  char *shell_msg = "Configuration Shell Active (USART1)\r\n";


  system_clock_init();
  enable_periph_clocks ();
  
  //  system_interrupts_init();
  GPIO_Init();
  UART_Init();
  DMA_Init();
  status_led_init();
  /*
    device_config_init();
    usb_init();

   */


  /* Send test message via USART1 */

  //UART_Send(USART2, (uint8_t *)msg, strlen(msg));

  serial_rx_handler(USART1, '^');
  serial_rx_handler(USART2, '*');
  serial_rx_handler(USART6, '$');

  while(1)
    {
      status_led_toggle();
      // usb_poll();
  serial_rx_handler(USART1, '^');
  serial_rx_handler(USART2, '*');
  serial_rx_handler(USART6, '$');
      for (volatile uint32_t i = 0; i < 1000000; i++); // Debounce
      /*                  
      if (!(GPIOA->IDR & GPIO_IDR_ID5)) { // PA5 low
        //UART_Send(USART2, (uint8_t *)shell_msg, strlen(shell_msg));
        for (volatile uint32_t i = 0; i < 10000; i++); // Debounce
      }
      */
    }

}
