/**
 * @file uartsum.c
 * @author Daniel Quadros
 * @brief Example of using the UART
 * @version 0.1
 * @date 2022-06-17
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

// Select UART and Pins
#define UART_ID uart0
#define UART_TX_PIN   0
#define UART_RX_PIN   1

// UART Configuration
#define BAUD_RATE 300
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// UART interrupt requuest
int UART_IRQ;

// Current number and sum
volatile int number;
volatile bool number_received = false;
volatile int sum = 0;

// Rx interrupt handler
void on_uart_rx() {
    // There can be multiple chars in the FIFO
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);

        if (ch == 0x0D) {
            // A number was entered
            // disable interrupt and signal number received
            irq_set_enabled(UART_IRQ, false);
            number_received = true;
            break;
        } else if ((ch >= '0') && (ch <= '9')) {
            // Update number, limit to 4 digits
            number = (number*10  + ch - '0') % 10000;
            // Echo the digit
            if (uart_is_writable(UART_ID)) {
                uart_putc(UART_ID, ch);
            }
        }
    }
}

// Main Program
int main() {
    char msg[30];  // Buffer for sum message

    // Set up UART
    uart_init(UART_ID, BAUD_RATE);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, true);    

    // Set the TX and RX pins
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Set up and enable receive interrupt
    UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    // Main loop
    while (1) {
        if (number_received) {
            // update sum
            sum = (sum + number) % 1000000; // limit to 6 digits

            // set up the sum message
            sprintf (msg, " Sum:%d\r\n", sum);

            // send sum 
            uart_puts(UART_ID, msg);

            // wait for space in the Tx FIFO, so we can echo received chars
            while (!uart_is_writable(UART_ID)) {
            }

            // get ready to receive another number
            number = 0;
            number_received = false;
            irq_set_enabled(UART_IRQ, true);
        }
    }

}

