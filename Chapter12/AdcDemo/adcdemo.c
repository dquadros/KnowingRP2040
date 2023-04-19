/**
 * @file adcdemo.c
 * @author Daniel Quadros
 * @brief Example of using the ADC in the RP2040 to read
 *        the internal temperature sensor and a externa light sensor
 * @version 0.1
 * @date 2022-07-06
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// Where the LDR is connected
#define GPIO_LDR        28
#define ADC_INPUT_LDR   2

// Internal temperature sensor
#define ADC_INPUT_TEMPSENSOR 4

// Factor to convert ADC reading to voltage
// Assumes 12-bit, ADC_VREF = 3.3V
const float conversionFactor = 3.3f / (1 << 12);

// Main Program
int main() {
    // Init stdio
    stdio_init_all();
    #ifdef LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    #endif
    printf("\nADC Example\n");

    // Init ADC
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_set_round_robin ((1 << ADC_INPUT_TEMPSENSOR) | (1 << ADC_INPUT_LDR));
    adc_select_input (ADC_INPUT_LDR);

    // Reduce the sampling to 1 ms between readings
    float clkdiv = 0.001f * 48000000.0f - 1;
    adc_set_clkdiv(clkdiv);
    adc_fifo_setup (true, false, 0, false, false);

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(GPIO_LDR);
    
    // Start the ADC
    adc_run(true);

    // Main loop
    const int MAX_COUNT = 500;
    float tempSum, ldrSum;
    while (1) {
        tempSum = 0.0f;
        ldrSum = 0.0f;
        for (int count = 0; count < MAX_COUNT; count++) {
            ldrSum +=  adc_fifo_get_blocking() * conversionFactor;
            tempSum +=  adc_fifo_get_blocking() * conversionFactor;
        }
        float ldrV = ldrSum/MAX_COUNT;
        float tempC = 27.0f - (tempSum/MAX_COUNT - 0.706f) / 0.001721f;

        // Print out the averages
        printf("LDR voltage: %.2f V  Temperature: %.2f\n", ldrV, tempC);
    }
}
