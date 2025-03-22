#ifndef LIST_TRANSACTIONS_H
#define LIST_TRANSACTIONS_H

#include "layer1_client.h"
#include <stdbool.h>

void register_list_transactions_command(void);
bool execute_list_transactions_command(Layer1Client *client, int argc, char **argv);
void list_transactions_help(void);

#endif /* LIST_TRANSACTIONS_H */ 
