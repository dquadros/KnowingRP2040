/**
 * @file pwmmesurement.c
 * @author Daniel Quadros
 * @brief Example of using the PWM peripheral in the RP2040 for
 *        measuring frequence and duty cycle
 *        This is am expansion of the measure_duty_cycle SDK example
 * @version 0.1
 * @date 2022-07-11
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

// Pins - this pins should be connected together
const uint OUTPUT_PIN = 1;
const uint MEASURE_PIN = 3;     // this must be an PWM "B" pin

// WRAP value for PWM generation
#define WRAP 1000

// Values for measurement
#define MEASURE_C_DIV     20
#define MEASURE_C_TIME    10      // ms
#define MEASURE_F_DIV     1
#define MEASURE_F_TIME    100     // ms

// Test values
struct {
    float freq;
    float duty;
} test[] = 
{
    { 500.0f, 0.0f },
    { 500.0f, 1.0f },
    { 500.0f, 0.25f },
    { 500.0f, 0.5f },
    { 500.0f, 0.75f },
    { 492.0f, 0.60f },
    { 947.0f, 0.60f },
    { 1000.0f, 0.25f },
    { 0.0, 0.0}
};

// Generate PWM
void generate_pwm(int slice, float freq, float duty) {
    float fsys = clock_get_hz(clk_sys);
    float div = fsys/(freq * (WRAP+1));
    pwm_config config = pwm_get_default_config ();
    pwm_config_set_wrap(&config, WRAP);
    pwm_config_set_clkdiv(&config, div);
    pwm_config_set_phase_correct(&config, false);
    pwm_config_set_clkdiv_mode(&config, PWM_DIV_FREE_RUNNING);
    pwm_init(slice, &config, false);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(OUTPUT_PIN), (uint16_t) (duty*(WRAP+1)));
    pwm_set_enabled(slice, true);
}

// Measure frequency
float measure_frequency(uint slice) {

    // Count once for every MEASURE_DIV cycles the PWM B input is high
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_RISING);
    pwm_config_set_clkdiv(&cfg, MEASURE_F_DIV);
    pwm_init(slice, &cfg, false);
    gpio_set_function(MEASURE_PIN, GPIO_FUNC_PWM);

    // This is where the actual count is done
    pwm_set_enabled(slice, true);
    sleep_ms(MEASURE_F_TIME);
    pwm_set_enabled(slice, false);

    // Calculate frequency
    return (pwm_get_counter(slice) * MEASURE_F_DIV * 1000.0f) / MEASURE_F_TIME;
}

// Measure duty cycle
float measure_duty_cycle(uint slice) {

    // Count once for every MEASURE_DIV cycles the PWM B input is high
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_HIGH);
    pwm_config_set_clkdiv(&cfg, MEASURE_C_DIV);
    pwm_init(slice, &cfg, false);
    gpio_set_function(MEASURE_PIN, GPIO_FUNC_PWM);

    // This is where the actual count is done
    pwm_set_enabled(slice, true);
    sleep_ms(MEASURE_C_TIME);
    pwm_set_enabled(slice, false);

    // Calculate duty cycle
    float counting_rate = clock_get_hz(clk_sys) * ((float) MEASURE_C_TIME / 1000.0f);
    float max_possible_count = counting_rate / MEASURE_C_DIV;
    return pwm_get_counter(slice) / max_possible_count;
}

// Main Program
int main() {
    // Init stdio
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("\nPWM Measurement Example\n");

    // Find out which PWM slice is connected to the pins
    uint slice_out = pwm_gpio_to_slice_num(OUTPUT_PIN);
    uint slice_mea = pwm_gpio_to_slice_num(MEASURE_PIN);

    assert(pwm_gpio_to_channel(MEASURE_PIN) == PWM_CHAN_B);

    // Connect PINs to the PWM
    gpio_set_function(OUTPUT_PIN, GPIO_FUNC_PWM);

    // Main loop
    while (true) {
        for (int i = 0; test[i].freq != 0.0; i++) {
            generate_pwm(slice_out, test[i].freq, test[i].duty);
            float freq = measure_frequency(slice_mea);
            float duty = measure_duty_cycle(slice_mea);
            pwm_set_enabled(slice_out, false);
            printf ("Freq  %.2f x %.2f   Duty %.2f x %.2f\n",
                test[i].freq, freq, test[i].duty, duty);
        }
        while (getchar_timeout_us(0) == PICO_ERROR_TIMEOUT) {
            sleep_ms(100);
        }
        printf("\n");
    }
}
