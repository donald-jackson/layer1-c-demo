#include "arg_parser.h"
#include <stdlib.h>
#include <string.h>

CommandArgs *parse_command_args(int argc, char **argv) {
    CommandArgs *args = malloc(sizeof(CommandArgs));
    if (!args) return NULL;

    args->args = malloc(sizeof(CommandArg) * argc);
    if (!args->args) {
        free(args);
        return NULL;
    }

    args->count = 0;

    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] == '-') {
            // This is a named argument
            const char *name = argv[i] + 2;  // Skip the '--'
            
            // Check if there's a value following
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args->args[args->count].name = name;
                args->args[args->count].value = argv[i + 1];
                args->count++;
                i++; // Skip the value
            }
        }
    }

    return args;
}

const char *get_arg_value(CommandArgs *args, const char *name) {
    if (!args || !name) return NULL;

    for (int i = 0; i < args->count; i++) {
        if (strcmp(args->args[i].name, name) == 0) {
            return args->args[i].value;
        }
    }

    return NULL;
}

void free_command_args(CommandArgs *args) {
    if (!args) return;
    free(args->args);
    free(args);
} 
