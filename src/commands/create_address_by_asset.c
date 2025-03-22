#include "commands/create_address_by_asset.h"
#include "arg_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static Command create_address_by_asset_command = {
    .name = "create-address-by-asset",
    .description = "Create a new address by asset type",
    .execute = execute_create_address_by_asset_command,
    .help = create_address_by_asset_help
};

void register_create_address_by_asset_command(void) {
    register_command(&create_address_by_asset_command);
}

bool execute_create_address_by_asset_command(Layer1Client *client, int argc, char **argv) {
    CommandArgs *args = parse_command_args(argc, argv);
    if (!args) {
        fprintf(stderr, "Error: Failed to parse arguments\n");
        return false;
    }

    const char *asset_pool_id = get_arg_value(args, "asset-pool-id");
    const char *asset = get_arg_value(args, "asset");
    const char *reference = get_arg_value(args, "reference");

    if (!asset_pool_id || !asset || !reference) {
        fprintf(stderr, "Error: Missing required arguments\n");
        create_address_by_asset_help();
        free_command_args(args);
        return false;
    }

    // Step 1: Create address without network (only asset)
    AddressResponse *create_response = layer1_create_address_by_asset(
        client,
        asset_pool_id,
        asset,
        reference
    );

    if (!create_response) {
        fprintf(stderr, "Error: Failed to create address\n");
        free_command_args(args);
        return false;
    }

    printf("Address creation initiated...\n");
    layer1_free_address_response(create_response);

    // Step 2: Poll for 1 second using list addresses API
    printf("Waiting for addresses to be created...\n");
    sleep(1);  // Wait for 1 second

    // Step 3: List addresses and display network + address
    AddressListResponse *list_response = layer1_list_addresses(
        client,
        asset_pool_id,
        reference
    );

    if (!list_response) {
        fprintf(stderr, "Error: Failed to list addresses\n");
        free_command_args(args);
        return false;
    }

    // Print the addresses
    printf("\nAddresses created:\n");
    for (int i = 0; i < list_response->contentCount; i++) {
        AddressResponse *addr = list_response->content[i];
        printf("Network: %-10s Address: %s\n", 
               addr->network ? addr->network : "PENDING",
               addr->address ? addr->address : "PENDING");
    }

    // Clean up
    layer1_free_address_list_response(list_response);
    free_command_args(args);
    return true;
}

void create_address_by_asset_help(void) {
    printf("Usage: create-address-by-asset --asset-pool-id <id> --asset <asset> --reference <reference>\n\n");
    printf("Create new addresses for all supported networks for the specified asset.\n\n");
    printf("Required arguments:\n");
    printf("  --asset-pool-id <id>    The ID of the asset pool\n");
    printf("  --asset <asset>         The asset (e.g. USDC, USDT)\n");
    printf("  --reference <reference>  A reference for the addresses\n");
} 

