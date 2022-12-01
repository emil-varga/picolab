#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

#include "scpi_parsing.h"
#include "commands.h"

const int led_pin = PICO_DEFAULT_LED_PIN;
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
    gpio_init(led_pin);
    gpio_init(0);
    gpio_init(1);
    gpio_init(2);
    gpio_init(3);
    gpio_init(4);

    gpio_set_dir(led_pin, GPIO_OUT);
    gpio_set_dir(0, GPIO_OUT);
    gpio_set_dir(1, GPIO_OUT);
    gpio_set_dir(2, GPIO_OUT);
    gpio_set_dir(3, GPIO_OUT);
    gpio_set_dir(4, GPIO_OUT);
    gpio_put(led_pin, 0);

    adc_gpio_init(26);
    adc_init();
    

    struct command_table_t *table = scpi_new_command_table();

    scpi_add_command(table, "LED", led);

    int n_msgs = 0;
    while(1) {
        int l = read_msg(msg_in, max_msg_len);
        if(l < 2) {
            sleep_ms(10);
            continue;
        }

        gpio_put(led_pin, 1);
        sleep_ms(100);
        gpio_put(led_pin, 0);
        struct parsed_command_t *cmd = scpi_parse_msg(msg_in);
        n_msgs++;
        printf("cmd_name: %s, #args: %d\n", cmd->cmd_name, cmd->num_args);
        scpi_run_command(table, cmd);
        free_parsed_command(cmd);
        gpio_put(led_pin, 1);
        sleep_ms(50);
        gpio_put(led_pin, 0);
    }
    scpi_free_command_table(table);
    return 0;
}