#ifndef CREATE_TRANSACTION_H
#define CREATE_TRANSACTION_H

#include "layer1_client.h"

void register_create_transaction_command(void);
bool execute_create_transaction_command(Layer1Client *client, int argc, char **argv);
void create_transaction_help(void);

#endif /* CREATE_TRANSACTION_H */
