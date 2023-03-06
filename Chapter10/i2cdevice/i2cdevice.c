/**
 * @file i2device.c
 * @author Daniel Quadros
 * @brief  Implements an I2C (slave) device
 *         For testing this program also includes a I2C master
 * @version 0.1
 * @date 2023-03-03
 * 
 * @copyright Copyright (c) 2023, Daniel Quadros
 * 
 * Part of the slave code was adapted from the slave_mem_i2c pico example by
 * Valentin Milea <valentin.milea@gmail.com> and  Raspberry Pi (Trading) Ltd.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "pico/util/datetime.h"
#include "hardware/i2c.h"
#include "hardware/rtc.h"
#include <pico/i2c_slave.h>

// Semaphore to indicate that the device is ready
static semaphore_t sem_device;

// Select I2C and Pins
// You'll need to wire pin GP4 to GP6 (SDA), and pin GP5 to GP7 (SCL).
#define I2C_MASTER_ID        i2c0
#define I2C_MASTER_SDA_PIN   4
#define I2C_MASTER_SCL_PIN   5

#define I2C_SLAVE_ID        i2c1
#define I2C_SLAVE_SDA_PIN   6
#define I2C_SLAVE_SCL_PIN   7

// I2C Configuration
#define I2C_BAUDRATE 100000u   // standard 100KHz

// I2C Device Address
static const uint I2C_SLAVE_ADDRESS = 0x1F;

// We will update the RTC in the background
// (just for fun, it is not really necessary)
static volatile uint8_t update = false;
static volatile uint8_t updating = false;
static datetime_t dtNew;

// The first byte written in a transaction is a command
#define CMD_READ_UPDATING 0
#define CMD_READ_RTC      1
#define CMD_WRITE_RTC     2

// I2C handler for the device
// Our handler is called from the I2C ISR, so it must return quickly.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    static uint8_t waitCmd = true;
    static uint8_t curCmd = CMD_READ_UPDATING;
    static datetime_t dtRead;
    static uint8_t pos = 0;

    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (waitCmd) {
            // writes always start with a command
            curCmd = i2c_read_byte_raw(i2c);
            if ((curCmd != CMD_READ_UPDATING) &&
                (curCmd != CMD_READ_RTC) &&
                (curCmd != CMD_WRITE_RTC)) {
                    curCmd = CMD_READ_RTC;  // invalid command -> read RTC
                }
            if (curCmd == CMD_READ_RTC) {
                rtc_get_datetime(&dtRead);  // get time if read
            }
            waitCmd = false;    // got a command
            pos = 0;    // start reading / writing RTC from first byte
        } else if ((curCmd == CMD_WRITE_RTC) && (pos < sizeof(datetime_t)) && !updating) {
            // save new date time
            uint8_t *p = (uint8_t *) &dtNew;
            p[pos] = i2c_read_byte_raw(i2c);
            pos++;
            if (pos == sizeof(datetime_t)) {
                // Got all data, update RTC in main loop
                updating = true;
                update = true;
            }
        } else {
            // ignore written bytes
            i2c_read_byte_raw(i2c);
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
        if (curCmd == CMD_READ_UPDATING) {
            i2c_write_byte_raw(i2c, updating);
        } else if ((curCmd == CMD_READ_RTC) && (pos < sizeof(datetime_t))) {
            uint8_t *p = (uint8_t *) &dtRead;
            i2c_write_byte_raw(i2c, p[pos]);
            pos++;
        } else {
            i2c_write_byte_raw(i2c, 0);
        }
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        waitCmd = true; // next write is a command
        break;
    default:
        break;
    }
}


// This rotine will run in core 1
void i2cDevice() {
    // Init the RTC
    datetime_t dt;
    dt.month = 3;
    dt.day = 3;
    dt.year = 2023;
    dt.dotw = 5;
    dt.hour = 21;
    dt.min = 0;
    dt.sec = 0;
    rtc_init();
    rtc_set_datetime(&dt);
    printf ("RTC started\n");
    sleep_ms(100);
    rtc_get_datetime(&dt);
    printf ("Current date: %02d/%02d/%04d %02d:%02d:%02d\n", dt.month, 
            dt.day, dt.year, dt.hour, dt.min, dt.sec);

    // Set up device I2C
    uint baud = i2c_init (I2C_SLAVE_ID, I2C_BAUDRATE);
    printf ("I2C Device @ %u Hz\n", baud);

    // Set up the device I2C pins
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);
    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    // Set it to slave mode
    i2c_slave_init(I2C_SLAVE_ID, I2C_SLAVE_ADDRESS, &i2c_slave_handler);

    printf ("I2C slave started\n");
    sem_release (&sem_device);

    // Main device loop
    while (true) {
        if (update) {
            // Update RTC
            rtc_set_datetime(&dtNew);
            sleep_ms(100);

            // Show new date and time
            rtc_get_datetime(&dt);
            printf ("New date: %02d/%02d/%04d %02d:%02d:%02d\n", dt.month, 
                    dt.day, dt.year, dt.hour, dt.min, dt.sec);
            update = false;
            updating = false;
        }
        sleep_ms (100);
    }
}

// Main Program
int main() {
    // Start stdio and wait for USB connection
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf ("\nDevice demo\n");

    // Start other core
    sem_init (&sem_device, 0, 1);
    multicore_launch_core1(i2cDevice);
    sem_acquire_blocking (&sem_device);
    
    // Set up master I2C
    uint baud = i2c_init (I2C_MASTER_ID, I2C_BAUDRATE);
    printf ("I2C Master @ %u Hz\n", baud);
    
    // Set up the master I2C pins
    gpio_init(I2C_MASTER_SDA_PIN);
    gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
    gpio_init(I2C_MASTER_SCL_PIN);
    gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);

    // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    gpio_pull_up(I2C_MASTER_SDA_PIN);
    gpio_pull_up(I2C_MASTER_SCL_PIN);

    printf ("I2C master started\n");

    // Main master loop
    while (true) {
        int c;
        int ret;
        datetime_t dt;

        // Ask for operation
        printf("(R)ead or (W)rite?\n");
        do {
            c = getchar_timeout_us(1000);
        } while ((c != 'r') && (c != 'R') && (c != 'w') && (c != 'W'));

        if ((c == 'r') || (c == 'R')) {
            // Read RTC through I2C
            uint8_t buffer[1];
            buffer[0] = CMD_READ_RTC;
            int ret = i2c_write_blocking (I2C_MASTER_ID, I2C_SLAVE_ADDRESS, buffer, 1, true);
            if (ret == 1) {
                ret = i2c_read_blocking(I2C_MASTER_ID, I2C_SLAVE_ADDRESS, (uint8_t *) &dt, sizeof(dt), false);
                if (ret != sizeof(dt)) {
                    printf ("Error reading date!\n");
                }
            } else {
                printf ("Error sending command, check wiring!\n");
            }

            // Show date and time
            printf ("RTC: %02d/%02d/%04d %02d:%02d:%02d\n", dt.month, 
                    dt.day, dt.year, dt.hour, dt.min, dt.sec);
        } else {
            // Get new date and time
            int dig[14];
            int n = 0;
            printf("Enter date and time as MMDDYYYYHHMMSS\n");
            while (n < 14) {
                c = getchar_timeout_us(1000);
                if ((c >= '0') && (c <= '9')) {
                    putchar_raw(c);
                    dig[n++] = c - '0';
                }
            }
            printf("\n");
            dt.month = dig[0]*10+dig[1];
            dt.day = dig[2]*10+dig[3];
            dt.year = dig[4]*1000+dig[5]*100+dig[6]*10+dig[7];
            dt.dotw = 0;
            dt.hour = dig[8]*10+dig[9];
            dt.min = dig[10]*10+dig[11];
            dt.sec = dig[12]*10+dig[13];

            // Wait for RTC ready
            while (true) {
                uint8_t buffer[1];
                buffer[0] = CMD_READ_UPDATING;
                int ret = i2c_write_blocking (I2C_MASTER_ID, I2C_SLAVE_ADDRESS, buffer, 1, true);
                if (ret == 1) {
                    ret = i2c_read_blocking(I2C_MASTER_ID, I2C_SLAVE_ADDRESS, buffer, 1, false);
                    if ((ret == 1) && (buffer[0] == false)) {
                        break;
                    }
                }
            }

            // Update RTC through I2C
            // Command and data must be in a single transaction
            uint8_t buffer[1+sizeof(dt)];
            buffer[0] = CMD_WRITE_RTC;
            memcpy (buffer+1, &dt, sizeof(dt));
            int ret = i2c_write_blocking (I2C_MASTER_ID, I2C_SLAVE_ADDRESS, buffer, 1+sizeof(dt), false);
        }
    }
}
