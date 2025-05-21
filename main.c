/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */

#include <stm32f4xx.h>
#include "version.h"

#include "system_clock.h"

#include "serial.h"

#include "usb.h"
#include "usb_std.h"
#include "usb_panic.h"
#include "usb_uid.h"
#include "circ_buf.h"
#include "usb_cdc.h"
#include "usb_io.h"
#include "usb_core.h"
#include "usb_descriptors.h"

#include "status_led.h"
#include "cdc_config.h"
#include "cdc_shell.h"

/*
#include "system_interrupts.h"
#include "status_led.h"
#include "device_config.h"

*/

int main() {
    system_clock_init();
    status_led_init();
    /*
    system_interrupts_init();
    device_config_init();
    usb_init();
    */
    while (1) {
      /*  usb_poll(); */
      ;
    }
}
