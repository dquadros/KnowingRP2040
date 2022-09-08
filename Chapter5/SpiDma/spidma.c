/**
 * @file spidma.c
 * @author Daniel Quadros
 * @brief Example of using DMA with SPI in the RP2040
 *        to drive a Nokia 5110 display
 * @version 0.1
 * @date 2022-09-07
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

// Display connections
#define PIN_SCE   20
#define PIN_RESET 19
#define PIN_DC    18
#define PIN_SDIN  15
#define PIN_SCLK  14

// Data/Command selection
#define LCD_CMD   0
#define LCD_DAT   1

// Screen size
#define LCD_DX    84
#define LCD_DY    48

// Display init cmds
uint8_t lcdInit[] = { 0x21, 0xB0, 0x04, 0x15, 0x20, 0x0C };

// Put display pointer in home position
uint8_t lcdHome[] =  { 0x40, 0x80 };

// Each byte in the display memory controls 8 vertical pixels
// We are going to divide the display in three horizontal strips:
//   Top      8 pixels high
//   Main    32 pixels high
//   Bottom   8 pixels high
uint8_t topScreen[2][LCD_DX];
uint8_t mainScreen[2][LCD_DX*4];
uint8_t bottomScreen[2][LCD_DX];
int screenDMA = 0;  // main screen programmed in DMA

// SPI Configuration
#define SPI_ID spi1
#define BAUD_RATE 4000000   // 4 MHz
#define DATA_BITS 8

// DMA channel numbers
int dma_chan_data;
int dma_chan_ctrl;

// Flag to signal end of screen update
volatile bool screenUpdated = true;

// Control blocks for transfering screen data
// We will change the data pointers as needed
struct {uint32_t len; const char *data;} control_blocks[] = {
    {LCD_DX,   NULL},
    {LCD_DX*4, NULL},
    {LCD_DX,   NULL},
    {0, NULL}                     // Null trigger to end chain.
};

// This rotine will run when the data DMA gets a null trigger
void dma_irq_handler() {
    // Clear the interrupt request.
    dma_hw->ints0 = 1u << dma_chan_data;
    // Set flag to indicate end
    screenUpdated = true;
}

// Init screen buffers
void initStrips() {
    // Horizontal Lines
    for (int i = 0; i < LCD_DX; i++) {
        topScreen[0][i] = 0x55;
        bottomScreen[0][i] = 0x66;
    }
    // Simple Patterns
    for (int i = 0; i < LCD_DX; i+=2) {
        topScreen[1][i] = 0x63;
        topScreen[1][i+1] = 0x63;
        bottomScreen[1][i] = 0x7F;
        bottomScreen[1][i+1] = 0x41;
    }
    // Main screen is already with zeros
}

// Init DMA
void initDMA() {
    // Get two channels
    dma_chan_data = dma_claim_unused_channel(true);
    dma_chan_ctrl = dma_claim_unused_channel(true);

    // Set up control channel
    dma_channel_config c = dma_channel_get_default_config(dma_chan_ctrl);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);
    channel_config_set_ring(&c, true, 3); // 1 << 3 byte boundary on write ptr
    dma_channel_configure(
        dma_chan_ctrl,
        &c,
        &dma_hw->ch[dma_chan_data].al3_transfer_count,
        &control_blocks[0],
        2,
        false       // Don't start yet.
    );

    // Set up data channel
    c = dma_channel_get_default_config(dma_chan_data);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(SPI_ID, true));
    channel_config_set_chain_to(&c, dma_chan_ctrl);
    channel_config_set_irq_quiet(&c, true);
    dma_channel_configure(
        dma_chan_data,
        &c,
        &spi_get_hw(SPI_ID)->dr,
        NULL,   // Initial read address and transfer count 
        0,      // are unimportant
        false   // Don't start yet.
    );

    // DMA will raise IRQ0 when it gets a null trigger
    dma_channel_set_irq0_enabled(dma_chan_data, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

// Init Display
void displayInit() {
    // Configure GPIO pins
    gpio_init(PIN_SCE);
    gpio_set_dir(PIN_SCE, true);
    gpio_put(PIN_SCE, true);
    gpio_init(PIN_RESET);
    gpio_set_dir(PIN_RESET, true);
    gpio_put(PIN_RESET, true);
    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, true);
    gpio_put(PIN_DC, true);

    // Set up SPI
    uint baud = spi_init (SPI_ID, BAUD_RATE);
    printf ("SPI @ %u Hz\n", baud);
    spi_set_format (SPI_ID, DATA_BITS, SPI_CPOL_1, SPI_CPHA_1, 
                    SPI_MSB_FIRST);

    // Set up the SPI pins
    gpio_set_function(PIN_SCLK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SDIN, GPIO_FUNC_SPI);

    // Reset the display controller
    gpio_put(PIN_RESET, false);
    sleep_ms(100);
    gpio_put(PIN_RESET, true);

    // Initialize the display controller
    // We will not use DMA for this
    gpio_put(PIN_SCE, false);   // leave it selected
    gpio_put(PIN_DC, false);
    spi_write_blocking(SPI_ID, lcdInit, sizeof(lcdInit));
    gpio_put(PIN_DC, true);
}

// Refresh the screen
void displayRefresh(int top, int bottom) {
    // Make sure previous refresh is finished
    while (!screenUpdated) {
        tight_loop_contents();
    }
    screenUpdated = false;

    // Switch buffer
    screenDMA = 1 - screenDMA;

    // Update data address in control block
    control_blocks[0].data = topScreen[top];
    control_blocks[1].data = mainScreen[screenDMA];
    control_blocks[2].data = bottomScreen[bottom];

    // Position data pointer at start of memory
    // (also not using DMA for this)
    gpio_put(PIN_DC, false);
    spi_write_blocking(SPI_ID, lcdHome, sizeof(lcdHome));
    gpio_put(PIN_DC, true);

    // Start DMA
    // Control channel will set the data channel transfers
    dma_channel_set_read_addr(dma_chan_ctrl, &control_blocks[0], 
            true);
}

// Draw the next frame
const uint8_t masks[] = { 0xC0, 0xF0, 0x0C, 0x0F };
void drawFrame() {
    int s = 1 - screenDMA;

    // Copy previous screen
    memcpy(mainScreen[s], mainScreen[screenDMA], 
           sizeof(mainScreen[0]));

    // Erase a random rectangle
    int n = (rand() % 16) + 2;
    int x = rand() % (LCD_DX - n);
    int y = rand() % 4;
    uint8_t mask = masks[rand() % 4];
    for (int i = 0; i < n ; i ++) {
        mainScreen[s][LCD_DX*y+x+i] &= mask;
    }

    // Draw a random rectangle
    n = (rand() % 16) + 2;
    x = rand() % (LCD_DX - n);
    y = rand() % 4;
    mask = masks[rand() % 4];
    for (int i = 0; i < n ; i ++) {
        mainScreen[s][LCD_DX*y+x+i] |= mask;
    }
}

// Main Program
int main() {
    // Init screen
    initStrips();
    initDMA();
    displayInit();
    displayRefresh(0, 0);

    // Main loop
    int frameCounter = 0;
    int border = 0;
    while (1) {
        sleep_ms(100);
        drawFrame();
        displayRefresh(border & 1, (border & 2) >> 1);
        if (++frameCounter == 100) {
            // Change borders from time to time
            frameCounter = 0;
            border = (border + 1) & 3;
        }
    }
}
