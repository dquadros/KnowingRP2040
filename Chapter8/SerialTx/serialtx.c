/**
 * @file serialtx.c
 * @author Daniel Quadros
 * @brief Example of using the PIO to send data serially
 * @version 0.1
 * @date 2022-06-12
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Our PIO program:
#include "serialtx.pio.h"

// Output pin
#define GPIO_DATA_PIN   28
#define GPIO_CLOCK_PIN  27


int main() {
    // Choose which PIO instance to use
    PIO pio = pio0;

    // Find a location (offset) in the instruction memory where there is 
    // enough space for our program and load it there
    uint offset = pio_add_program(pio, &serialtx_program);

    // Find a free state machine on our chosen PIO
    // Configure it to run our program, and start it, using the
    // helper function we included in our .pio file.
    uint sm = pio_claim_unused_sm(pio, true);
    serialtx_program_init(pio, sm, offset, GPIO_DATA_PIN, GPIO_CLOCK_PIN, 200000.0f);

    // The state machine is now running.
    while (true) {
        // Send same test value we can look at using an osciloscope
        pio_sm_put_blocking (pio, sm, 0x0111);
        // Insert a small pause between data
        sleep_ms(1);
    }
}

