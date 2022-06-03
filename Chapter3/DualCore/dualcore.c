/**
 * @file dualcore.c
 * @author Daniel Quadros
 * @brief Example of using the two ARM cores in the RP2040
 *        A mutex is used to control usage of the ADC
 *        An interprocessor FIFO is used to pass data between the cores
 * @version 0.1
 * @date 2022-06-03
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// Where the LDR is connected
#define GPIO_LDR        28
#define ADC_INPUT_LDR   2

// Internal temperature sensor
#define ADC_INPUT_TEMPSENSOR 4

// Mutex for ADC
mutex_t adc_mutex;

// Factor to convert ADC reading to voltage
// Assumes 12-bit, ADC_VREF = 3.3V
const float conversionFactor = 3.3f / (1 << 12);

// This rotine will run in core 1
void readRpTemp() {
    while(1) {
        // Get access to the ADC
        mutex_enter_blocking(&adc_mutex);

        // Select ADC input and read temperature sensor voltage
        adc_select_input(ADC_INPUT_TEMPSENSOR);
        adc_read(); // throw away first reading after changing input
        uint16_t adc = adc_read();

        // Release the ADC
        mutex_exit(&adc_mutex);

        // Convert reading to temperature in units of 0.1 C
        float tempC = 27.0f - (adc*conversionFactor - 0.706f) / 0.001721f;
        int32_t tempDC = (int32_t) ((tempC * 10.0f) + 0.5f);

        // Pass the value to the other core
        multicore_fifo_push_blocking(tempDC);
    }
}

// Main Program
int main() {
    // Init stdio
    stdio_init_all();
    printf("\nDual Core Example\n");

    // Init ADC
    adc_init();
    adc_set_temp_sensor_enabled(true);
    mutex_init (&adc_mutex);

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(GPIO_LDR);
    
    // Start other core
    multicore_launch_core1(readRpTemp);

    // Main loop
    const int MAX_COUNT = 10000;
    int count = 0;
    float tempSum = 0.0f;
    float ldrSum = 0.0f;
    while (1) {
        // Get access to the ADC
        mutex_enter_blocking(&adc_mutex);

        // Select ADC input and read LDR voltage
        adc_select_input(ADC_INPUT_LDR);
        adc_read(); // throw away first reading after changing input
        uint16_t adc = adc_read();

        // Release the ADC
        mutex_exit(&adc_mutex);

        // Convert reading to voltage and accumulate
        ldrSum += adc * conversionFactor;

        // Get a temperature reading and accumulate
        tempSum += multicore_fifo_pop_blocking()*0.1f;

        // Print out the averages after MAX_COUNT readings 
        if (++count == MAX_COUNT) {
            printf("LDR voltage: %.2f V  Temperature: %.2f\n", ldrSum/MAX_COUNT, tempSum/MAX_COUNT);
            count = 0;
            tempSum = 0.0f;
            ldrSum = 0.0f;
        }
    }
}