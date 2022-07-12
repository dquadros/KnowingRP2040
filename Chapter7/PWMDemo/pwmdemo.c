/**
 * @file pwmdemo.c
 * @author Daniel Quadros
 * @brief Example of using the PWM in the RP2040
 *        This example was used to generate the figures in the boot
 * @version 0.1
 * @date 2022-07-09
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// PWM pins
#define PIN_A   0
#define PIN_B   1

// WRAP value
#define WRAP 1000

// Frequency
#define FREQ 500.0f

// Main Program
int main() {
    // Init stdio
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("\nPWM Example\n");

    // Find out which PWM slice is connected to the pins
    uint slice_num = pwm_gpio_to_slice_num(PIN_A);
    if (slice_num != pwm_gpio_to_slice_num(PIN_B)) {
        printf("Pins are not in the same slice!\n");
        printf("Aborting...\n");
        while (true) {
            sleep_ms(100);
        }
    }

    // Configure the slice
    // f = fsys / (clock divisor * (wrap value+1)
    // clock divisor = fsys / (f * (wrap value+1))
    float fsys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS)*1000.0f;
    float div = fsys/(FREQ * (WRAP+1));
    printf("fsys= %.2f div=%.2f\n", fsys, div);
    pwm_config config = pwm_get_default_config ();
    pwm_config_set_wrap(&config, WRAP);
    pwm_config_set_clkdiv(&config, div);
    pwm_config_set_phase_correct(&config, false);
    pwm_config_set_clkdiv_mode(&config, PWM_DIV_FREE_RUNNING);
    pwm_init(slice_num, &config, false);
    pwm_set_both_levels(slice_num, WRAP/2, WRAP/4);
    pwm_set_enabled(slice_num, true);

    // Connect PINs to the PWM
    gpio_set_function(PIN_A, GPIO_FUNC_PWM);
    gpio_set_function(PIN_B, GPIO_FUNC_PWM);

    // Main loop
    bool phase_correct = false;
    while (true) {
        if (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {
            // Change phase correct if anything received from stdio
            // Stop PWM while changing configuration
            // If pahse correct, PWM will count twice,
            //   so we double the clock frequence
            phase_correct = !phase_correct;
            pwm_set_enabled(slice_num, false);
            pwm_set_clkdiv(slice_num, phase_correct? div/2.0f : div);
            pwm_set_phase_correct(slice_num, phase_correct);
            pwm_set_counter(slice_num, 0);
            pwm_set_enabled(slice_num, true);
            printf ("Phase correct: %s\n", phase_correct? "ON" : "OFF");
        }
    }
}
