/**
 * @file serialrx.c
 * @author Daniel Quadros
 * @brief Example of using the PIO to receive serial data
 * @version 0.1
 * @date 2022-06-12
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Our PIO program:
#include "serialrx.pio.h"

// Output pin
#define GPIO_DATA_PIN   27


int main() {
    // Init stdio
    stdio_init_all();

    // Choose which PIO instance to use
    PIO pio = pio0;

    // Find a location (offset) in the instruction memory where there is 
    // enough space for our program and load it there
    uint offset = pio_add_program(pio, &serialrx_program);

    // Find a free state machine on our chosen PIO
    // Configure it to run our program and start it, using the
    // helper function we included in our .pio file.
    uint sm = pio_claim_unused_sm(pio, true);
    serialrx_program_init(pio, sm, offset, GPIO_DATA_PIN, 200000.0f);

    // The state machine is now running.
    while (true) {
        // Wait to receive a word
        uint32_t rxword = pio_sm_get_blocking (pio, sm);
        // Print it in stdout
        // The received data will be in the upper 12 bits
        printf ("0x%04X ", rxword);
    }
}

