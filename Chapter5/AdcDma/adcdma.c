/**
 * @file adcdma.c
 * @author Daniel Quadros
 * @brief Example of using DMA with the ADC in the RP2040 to read
 *        the internal temperature sensor
 * @version 0.1
 * @date 2022-09-06
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

// Internal temperature sensor
#define ADC_INPUT_TEMPSENSOR 4

// DMA channel number
int dma_chan;

// Factor to convert ADC reading to voltage
// Assumes 12-bit, ADC_VREF = 3.3V
const float conversionFactor = 3.3f / (1 << 12);

// Buffers for ADC readings
#define N_SAMPLES 1000
int iBuf = 0;   // buffer currently used by DMA
uint16_t buffer[2][N_SAMPLES];
uint32_t finishedXfer[2];


// This rotine will run when DMA finishes filling a buffer
void dma_irq_handler() {
    // Stop ADC
    adc_run(false);
    // Clear the interrupt request.
    dma_hw->ints0 = 1u << dma_chan;
    // Register when DMA finished
    finishedXfer[iBuf] = to_ms_since_boot(get_absolute_time());
}

// Main Program
int main() {
    // Init stdio
    stdio_init_all();
    printf("\nADC DMA Example\n");

    // Init ADC
    // We will read the temperature sensor as fast as possible
    // and generate a DREQ when a sample goes to the FIFO
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input (ADC_INPUT_TEMPSENSOR);
    adc_fifo_setup (true, true, 1, false, false);
    adc_set_clkdiv(0);

    // Init DMA
    dma_chan = dma_claim_unused_channel(true);
    dma_sniffer_enable(dma_chan, 0xf, false);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, DREQ_ADC);
    channel_config_set_sniff_enable(&c, true);

    dma_channel_configure(
        dma_chan,
        &c,
        NULL,             // Don't provide a write address yet
        &adc_hw->fifo,    // Read address (only need to set this once)
        N_SAMPLES,        // Transfer N_SAMPLES values
        false             // Don't start yet
    );    

    // DMA will raise IRQ0 when the channel finishes to fill the buffer
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);


    // Start transfer to first buffer
    dma_hw->sniff_data = 0;
    dma_channel_set_write_addr(dma_chan, buffer[iBuf], true);

    // Start the ADC
    adc_run(true);

    // Main loop
    while (1) {
        // Make sure last transfer finished
        dma_channel_wait_for_finish_blocking(dma_chan);
        uint32_t finished = finishedXfer[iBuf];

        // Get the sum of the samples
        uint32_t sum = dma_hw->sniff_data;

        // Switch buffers
        iBuf = 1-iBuf;

        // Set up and start DMA transfer to other buffer
        dma_hw->sniff_data = 0;
        dma_channel_set_write_addr(dma_chan, buffer[iBuf], true);
        adc_run(true);

        // At this point we can process buffer 1-iBuf, we will just
        // calculate and print average temperature
        float tempSum = sum * conversionFactor;
        float tempC = 27.0f - (tempSum/N_SAMPLES - 0.706f) / 0.001721f;
        printf("Temperature: %.2f ", tempC);
        uint32_t printed = to_ms_since_boot(get_absolute_time());

        // Show when the transfer finished and when we finished printing
        printf ("Finished transfer: %u Finished printing: %u\n", finished, printed);
    }
}
