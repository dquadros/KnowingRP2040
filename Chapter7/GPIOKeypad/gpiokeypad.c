/**
 * @file gpio7segment.c
 * @author Daniel Quadros
 * @brief Example of using the GPIO in the RP2040 to
 *        read a 4x4 matrix keypad
 * @version 0.1
 * @date 2022-07-14
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"

// GPIOs
// Rows:    GPIO10 to GPIO13
// Columns: GPIO18 to GPIO21
#define nRows    4
#define nColumns 4
static const int firstRow = 10;
static const int firstColumn = 18;

// GPIO masks
static uint32_t rowMask;
static uint32_t columnMask;

// Timer to scan the keypad
static struct repeating_timer timer;
 
// Columns readings
static const int DEBOUNCE = 5;
static uint32_t kp_debounced[nRows];
static uint32_t kp_debouncing[nRows];
static int debunceCounter[nRows];

// Keys queue
#define sizeQueue 5
static int inQueue = 0, outQueue = 0;
static char queue[sizeQueue+1];

// Kepad decoding
static int decod[nRows][nColumns] = {
    {  '1', '2', '3', 'A' },
    {  '*', '0', '#', 'D' },
    {  '7', '8', '9', 'C' },
    {  '4', '5', '6', 'B' }
};

// Local routines
static uint32_t buildMask(int first, int n);
static void init(void);
static bool scanKeypad(struct repeating_timer *t);
static int readKey(void);
 
// Main Program
int main() {
    // Init stdio
    stdio_init_all();
    #ifdef LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    #endif

    printf("\nKeypad GPIO Input Example\n");
    init();
    while (1) {
        int key = readKey();
        if (key != -1) {
            printf ("Key = %c\n", key);
        }
        sleep_ms(1);
    }
    return 0;
}

// Utility routine to build a mask for 'n' pins starting from 'first'
static uint32_t buildMask(int first, int n) {
    uint32_t mask = 0;
    for (int i = 0; i < n; i++) {
        mask |= 1 << (first+i);
    }
    return mask;
}

// Initialization
static void init() {
 
    // Build masks
    rowMask = buildMask (firstRow, nRows);
    columnMask = buildMask (firstColumn, nColumns);

    // GPIO init
    gpio_init_mask (rowMask | columnMask);
    gpio_set_dir_masked (rowMask | columnMask, rowMask);
    for (int i = 0; i < nColumns; i++) {
        gpio_pull_down(firstColumn+i);
    }

    // Scan keypad every 10 miliseconds
    add_repeating_timer_ms(10, scanKeypad, NULL, &timer);
}
 
// Scan the current row of the keypad
static bool scanKeypad(struct repeating_timer *t) {
    static int curRow = firstRow;
    static int countRow = 0;

    // Turn on current row
    gpio_put_masked (rowMask, 1 << curRow);

    // Read columns
    uint32_t current = gpio_get_all() & columnMask;

    // Debounce
    if (current != kp_debouncing[countRow]) {
        // reading changed, start debouncing again
        kp_debouncing[countRow] = current;
        debunceCounter[countRow] = 0;
    } else if (debunceCounter[countRow] <= DEBOUNCE) {
        if (debunceCounter[countRow] == DEBOUNCE) {
            // consider value stable
            if (kp_debounced[countRow] != current) {
                // Find key pressed
                uint32_t dif = kp_debounced[countRow] ^ current;
                int i = 0;
                while (i < nColumns) {
                    uint32_t mask = 1 << (i+firstColumn);
                    if (((dif & mask) != 0) && ((current & mask) != 0)) {
                        // there is a change and key is pressed
                        break;
                    }
                    i++;
                }
                if (i < nColumns) {
                    int key = decod[countRow][i];
                    int aux = inQueue+1;
                    if (aux > sizeQueue) {
                        aux = 0;
                    }
                    if (aux != outQueue) {
                        queue[inQueue] = key;
                        inQueue = aux;
                    } else {
                        // queue is full, ignore key
                    }
                }
                // uodate debounced status
                kp_debounced[countRow] = current;
            }
        }
        debunceCounter[countRow]++;
    }

    // Move on to next row
    if (++countRow == nRows) {
        countRow = 0;
        curRow = firstRow;
    } else {
        curRow++;
    }

    return true; // keep executing
}


// Read a key from the key queue, returns -1 if queue empty
static int readKey(void) {
    int key = -1;
    if (inQueue != outQueue) {
        key = queue[outQueue];
        if (outQueue++ == sizeQueue) {
            outQueue = 0;
        }
    }
    return key;
}