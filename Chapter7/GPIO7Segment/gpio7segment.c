/**
 * @file gpio7segment.c
 * @author Daniel Quadros
 * @brief Example of using the GPIO in the RP2040 to drive a
 *        4 digit 7 segment common anode display
 * @version 0.1
 * @date 2022-07-12
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
 
// Display connections
// Segments:  A:6 B:4 C:1 D:2 E:3 F:5 G:0
// Digits:    1:7 2:8 3:9 4:10
#define SEGMENTS_MASK   0x0007F
#define DIGITS_MASK     0x00780
#define DIGIT_1         7
#define DIGIT_2         8
#define DIGIT_3         9
#define DIGIT_4         10
 
// Digit selection GPIOs
int digit[] = { DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4 };
 
// What segments to turn on for each digit
int segments[] = { // AFB EDCG   0 = on, 1 = off
    0x01,        // 000 0001     --A--
    0x6D,        // 110 1101     F   B
    0x22,        // 010 0010     --G--
    0x28,        // 010 1000     E   C
    0x4C,        // 100 1100     --D--
    0x18,        // 001 1000
    0x10,        // 001 0000
    0x2D,        // 010 1101
    0x00,        // 000 0000
    0x08         // 000 1000
 };
 
// Timer to update the display
struct repeating_timer timer;
 
// Value to show on display
volatile int value[4];
 
// Local routines
static void init(void);
static bool updateDisplay(struct repeating_timer *t);
 
// Main Program
int main() {
    init();
    while (1) {
        // Increment value
        int i = 3;
        while ((i >= 0) && (value[i]==9)) {
            value[i] = 0;
            i--;
        }
        if (i >= 0) {
            value[i]++;
        }
        // Wait 1 second
        sleep_ms(1000);
    }
    return 0;
}
 
// Initialization
void init() {
    int i;
 
    // GPIO init
    gpio_init_mask (SEGMENTS_MASK | DIGITS_MASK);
    gpio_set_dir_masked (SEGMENTS_MASK | DIGITS_MASK, SEGMENTS_MASK | DIGITS_MASK);
    for (i = 0; i < 4; i++) {
        gpio_set_drive_strength (digit[i], GPIO_DRIVE_STRENGTH_12MA);
    }
     
    // Update a digit every 5 miliseconds
    add_repeating_timer_ms(5, updateDisplay, NULL, &timer);
}
 
// Update Display
bool updateDisplay(struct repeating_timer *t) {
    static int nDig = 3;
 
    gpio_put (digit[nDig], false); // turn off previous digit
    nDig = (nDig + 1) & 3;  // moves on to next digit
     
    // set up segments
    gpio_put_masked (SEGMENTS_MASK, segments[value[nDig]]); 
     
    gpio_put (digit[nDig], true); // turns on current digit
 
    return true;    // keep calling this routine
}