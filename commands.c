#include "commands.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

int led(const struct parsed_command_t *cmd)
{
    if(cmd->num_args < 2)
        return -1;
    uint gpio = atoi(cmd->args[0]);
    printf("LED %d %s\n", gpio, cmd->args[1]);
    if(!strcasecmp(cmd->args[1], "ON") || !strcasecmp(cmd->args[1], "1"))
        gpio_put(gpio, 1);
    else
        gpio_put(gpio, 0);
    
    return 0;
}

int daq(const struct parsed_command_t *cmd)
{
    if(cmd->num_args < 2)
        return -1;
    printf("daq %s %s\n", cmd->args[0], cmd->args[1]);
    int samples = atoi(cmd->args[0]);
    int rate = atoi(cmd->args[1]);
    
    uint8_t *buffer = (uint8_t*)malloc(samples*sizeof(uint8_t));
    sleep_ms(1000);
    // Set up the DMA to start transferring data as soon as it appears in FIFO
    uint dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);

    // Reading from constant address, writing to incrementing byte addresses
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);

    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);

    dma_channel_configure(dma_chan, &cfg,
        buffer,    // dst
        &adc_hw->fifo,  // src
        samples,  // transfer count
        true            // start immediately
    );

    adc_run(true);

    // Once DMA finishes, stop any new conversions from starting, and clean up
    // the FIFO in case the ADC was still mid-conversion.
    dma_channel_wait_for_finish_blocking(dma_chan);
    adc_run(false);
    adc_fifo_drain();
    for(int k=0; k < samples; ++k)
        printf("%d\n", buffer[k]);
}