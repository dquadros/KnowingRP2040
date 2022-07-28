/**
 * @file adxl345.c
 * @author Daniel Quadros
 * @brief Example of using the SPI to interface an ADXL345 accelerometer
 *        Details on the ADXL345 can be found in its datasheet
 * @version 0.1
 * @date 2022-07-27
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

// Select SPI and Pins
#define SPI_ID spi0
#define SPI_SCLK_PIN   18
#define SPI_MISO_PIN   16
#define SPI_MOSI_PIN   19
#define SPI_SS_PIN     17

// SPI Configuration
#define BAUD_RATE 1000000   // 1 MHz
#define DATA_BITS 8

// ADXL345 Registers
#define DEVID         0x00
#define BW_RATE       0x2C
#define POWER_CTL     0x2D
#define DATA_FORMAT   0x31
#define DATAX0        0x32

// This bits are ORed to the register address
#define READ_BIT      0x80      // this is a read
#define MULTI_BIT     0x40      // multiple bytes are transfered

// Structure to hold raw accleration values
typedef struct 
{
  int x;
  int y;
  int z;
} AccelRaw;


// Local routines
static void ADXL345_init (void);
static uint8_t ADXL345_readId(void);
static void ADXL345_readAccel(AccelRaw *raw);

// Assert the SS signal
static inline void ss_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(SPI_SS_PIN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

// Remove SS signal
static inline void ss_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(SPI_SS_PIN, 1);
    asm volatile("nop \n nop \n nop");
}

// Main Program
int main() {
    // Start stdio and wait for USB connection
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Hello, ADXL345!\n");

    // Set up the SS pin
    gpio_init(SPI_SS_PIN);
    gpio_set_dir(SPI_SS_PIN, GPIO_OUT);
    gpio_put(SPI_SS_PIN, 1);

    // Set up SPI
    uint baud = spi_init (SPI_ID, BAUD_RATE);
    printf ("SPI @ %u Hz\n", baud);
    spi_set_format (SPI_ID, DATA_BITS, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    // Set up the SPI pins
    gpio_set_function(SPI_SCLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI_PIN, GPIO_FUNC_SPI);

    // Init the ADXL345
    ADXL345_init ();

    // Report ADXL345 identification
    printf ("ID = %o\n", ADXL345_readId());

    // Main loop
    AccelRaw raw;
    while (1) {
        ADXL345_readAccel(&raw);
        printf ("Accel X=%d Y=%d Z=%d\n", raw.x, raw.y, raw.z);
        sleep_ms(1000);
    }

}

// Initialize ADXL345
static void ADXL345_init () {
    uint8_t buf[2];

    // Turn off LOW_POWER and select sample rate
    buf[0] = BW_RATE;
    buf[1] = 0x0F;      // Maximum sample rate
    ss_select();
    spi_write_blocking(SPI_ID, buf, 2);
    ss_deselect();

    // Select data format
    buf[0] = DATA_FORMAT;
    buf[1] = 0x0B;  //4wire SPI  +/- 16g range, 13-bit resolution
    ss_select();
    spi_write_blocking(SPI_ID, buf, 2);
    ss_deselect();

    // Start measurements
    buf[0] = POWER_CTL;
    buf[1] = 0x08;
    ss_select();
    spi_write_blocking(SPI_ID, buf, 2);
    ss_deselect();
}

// Reads ADXL345 identification
static uint8_t ADXL345_readId() {
    uint8_t bufTx[] = { DEVID | READ_BIT, 0x00 };
    uint8_t bufRx[2] = { 0x55, 0x55 };

    ss_select();
    spi_write_read_blocking (SPI_ID, bufTx, bufRx, 2);
    ss_deselect();

    return bufRx[1];
}

// Reads raw acceleration data
static void ADXL345_readAccel(AccelRaw *raw) {
    uint8_t  selReg[] =  { DATAX0 | READ_BIT | MULTI_BIT};
    uint8_t buf[6];


    ss_select();
    spi_write_blocking (SPI_ID, selReg, 1);     // Selects first register
    spi_read_blocking (SPI_ID, 0x00, buf, 6);   // Reads 6 registers
    ss_deselect();

  raw->x = (((int)buf[1]) << 8) | buf[0];
  raw->y = (((int)buf[3]) << 8) | buf[2];
  raw->z = (((int)buf[5]) << 8) | buf[4];
}
