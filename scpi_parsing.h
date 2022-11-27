#ifndef SCPI_PARSING_H
#define SCPI_PARSING_H

#include "pico/stdlib.h"

#define MAX_COMMAND_LEN 256

typedef void (*command_callback_t)(char *response, int argN, ...);

struct command_entry_t {
    char cmd_name[MAX_COMMAND_LEN];
    command_callback_t func;
};

struct command_table_t {
    size_t table_size;
    size_t entries;
    struct command_entry_t *commands;
};

struct parsed_command_t {
    char *cmd_name;
    bool query;
    int num_args;
    char **args;
};

void free_parsed_command(struct parsed_command_t *cmd);

struct command_table_t *new_command_table();
int add_command(struct command_table_t *table, const char *command, command_callback_t func);

struct parsed_command_t *scpi_parse_msg(char *msg);

#endif