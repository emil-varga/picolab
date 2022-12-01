#include "commands.h"
#include <string.h>
#include <stdlib.h>
#include "hardware/gpio.h"

int led(const struct parsed_command_t *cmd)
{
    if(cmd->num_args < 2)
        return -1;
    uint gpio = atoi(cmd->args[0]);
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
    int samples = atoi(cmd->args[0]);
    int rate = atoi(cmd->args[1]);

    uint8_t *buffer = (uint8_t)malloc(samples*sizeof(uint8_t));
    
}