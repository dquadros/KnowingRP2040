/**
 * @file sleep.c
 * @author Daniel Quadros
 * @brief Example of using the SLEEP and DORMANT states
 * @version 0.1
 * @date 2022-08-18
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 * This examples is an adaptation of the hello_sleep and
 * hello_dormant examples in pico_playground.
 * 
 */

#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "pico/time.h"

#include "hardware/gpio.h"
#include "hardware/rtc.h"

// GPIO connections
#define LED   0
#define BTN1  2
#define BTN2  4

// For button detection and debounce
typedef enum { NOT_PRESSED, DBC_PRESS, PRESSED, DBC_RELEASE } BTN_STATE;

// Aux routine to ger milliseconds since boot
static inline uint32_t board_millis(void) {
	return to_ms_since_boot(get_absolute_time());
}

// This routine will be called when the RTC wakes the RP2040
static void sleep_callback(void) {
}

// Puts the RP2040 to sleep for 5 seconds
static void rtc_sleep(void) {
    // Arbitrary start on 31 March 2021 18:00:00
    datetime_t t = {
            .year  = 2021,
            .month = 05,
            .day   = 31,
            .dotw  = 3, // 0 is Sunday
            .hour  = 18,
            .min   = 00,
            .sec   = 00
    };

    // Start the RTC to our arbitraty start
    rtc_init();
    rtc_set_datetime(&t);

    // Sleep 5 seconds
    t.sec = 5;
    sleep_goto_sleep_until(&t, &sleep_callback);
}


// Main routine
int main() {

    // We will run from XOSC
    sleep_run_from_xosc();

    // Init the GPIO pins
     gpio_init(LED);
     gpio_set_dir(LED, true);
     gpio_init(BTN1);
     gpio_set_dir(BTN1, false);
     gpio_pull_up(BTN1);
     gpio_init(BTN2);
     gpio_set_dir(BTN2, false);
     gpio_pull_up(BTN2);

    // Main loop
    uint32_t ledTime = board_millis();
    bool ledValue = false;
    BTN_STATE btn1State = NOT_PRESSED;
    BTN_STATE btn2State = NOT_PRESSED;
    uint32_t btnTime;
    while (true) {
        // Blink LED every 300 ms (if awake)
        if (board_millis() > ledTime) {
            ledValue = !ledValue;
            gpio_put(LED, ledValue);
            ledTime = board_millis() + 300;
        }

        // Check button 1
        if (btn2State == NOT_PRESSED) {
            switch (btn1State) {
                case NOT_PRESSED:
                    if (!gpio_get(BTN1)) {
                        btn1State = DBC_PRESS;
                        btnTime = board_millis() + 100;
                    }
                    break;
                case DBC_PRESS:
                    if (board_millis() > btnTime) {
                        btn1State = PRESSED;
                    }
                    break;
                case PRESSED:
                    if (gpio_get(BTN1)) {
                        btn1State = DBC_RELEASE;
                        btnTime = board_millis() + 100;
                    }
                    break;
                case DBC_RELEASE:
                    if (board_millis() > btnTime) {
                        btn1State = NOT_PRESSED;
                        // Button was pressed and released
                        // Sleep
                        gpio_put(LED, false);
                        rtc_sleep();
                    }
                    break;
            }
        }

        // Check button 2
        if (btn1State == NOT_PRESSED) {
            switch (btn2State) {
                case NOT_PRESSED:
                    if (!gpio_get(BTN2)) {
                        btn2State = DBC_PRESS;
                        btnTime = board_millis() + 100;
                    }
                    break;
                case DBC_PRESS:
                    if (board_millis() > btnTime) {
                        btn2State = PRESSED;
                    }
                    break;
                case PRESSED:
                    if (gpio_get(BTN2)) {
                        btn2State = DBC_RELEASE;
                        btnTime = board_millis() + 100;
                    }
                    break;
                case DBC_RELEASE:
                    if (board_millis() > btnTime) {
                        btn2State = NOT_PRESSED;
                        // Button was pressed and released
                        // Put in dormant mode until button 1 is released
                        gpio_put(LED, false);
                        sleep_goto_dormant_until_pin(BTN1, true, true);
                        // Give some time for BTN1 release debounce
                        busy_wait_ms(100);
                    }
                    break;
            }
        }

    }

    return 0;
}
