#ifndef COMMANDS_H
#define COMMANDS_H

#include "hardware/gpio.h"
#include "scpi_parsing.h"

int led(const struct parsed_command_t *cmd);
int daq(const struct parsed_command_t *cmd);
int idn(const struct parsed_command_t *cmd);
int readP(const struct parsed_command_t *cmd);
int readT(const struct parsed_command_t *cmd);
int readPT(const struct parsed_command_t *cmd);

int readACC(const struct parsed_command_t *cmd);
int readGYRO(const struct parsed_command_t *cmd);

#endif