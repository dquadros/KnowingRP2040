/**
 * @file i2c24c32.c
 * @author Daniel Quadros
 * @brief Accessing a 24C32 EEProm using I2C
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

// EEProm
#define EEPROM_ADDR 0x50
#define PAGE_SIZE   32

// Main Program
int main() {
    // Start stdio and wait for USB connection
    stdio_init_all();
    #ifdef LIB_PICO_STDIO_USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    #endif

    // Set up I2C
    uint baud = i2c_init (I2C_ID, BAUD_RATE);
    printf ("I2C @ %u Hz\n", baud);
    
    // Set up the I2C pins
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SCL_PIN);
    gpio_pull_up(I2C_SDA_PIN);


    printf("I2C Example: 24C32 EEPROM\n");

    // Fill the first 256 bytes with 0x00 to 0xFF, using Page Write
    uint8_t value = 0;
    uint8_t buffer[PAGE_SIZE+2];
    for (uint16_t addr = 0; addr < 0xFF; addr += PAGE_SIZE) {
        // Write a page
        printf ("\rWriting at 0x%02X", addr);
        buffer[0] = addr >> 8;
        buffer[1] = addr & 0xFF;
        for (int i = 0; i < PAGE_SIZE; i++) {
            buffer[i+2] = value++;
        }
        int ret = i2c_write_blocking (I2C_ID, EEPROM_ADDR, buffer, PAGE_SIZE+2, false);
        if (ret == (PAGE_SIZE+2)) {
            // Wait for write to complete
            // 24C32 will acknoledge address only when writting finished
            while (i2c_read_blocking(I2C_ID, EEPROM_ADDR, buffer, 1, false) != 1) {
                sleep_ms(1);
            }
        } else {
            printf ("*** Something went wrong ***\n");
        }
    }
    printf ("\rWriting concluded.\n");

    // Dump the first 256 bytes using sequential read
    printf ("Reading EEPROM:\n");
    uint8_t bufferRx[16];
    for (uint16_t addr = 0; addr < 0xFF; addr += 16) {
        buffer[0] = addr >> 8;
        buffer[1] = addr & 0xFF;
        int ret = i2c_write_blocking (I2C_ID, EEPROM_ADDR, buffer, 2, true);
        if (ret == 2) {
            ret = i2c_read_blocking(I2C_ID, EEPROM_ADDR, bufferRx, 16, false);
            if (ret == 16) {
                printf ("0x%02X:", addr);
                for (int i = 0; i < 16; i++) {
                    printf (" %02X", bufferRx[i]);
                }
                printf ("\n");
            }
        }
    }
    printf("Done.\n");

    // Main loop
    while (1) {
        sleep_ms(1000);
    }
}
