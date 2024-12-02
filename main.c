#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"

#include "scpi_parsing.h"
#include "commands.h"
#include "pins.h"
#include "bmp280.h"

const int max_msg_len = 512;

int read_msg(char *buf, const int max_len) {
    int i = 0;
    while(i < max_len - 1) {
        int c = getchar_timeout_us(100);
        if(c == PICO_ERROR_TIMEOUT)
            break;
        if(c == '\n') c = '\0';
        
        buf[i++] = c & 0xFF;
    }
    buf[i] = '\0';
    return i;
}

int main() {
    char msg_in[max_msg_len];
    char msg_out[max_msg_len];

    stdio_usb_init();
    stdio_set_translate_crlf(&stdio_usb, false);

    //Initialize LED pins
    gpio_init(LED0_PIN);
    gpio_init(LED1_PIN);
    gpio_init(LED2_PIN);
    gpio_init(LED3_PIN);
    gpio_init(LED4_PIN);

    gpio_set_dir(0, GPIO_OUT);
    gpio_set_dir(1, GPIO_OUT);
    gpio_set_dir(2, GPIO_OUT);
    gpio_set_dir(3, GPIO_OUT);
    gpio_set_dir(4, GPIO_OUT);

    //Initialize ADC and DMA for reading the piezo
    adc_gpio_init(PIEZO_ADC_PIN);
    adc_init();
    adc_select_input(PIEZO_ADC_CHAN);
    adc_fifo_setup(
        true,
        true,
        1,
        false,
        true
    );

    //Initialize I2C 0 for communication
    i2c_inst_t *i2c = i2c0;
    i2c_init(i2c, 400*1000);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);

    bmp280_read_calibration(i2c);
    uint8_t ctrl_meas = 0;
    ctrl_meas = 0b111 << 4; //16x oversampling for temperature
    ctrl_meas |= 0b111 << 2; //16x oversampling for pressure
    ctrl_meas |= 0b11; // normal mode
    write_register(i2c, BMP280_I2C_ADDR, 0xF4, &ctrl_meas, 1);

    uint8_t to_write = 0;
    //stop the sleep
    write_register(i2c, MPU6050_I2C_ADDR, 0x6B, &to_write, 1);
    //set the accelerometer full scale range to +/- 2g
    write_register(i2c, MPU6050_I2C_ADDR, 0x1C, &to_write, 1);
    //set the gyroscope full range scale to +/- 250 deg/s
    write_register(i2c, MPU6050_I2C_ADDR, 0x1B, &to_write, 1);

    //build the table containing assigning functions to scpi commands
    struct command_table_t *table = scpi_new_command_table();
    scpi_add_command(table, ":LED", led);
    scpi_add_command(table, ":DAQ?", daq);
    scpi_add_command(table, "*IDN?", idn);
    scpi_add_command(table, ":READ:PT?", readPT);
    scpi_add_command(table, ":READ:P?", readP);
    scpi_add_command(table, ":READ:T?", readT);
    scpi_add_command(table, ":READ:ACC?", readACC);
    scpi_add_command(table, ":READ:GYR?", readGYRO);

    int n_msgs = 0;
    while(1) {
        int l = read_msg(msg_in, max_msg_len);
        if(l < 2) {
            sleep_ms(10);
            continue;
        }

        struct parsed_command_t *cmd = scpi_parse_msg(msg_in);
        n_msgs++;
        //printf("cmd_name: %s, #args: %d\n", cmd->cmd_name, cmd->num_args);
        scpi_run_command(table, cmd);
        free_parsed_command(cmd);
    }
    scpi_free_command_table(table);
    return 0;
}