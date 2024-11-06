#include "commands.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "bmp280.h"

#include "pins.h"

int idn(const struct parsed_command_t *cmd)
{
    printf("PICO\n");
    return 0;
}

int led(const struct parsed_command_t *cmd)
{
    static int led_pins[] = {LED0_PIN, LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN};
    if(cmd->num_args < 2)
        return -1;
    int led_id = atoi(cmd->args[0]);
    uint gpio = led_pins[led_id];
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

    const float base_clk = 48e6;
    if(rate < 0 || rate > 500e3) {
        printf("Error: Bad rate.");
        return -1;
    }
    float clk_div = base_clk/rate;
    
    uint8_t *buffer = (uint8_t*)malloc(samples*sizeof(uint8_t));
    sleep_ms(1000);

    adc_set_clkdiv(clk_div);

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

int readPT(const struct parsed_command_t *cmd) {
    int32_t pressure, temperature;
    bmp280_read(i2c0, &temperature, &pressure);
    printf("T%d P%d\n", temperature, pressure);
    return 0;
}

int readT(const struct parsed_command_t *cmd) {
    int32_t pressure, temperature;
    bmp280_read(i2c0, &temperature, &pressure);
    printf("%d\n", temperature);
    return 0;
}

int readP(const struct parsed_command_t *cmd) {
    int32_t pressure, temperature;
    bmp280_read(i2c0, &temperature, &pressure);
    printf("%d\n", pressure);
    return 0;
}