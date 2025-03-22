#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <stdbool.h>

typedef struct {
    const char *name;
    const char *value;
} CommandArg;

typedef struct {
    CommandArg *args;
    int count;
} CommandArgs;

CommandArgs *parse_command_args(int argc, char **argv);
const char *get_arg_value(CommandArgs *args, const char *name);
void free_command_args(CommandArgs *args);

#endif // ARG_PARSER_H 
