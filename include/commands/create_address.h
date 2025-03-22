#ifndef CREATE_ADDRESS_H
#define CREATE_ADDRESS_H

#include "layer1_client.h"

// Response structure for create address command
typedef struct {
    char *address;
    char *asset;
    char *network;
    char *reference;
    char *asset_pool_id;
    char *id;
} CreateAddressResponse;

// Register the create address command
void register_create_address_command(void);

// Execute the create address command
bool execute_create_address_command(Layer1Client *client, int argc, char **argv);

// Display help for the create address command
void create_address_help(void);

// Parse the response from the API
CreateAddressResponse *parse_create_address_response(const char *json);

// Free the response structure
void free_create_address_response(CreateAddressResponse *response);

#endif // CREATE_ADDRESS_H
