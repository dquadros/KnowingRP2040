/**
 * @file rtcdemo.c
 * @author Daniel Quadros
 * @brief Example of using the Real Time Clock
 *        Based on the hello_48MHz and  hello_gpout SDK examples
 * @version 0.1
 * @date 2022-07-14
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "hardware/rtc.h"

static volatile bool fired;

// This rotine will be called when the alarm fires
static void alarm_callback(void) {
    datetime_t dt;

    // Disable alarm
    rtc_disable_alarm();

    // Get the current time and convert it to a string
    rtc_get_datetime(&dt);
    char datetime_buf[256];
    char *datetime_str = &datetime_buf[0];
    datetime_to_str(datetime_str, sizeof(datetime_buf), &dt);

    // Inform alarm fired
    printf("Alarm fired at %s\n", datetime_str);
    stdio_flush();
    fired = true;
}


// Main Program
int main() {
    stdio_init_all();
    #ifdef LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    #endif

    printf("RTC Example\n");

    // Initializes the RTC
    datetime_t dt;
    rtc_init();
    while (true) {
        int dig[14];
        int n = 0;
        int c;
        printf("Enter date and time as MMDDYYYYHHMMSS\n");
        while (n < 14) {
            c = getchar_timeout_us(1000);
            if ((c >= '0') && (c <= '9')) {
                 putchar_raw(c);
                dig[n++] = c - '0';
            }
        }
        printf("\n");
        dt.month = dig[0]*10+dig[1];
        dt.day = dig[2]*10+dig[3];
        dt.year = dig[4]*1000+dig[5]*100+dig[6]*10+dig[7];
        dt.dotw = 0;
        dt.hour = dig[8]*10+dig[9];
        dt.min = dig[10]*10+dig[11];
        dt.sec = dig[12]*10+dig[13];
        if (rtc_set_datetime(&dt)) {
            break;
        }
    }

    // Main loop: set alarm and wait
    dt.month = -1;
    dt.day = -1;
    dt.year = -1;
    dt.dotw = -1;
    dt.hour = -1;
    while (true) {
        fired = false;
        dt.min = (dt.min + 1 + (rand() % 5)) % 60;
        rtc_set_alarm(&dt, alarm_callback);
        printf ("Alarm set for xx:%02d:%02d\n", dt.min, dt.sec);
        while (!fired) {
            // do nothing
        }
    }

    return 0;
}
