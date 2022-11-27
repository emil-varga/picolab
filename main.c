#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "scpi_parsing.h"

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
    gpio_set_dir(led_pin, GPIO_OUT);
    gpio_put(led_pin, 0);

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
        free_parsed_command(cmd);
        gpio_put(led_pin, 1);
        sleep_ms(50);
        gpio_put(led_pin, 0);
    }
    return 0;
}