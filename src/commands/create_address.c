#include "commands/create_address.h"
#include "arg_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Command create_address_command = {
    .name = "create-address",
    .description = "Create a new address",
    .execute = execute_create_address_command,
    .help = create_address_help
};

void register_create_address_command(void) {
    register_command(&create_address_command);
}

bool execute_create_address_command(Layer1Client *client, int argc, char **argv) {
    CommandArgs *args = parse_command_args(argc, argv);
    if (!args) {
        fprintf(stderr, "Error: Failed to parse arguments\n");
        return false;
    }

    const char *asset_pool_id = get_arg_value(args, "asset-pool-id");
    const char *network = get_arg_value(args, "network");
    const char *reference = get_arg_value(args, "reference");

    if (!asset_pool_id || !network || !reference) {
        fprintf(stderr, "Error: Missing required arguments\n");
        create_address_help();
        free_command_args(args);
        return false;
    }

    // Call the Layer1 client API
    AddressResponse *response = layer1_create_address(
        client,
        asset_pool_id,
        network,
        NULL,
        reference
    );

    if (!response) {
        fprintf(stderr, "Error: Failed to create address\n");
        free_command_args(args);
        return false;
    }

    // Print the response
    printf("Address created successfully:\n");
    printf("  ID: %s\n", response->id);
    printf("  Address: %s\n", response->address);
    printf("  Network: %s\n", response->network);
    printf("  Reference: %s\n", response->reference);
    printf("  Asset Pool ID: %s\n", response->assetPoolId);
    printf("  Created At: %s\n", response->createdAt);

    // Clean up
    layer1_free_address_response(response);
    free_command_args(args);
    return true;
}

void create_address_help(void) {
    printf("Usage: create-address --asset-pool-id <id> --network <network> --asset <asset> --reference <reference>\n\n");
    printf("Create a new address for receiving assets.\n\n");
    printf("Required arguments:\n");
    printf("  --asset-pool-id <id>    The ID of the asset pool\n");
    printf("  --network <network>     The network (e.g. ETHEREUM, TRON, SOLANA)\n");
    printf("  --asset <asset>         The asset (e.g. USDC, USDT)\n");
    printf("  --reference <reference> A reference for the address\n");
}
