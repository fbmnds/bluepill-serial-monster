/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */

#include "version.h"

#include <stm32f4xx.h>
#include "system_clock.h"
#include "status_led.h"
#include "stm32f4xx_it.h"
/*
#include "device_config.h"
#include "usb.h"
*/


int main() {
  system_clock_init();
  status_led_init();
  system_interrupts_init();

  /*
    device_config_init();
    usb_init();

   */
  

  while(1)
    {
      status_led_toggle();
      // usb_poll();
    }

}
