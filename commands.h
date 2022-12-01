#ifndef COMMANDS_H
#define COMMANDS_H

#include "hardware/gpio.h"
#include "scpi_parsing.h"

int led(const struct parsed_command_t *cmd);
int daq(const struct parsed_command_t *cmd);

#endif