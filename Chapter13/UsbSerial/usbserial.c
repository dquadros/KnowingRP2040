/**
 * @file usbserial.c
 * @author Daniel Quadros
 * @brief Example of a simple USB Serial Adapter
 * @version 0.1
 * @date 2022-06-20
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"

// Select UART and Pins
#define UART_ID uart0
#define UART_TX_PIN   0
#define UART_RX_PIN   1

// Raspberry Pi Pico LED
#define LED_PIN 25

// Local routines
void serial_init(void);
void cdc_task(void);

// Main Program
int main(void)
{
  // Initialize the LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 0);

  // Initialize the UART
  serial_init();

  // Initialize the USB Stack
  board_init();
  tusb_init();

  // Main loop
  while (1)
  {
    tud_task();
    cdc_task();
  }

  return 0;
}

// UART Initialization
void serial_init() {
    // Set up UART, parameters will be overwritten later
    uart_init(UART_ID, 115200);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);    

    // Set the TX and RX pins
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
}


//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+

// Moves data between USB and UART
// Not optimized!
void cdc_task(void) {
  // connected() check for DTR bit, its assume that the application
  // in the host set it when connecting
  if ( tud_cdc_connected() ) {

    // send trough the USB data received by the UART
    if (uart_is_readable(UART_ID)) {
      while (uart_is_readable(UART_ID) && (tud_cdc_write_available() > 0)) {
        tud_cdc_write_char(uart_getc(UART_ID));
      }
      tud_cdc_write_flush();  // so we don't wait for a full buffer to send
    }

    // send trough the UART data received by the USB
    while (uart_is_writable(UART_ID) && (tud_cdc_available() > 0)) {
      uart_putc_raw(UART_ID, tud_cdc_read_char());
    }
  } else {
    // ignore data received through the UART
    while (uart_is_readable(UART_ID)) {
      uart_getc(UART_ID);
    }

    // ignore data received through the USB
    if (tud_cdc_available() > 0) {
      tud_cdc_read_flush();
    }
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if ( dtr )
  {
    // Terminal connected
    gpio_put(LED_PIN, 1);
  } else
  {
    // Terminal disconnected
    gpio_put(LED_PIN, 0);
  }
}

// Invoked when line coding is change via SET_LINE_CODING
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding) {

  // 0: 1 stop bit - 1: 1.5 stop bits - 2: 2 stop bits
  uint stop_bits = 2;
  if (p_line_coding->stop_bits == 0) {
    stop_bits = 1;
  }

  // 0: None - 1: Odd - 2: Even - 3: Mark - 4: Space
  // TODO: implement Mark & Space parity
  uart_parity_t parity = UART_PARITY_NONE;
  if (p_line_coding->parity == 1) {
    parity = UART_PARITY_ODD;
  } else if (p_line_coding->parity == 2) {
    parity = UART_PARITY_EVEN;
  }

  uart_set_baudrate(UART_ID, p_line_coding->bit_rate);
  uart_set_format(UART_ID, p_line_coding->data_bits, stop_bits, parity);
}

