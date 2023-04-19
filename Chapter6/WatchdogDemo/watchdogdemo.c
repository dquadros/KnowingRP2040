/**
 * @file watchdogdemo.c
 * @author Daniel Quadros
 * @brief Example of using the Watchdog
 * @version 0.1
 * @date 2022-07-14
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

int main() {
    // Init sdio
    stdio_init_all();
    #ifdef LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    #endif

    printf("Watchdog Example\n\n");
    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
        sleep_ms(500);
    } else {
        printf("Clean boot\n");
    }

    // Enable the watchdog with a 100ms timeout
    watchdog_enable(100, false);
   
    // Lets play watchdog roulet!
    while (true) {
        printf (".");
        sleep_ms(rand() % 101); // 0 to 100ms 
        watchdog_update();
    }

    return 0;
}
