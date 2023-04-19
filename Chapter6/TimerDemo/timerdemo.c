/**
 * @file ctimerdemo.c
 * @author Daniel Quadros
 * @brief Example of using the Timer
 * @version 0.1
 * @date 2022-07-14
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#define ALARM_NO 1

static volatile bool fired;

// Alarm callback routine
void rotAlarm(uint alarm_num) {
    printf ("Alarm %d fired\n");
    fired = true;
}

// Main program
int main() {
    stdio_init_all();
    #ifdef LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    #endif

    printf("Timer Example\n\n");

    // Reading the timer a few times
    for (int i = 0; i < 5; i++) {
        printf("Timer: %llu\n", time_us_64());
        busy_wait_us_32(rand() % 10000);    // wait a random time 0 to 9,999 us
    }
    printf ("\n");

    // Set up the alarm
    hardware_alarm_claim(ALARM_NO);
    hardware_alarm_set_callback(ALARM_NO, rotAlarm);

    // Wait for the alarm at random times
    while (true) {
        fired = false;
        uint32_t delay = 1000 * (1 + rand() % 30);  // 1 to 30 sconds
        absolute_time_t now;
        update_us_since_boot(&now, time_us_64());
        absolute_time_t target =  delayed_by_ms(now, delay);
        hardware_alarm_set_target(ALARM_NO, target);
        printf ("Waiting for %llu (delay %us)\n",  to_us_since_boot(target), delay/1000);
        while (!fired) {
            tight_loop_contents();
        }
        printf("Timer: %llu\n\n", time_us_64());
    }

    return 0;
}
