#ifndef CREATE_ADDRESS_BY_ASSET_H
#define CREATE_ADDRESS_BY_ASSET_H

#include "layer1_client.h"

// Response structure for create address by asset command
typedef struct {
    char *id;
    char *address;
    char *asset;
    char *network;
    char *reference;
    char *asset_pool_id;
} CreateAddressByAssetResponse;

// Register the create address by asset command
void register_create_address_by_asset_command(void);

// Execute the create address by asset command
bool execute_create_address_by_asset_command(Layer1Client *client, int argc, char **argv);

// Display help for the create address by asset command
void create_address_by_asset_help(void);

// Parse the response from the API
CreateAddressByAssetResponse *parse_create_address_by_asset_response(const char *json);

// Free the response structure
void free_create_address_by_asset_response(CreateAddressByAssetResponse *response);

#endif // CREATE_ADDRESS_BY_ASSET_H 

