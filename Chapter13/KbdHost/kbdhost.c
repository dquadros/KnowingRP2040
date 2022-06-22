/**
 * @file kbdhost.c
 * @author Daniel Quadros
 * @brief A USB keyboard host
 * @version 0.1
 * @date 2022-06-21
 *
 * Based in the host_cdc_msc_hid example in the Pico C SDK
 * that is based in the tinyusb cdc_msc_hid example
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
#include "hardware/uart.h"

// Select UART and Pins
#define UART_ID uart0
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// keycodes translation table
#define NKEYS 128
static uint8_t const keycode2ascii[NKEYS][2] = {HID_KEYCODE_TO_ASCII};

#define MAX_KEY 6 // Maximun number of pressed key in the boot layout report

// Caps lock control
static bool capslock_key_down_in_last_report = false;
static bool capslock_key_down_in_this_report = false;
static bool capslock_on = false;

// Keyboard LED control
static uint8_t leds = 0;
static uint8_t prev_leds = 0xFF;

// Keyboard address and instance (assumes there is only one)
static uint8_t keybd_dev_addr = 0xFF;
static uint8_t keybd_instance;

// Each HID instance has multiple reports
#define MAX_REPORT 4
static uint8_t _report_count[CFG_TUH_HID];
static tuh_hid_report_info_t _report_info_arr[CFG_TUH_HID][MAX_REPORT];

//--------------------------------------------------------------------+
// Local routines
//--------------------------------------------------------------------+
void serial_init(void);
void hid_task(void);
static void process_kbd_report(hid_keyboard_report_t const *report);

//--------------------------------------------------------------------+
// Main Program
//--------------------------------------------------------------------+
int main(void)
{
  // Initialize the UART
  serial_init();

  // Initialize the USB Stack
  board_init();
  tusb_init();

  // Main loop
  while (1)
  {
    tuh_task();
    hid_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// UART Initialization
//--------------------------------------------------------------------+
void serial_init()
{
  // Set up UART, parameters will be overwritten later
  uart_init(UART_ID, 115200);
  uart_set_hw_flow(UART_ID, false, false);
  uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
  uart_set_fifo_enabled(UART_ID, false);

  // Set the TX and RX pins
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

//--------------------------------------------------------------------+
// This will be called by the main loop
//--------------------------------------------------------------------+
void hid_task(void)
{
  // update keyboard leds
  if (keybd_dev_addr != 0xFF)
  { // only if keyboard attached
    if (leds != prev_leds)
    {
      tuh_hid_set_report(keybd_dev_addr, keybd_instance, 0, HID_REPORT_TYPE_OUTPUT, &leds, sizeof(leds));
      prev_leds = leds;
    }
  }
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len)
{
  // Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
  // can be used to parse common/simple enough descriptor.
  _report_count[instance] = tuh_hid_parse_report_descriptor(_report_info_arr[instance], MAX_REPORT, desc_report, desc_len);
  // Check if at least one of the reports is for a keyboard
  for (int i = 0; i < _report_count[instance]; i++)
  {
    if ((_report_info_arr[instance][i].usage_page == HID_USAGE_PAGE_DESKTOP) &&
        (_report_info_arr[instance][i].usage == HID_USAGE_DESKTOP_KEYBOARD))
    {
      keybd_dev_addr = dev_addr;
      keybd_instance = instance;
    }
  }

  // request to receive report
  tuh_hid_receive_report(dev_addr, instance);
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  keybd_dev_addr = 0xFF; // keyboard not available
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
  uint8_t const rpt_count = _report_count[instance];
  tuh_hid_report_info_t *rpt_info_arr = _report_info_arr[instance];
  tuh_hid_report_info_t *rpt_info = NULL;

  if ((rpt_count == 1) && (rpt_info_arr[0].report_id == 0))
  {
    // Simple report without report ID as 1st byte
    rpt_info = &rpt_info_arr[0];
  }
  else
  {
    // Composite report, 1st byte is report ID, data starts from 2nd byte
    uint8_t const rpt_id = report[0];

    // Find report id in the arrray
    for (uint8_t i = 0; i < rpt_count; i++)
    {
      if (rpt_id == rpt_info_arr[i].report_id)
      {
        rpt_info = &rpt_info_arr[i];
        break;
      }
    }

    report++;
    len--;
  }

  if (rpt_info && (rpt_info->usage_page == HID_USAGE_PAGE_DESKTOP))
  {
    switch (rpt_info->usage)
    {
    case HID_USAGE_DESKTOP_KEYBOARD:
      // Assume keyboard follow boot report layout
      process_kbd_report((hid_keyboard_report_t const *)report);
      break;

    default:
      break;
    }
  }

  // continue to request to receive report
  tuh_hid_receive_report(dev_addr, instance);
}

//--------------------------------------------------------------------+
// Keyboard
//--------------------------------------------------------------------+

// look up key in a report
static inline bool find_key_in_report(hid_keyboard_report_t const *report, uint8_t keycode)
{
  for (uint8_t i = 0; i < MAX_KEY; i++)
  {
    if (report->keycode[i] == keycode)
    {
      return true;
    }
  }

  return false;
}

// process keyboard report
static void process_kbd_report(hid_keyboard_report_t const *report)
{
  static hid_keyboard_report_t prev_report = {0, 0, {0}}; // previous report to check key released

  // Check caps lock
  capslock_key_down_in_this_report = find_key_in_report(report, HID_KEY_CAPS_LOCK);
  if (capslock_key_down_in_this_report && !capslock_key_down_in_last_report)
  {
    // CAPS LOCK was pressed
    capslock_on = !capslock_on;
    if (capslock_on)
    {
      leds |= KEYBOARD_LED_CAPSLOCK;
    }
    else
    {
      leds &= ~KEYBOARD_LED_CAPSLOCK;
    }
  }

  // check other pressed keys
  for (uint8_t i = 0; i < MAX_KEY; i++)
  {
    uint8_t key = report->keycode[i];
    if ((key != 0) && (key != HID_KEY_CAPS_LOCK) && !find_key_in_report(&prev_report, key))
    { // ignore fillers, Caps lock and keys already pressed
      // Find corresponding ASCII code
      uint8_t ch = (key < NKEYS) ? keycode2ascii[key][0] : 0; // unshifted key code, to test for letters
      bool const is_ctrl = report->modifier & (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_RIGHTCTRL);
      bool is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
      if (capslock_on && (ch >= 'a') && (ch <= 'z'))
      {
        // capslock affects only letters
        is_shift = !is_shift;
      }
      ch = (key < NKEYS) ? keycode2ascii[key][is_shift ? 1 : 0] : 0;
      if (is_ctrl)
      {
        // control char
        if ((ch >= 0x60) && (ch <= 0x7F))
        {
          ch = ch - 0x60;
        }
        else if ((ch >= 0x40) && (ch <= 0x5F))
        {
          ch = ch - 0x40;
        }
      }

      if (ch)
      {
        // send key code to UART
        uart_putc_raw(UART_ID, ch);
      }
    }
  }

  // save current status
  prev_report = *report;
  capslock_key_down_in_last_report = capslock_key_down_in_this_report;
}
