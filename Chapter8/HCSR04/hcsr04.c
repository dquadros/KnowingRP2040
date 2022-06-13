/**
 * @file hcsr04.c
 * @author Daniel Quadros
 * @brief Example of using the PIO to interface an HC-SR04 sensor
 * @version 0.1
 * @date 2022-06-13
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Our PIO program:
#include "hcsr04.pio.h"

// Output pin
#define GPIO_TRIGGER_PIN   27
#define GPIO_ECHO_PIN      28
#define ECHO_TIMEOUT_US    150000

int main() {
    // Init stdio
    stdio_init_all();

    // Choose which PIO instance to use
    PIO pio = pio0;

    // Find a location (offset) in the instruction memory where there is 
    // enough space for our program and load it there
    uint offset = pio_add_program(pio, &hcsr04_program);

    // Find a free state machine on our chosen PIO
    // Configure it to run our program and start it, using the
    // helper function we included in our .pio file.
    uint sm = pio_claim_unused_sm(pio, true);

    while (true) {
        printf ("Reading... ");
        // Init and start the state machine
        hcsr04_program_init(pio, sm, offset, GPIO_TRIGGER_PIN, GPIO_ECHO_PIN);
        // Start a reading
        pio_sm_put (pio, sm, ECHO_TIMEOUT_US);
        // Get the result (remaing microseconds from the timeout)
        uint32_t val = pio_sm_get_blocking (pio, sm);
        // Stop the state machine
        pio_sm_set_enabled (pio, sm, false);

        // Print the distance
        printf ("Distance: %.1f cm\n", ((ECHO_TIMEOUT_US - val)*0.0343f)/2.0f);

        // Wait 2 seconds before next reading
        sleep_ms(2000);
    }
}

