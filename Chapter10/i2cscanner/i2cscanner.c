/**
 * @file i2cscanner.c
 * @author Daniel Quadros
 * @brief Finding out the addresses of connected I2C devices
 * @version 0.1
 * @date 2022-07-28
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Select I2C and Pins
#define I2C_ID        i2c0
#define I2C_SCL_PIN   17
#define I2C_SDA_PIN   16

// I2C Configuration
#define BAUD_RATE 100000   // standard 100KHz

// Main Program
int main() {
    // Start stdio and wait for USB connection
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Set up I2C
    uint baud = i2c_init (I2C_ID, BAUD_RATE);
    printf ("I2C @ %u Hz\n", baud);
    
    // Set up the I2C pins
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SCL_PIN);
    gpio_pull_up(I2C_SDA_PIN);


    printf("Scanning I2C devices...\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr <= 0x7F; ++addr) {
        if ((addr % 16) == 0) {
            printf("%02x ", addr);
        }


        // scan only non-reserved address
        int ret = PICO_ERROR_GENERIC;
        if (((addr & 0x78) != 0) && ((addr & 0x78) != 0x78)) {
            uint8_t rxdata;
            ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);
        }
        printf(ret < 0 ? "." : "X");
        printf((addr % 16) == 15 ? "\n" : "  ");
    }
    printf("Done.\n");

    // Main loop
    while (1) {
        sleep_ms(1000);
    }
}
