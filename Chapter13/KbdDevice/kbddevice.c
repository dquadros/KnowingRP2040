/**
 * @file kbddevice.c
 * @author Daniel Quadros
 * @brief A five key USB keyboard device
 * @version 0.1
 * @date 2022-06-21
 * 
 * Based in the dev_hid_composite example in the Pico C SDK
 * that is based in the tinyusb hid_boot_interface example
 * 
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Raspberry Pi Pico LED - Used for CAPS LOCK
#define LED_PIN 25

//--------------------------------------------------------------------+
// Keyboard control
//--------------------------------------------------------------------+

// Keys are connect between a pin and ground
uint kbd_pin[] = { 20, 19, 18, 17, 16 };
#define NKEYS (sizeof(kbd_pin)/sizeof(uint))

// USB codes for the keys
uint8_t kbd_code[] = { 0x1E, 0x1F, 0x20, 0x21, HID_KEY_CAPS_LOCK };

// Are the keys pressed?
bool key_pressed[NKEYS];
uint nkeys_pressed = 0;

// Last reported keycodes
uint8_t keycode[6] = { 0 };

//--------------------------------------------------------------------+
// Local routines
//--------------------------------------------------------------------+
void kbd_init(void);
void kbd_check(void);
void hid_task(void);

//--------------------------------------------------------------------+
// Main Program
//--------------------------------------------------------------------+
int main(void)
{
  // Initialize the LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 0);

  // Initialize the "keyboard"
  kbd_init();

  // Initialize the USB Stack
  board_init();
  tusb_init();

  // Main loop
  while (1)
  {
    tud_task();
    hid_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// Keyboard
//--------------------------------------------------------------------+

// Keyboard Initialization
void kbd_init() {
  for (int i = 0; i < NKEYS; i++) {
    uint pin = kbd_pin[i];
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
    key_pressed[i] = false;
  }
}

// Check for keys pressed and released and update global variables
void kbd_check() {
  for (int i = 0; i < NKEYS; i++) {
    bool pressed = !gpio_get(kbd_pin[i]);   // pressed = low
    if (pressed != key_pressed[i]) {
      // changed state
      if (pressed) {
        // Try to put key in report
        for (int j = 0; j < 6; j++) {
          if (keycode[j] == 0) {
            keycode[j] = kbd_code[i];
            key_pressed[i] = true;
            nkeys_pressed++;
            break;
          }
        }
      } else {
        // remove from report
        for (int j = 0; j < 6; j++) {
          if (keycode[j] == kbd_code[i]) {
            keycode[j] = 0;
            key_pressed[i] = false;
            nkeys_pressed--;
            break;
          }
        }
      }
    }
  }
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
// USB HID
//--------------------------------------------------------------------+

// Send the HID report
static void send_hid_report()
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) {
    return;
  }

  // use to avoid send multiple consecutive zero report for keyboard
  static bool notified_key = false;

  if (nkeys_pressed) {
    // We have keys pressed
    tud_hid_keyboard_report(0, 0, keycode);
    notified_key = true;
  } else
  {
    // No key pressed, send empty report just one time
    if (notified_key)  {
      tud_hid_keyboard_report(0, 0, NULL);
      notified_key = false;
    }
  }
}

// Every 10ms, we will sent a report
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  // Check if is time for an update
  if ( (board_millis() - start_ms) < interval_ms) {
    return;
  } 
  start_ms += interval_ms;

  // Check the keys in the keyboard
  kbd_check();

  // Remote wakeup
  if ( tud_suspended() && (nkeys_pressed > 0) )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  } else
  {
    send_hid_report();
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    if (bufsize) {
      // Update the Caps Lock LED
      gpio_put(LED_PIN, (buffer[0] & KEYBOARD_LED_CAPSLOCK)? 1 : 0);
    }
  }
}

