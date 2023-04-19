/**
 * @file pioint.c
 * @author Daniel Quadros
 * @brief Example of using PIO interrupts
 * @version 0.1
 * @date 2022-08-17
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include "stdio.h"
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Our PIO program:
#include "pioint.pio.h"

// Flag to signal interrupts received
volatile int intRx = 0;

// critical section for accessing the flag
critical_section_t cs_intRx;

// PIO and State Machines
PIO pio = pio0;
int sm1, sm2;

// Rx interrupt handler for sm1
void on_sm1_int() {
    if (pio_interrupt_get(pio, sm1)) {
        pio_interrupt_clear(pio, sm1);
        intRx |= 1;
    }
}

// Rx interrupt handler for sm2
void on_sm2_int() {
    if (pio_interrupt_get(pio, sm2)) {
        pio_interrupt_clear(pio, sm2);
        intRx |= 2;
    }
}


// Main routine
int main() {
    // Start stdio and wait for USB connection
    stdio_init_all();
    #ifdef LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    #endif
    printf ("PIO Interrupt demo\n");

    // Init the critical section
    critical_section_init (&cs_intRx);

    // Find a location (offset) in the instruction memory where there is 
    // enough space for our program and load it there
    uint offset = pio_add_program(pio, &pioint_program);

    // Find a free state machine on our chosen PIO
    // Configure it to run our program, and start it, using the
    // helper function we included in our .pio file.
    sm1 = pio_claim_unused_sm(pio, true);
    pioint_program_init(pio, sm1, offset, 200000.0f);
    printf ("SM1 = %d\n", sm1);

    // Find another free state machine on our chosen PIO
    // Configure it to run our program, and start it, using the
    // helper function we included in our .pio file.
    sm2 = pio_claim_unused_sm(pio, true);
    pioint_program_init(pio, sm2, offset, 200000.0f);
    printf ("SM2 = %d\n", sm2);

    // Set up the interrupt handlers
    irq_add_shared_handler(PIO0_IRQ_0, on_sm1_int, 1);
    irq_add_shared_handler(PIO0_IRQ_0, on_sm2_int, 2);
    irq_set_enabled(PIO0_IRQ_0, true);

    // The state machines are now running.
    // Set the delays and start interrupts
    pio_sm_put_blocking (pio, sm1, 400000);
    pio_sm_put_blocking (pio, sm2, 700000);

    // Loop testing for interrupts
    int flags;
    while (true) {
        sleep_ms(1);

        // Copy and clear interrupt flag
        critical_section_enter_blocking(&cs_intRx);
        flags = intRx;
        intRx &= ~3;
        critical_section_exit(&cs_intRx);

        // Print message if interrupt received
        if (flags & 1) {
            printf ("<< INT SM1 >>\n");
        }
        if (flags & 2) {
            printf ("<< INT SM2 >>\n");
        }
    }
}
