#include<ctype.h>
#include<string.h>
#include<stdlib.h> 
#include<stdio.h>
#include "pico/stdlib.h"
#include "scpi_parsing.h"

struct command_table_t *new_command_table()
{
    return NULL;
};

int add_command(struct command_table_t *table, const char *command, command_callback_t func)
{
    if(table->entries < table->table_size)
    {
        int k = table->entries;
        strncpy(table->commands[k].cmd_name, command, MAX_COMMAND_LEN);
        table->commands[k].func = func;
        table->entries++;
    }
}

void scpi_strncpy_special(char *dst, const char *src, size_t n);

struct token_list;
void token_list_add(struct token_list **list, const char *token);
char *token_list_pop(struct token_list **list);

struct parsed_command_t *scpi_parse_msg(char *msg)
{
    struct parsed_command_t *command = (struct parsed_command_t*)malloc(sizeof(struct parsed_command_t));

    char *saveptr = NULL;
    char *msg_start = msg;
    char *token;

    //save tokens in a linked list
    int tokens = 0;
    struct token_list *list = NULL;
    do {
        token = strtok_r(msg_start, " \t", &saveptr);
        msg_start = NULL;
        if(strlen(token) == 0)
            break;
        token_list_add(&list, token);
        tokens++;
    } while(token);

    command->num_args = tokens-1;
    if(command->num_args > 0) {
        command->args = (char**)malloc(command->num_args*sizeof(char*));
        for(int k=0; k < command->num_args; ++k) {
            command->args[command->num_args-k-1] = token_list_pop(&list);
        }            
    }
    command->cmd_name = token_list_pop(&list);

    return command;
}

struct token_list {
    char *token;
    struct token_list *next;
};

void token_list_add(struct token_list **list, const char *msg)
{
    struct token_list *new = (struct token_list*)malloc(sizeof(struct token_list));
    new->token = strdup(msg);
    
    new->next = *list;
    *list = new;
}

char *token_list_pop(struct token_list **list)
{
    if(!(*list))
        return NULL;
    struct token_list *next = (*list)->next;
    char *tok = (*list)->token;
    free(*list);
    *list = next;
    return tok;
}

void free_parsed_command(struct parsed_command_t *cmd){
    free(cmd->cmd_name);
    for(int k=0; k<cmd->num_args; ++k)
        free(cmd->args[k]);
    free(cmd);
}