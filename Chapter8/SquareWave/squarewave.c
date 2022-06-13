/**
 * @file squarewave.c
 * @author Daniel Quadros
 * @brief Example of using the PIO to generate a square wave
 * @version 0.1
 * @date 2022-06-11
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Our PIO program:
#include "squarewave.pio.h"

// Output pin
#define GPIO_WAVE_OUT   28


int main() {
    // Choose which PIO instance to use (there are two instances)
    PIO pio = pio0;

    // Find a location (offset) in the instruction memory where there is 
    // enough space for our program and load it there
    uint offset = pio_add_program(pio, &sqwave_program);

    // Find a free state machine on our chosen PIO
    // Configure it to run our program, and start it, using the
    // helper function we included in our .pio file.
    uint sm = pio_claim_unused_sm(pio, true);
    sqwave_program_init(pio, sm, offset, GPIO_WAVE_OUT, 1000000.0f);

    // The state machine is now running.
    while (true) {
        sleep_ms(500);
    }
}

