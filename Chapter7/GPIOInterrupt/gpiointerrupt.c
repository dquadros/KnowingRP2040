/**
 * @file gpiointerrupt.c
 * @author Daniel Quadros
 * @brief Example of using GPIO interrupts in the RP2040
 * @version 0.1
 * @date 2022-10-19
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <time.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
 
 #define millis() to_ms_since_boot(get_absolute_time())
 
//  A button is connected betweem this pin and ground
#define BUTTON_PIN 16

// Structure to represent a GPIO event
typedef struct {
	uint32_t event_mask;
	uint32_t event_time;
} GPIO_EVENT;

// GPIO event queue
#define MAX_EVENTS 100
static GPIO_EVENT event_queue[MAX_EVENTS+1];
static volatile int event_in = 0;  // where to place next event
static int event_out = 0;  // where to take out next event


// Interrupt  handler
void gpio_interrupt (uint gpio, uint32_t events) {
	// set up the event information
	event_queue[event_in].event_mask = events;
	event_queue[event_in].event_time = millis();
	// check if there is space for it in the queue
	int aux = event_in + 1;
	if (aux > MAX_EVENTS) {
		aux = 0;
	}
	if (aux != event_out) {
		// Ok, advance the input index
		event_in = aux;
	}
}


// Main Program
int main() {
	
    // Init stdio0
    stdio_init_all();

    // Init the button pin
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
	gpio_set_input_hysteresis_enabled(BUTTON_PIN, true);
	
    //  Atach our callback to the gpio interrupt
    gpio_set_irq_callback (gpio_interrupt);
    gpio_set_irq_enabled(BUTTON_PIN,  GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,  true);
	irq_set_enabled(IO_IRQ_BANK0, true);

    // main loop
    while (1) {
		// Print out recorded events
		while (event_in != event_out) {
			// print an event
			printf ("%7d %s %s\n",  event_queue[event_out].event_time,
				(event_queue[event_out].event_mask & GPIO_IRQ_EDGE_FALL) ?
					"PRESS" : "     ",
				(event_queue[event_out].event_mask & GPIO_IRQ_EDGE_RISE) ?
					"RELEASE" : ""
			);
			// remove it from the queue
			int aux = event_out;
			if (++aux > MAX_EVENTS) {
				aux = 0;
			}
			event_out = aux;
		}
        sleep_ms(10);
    }
    return 0;
}
