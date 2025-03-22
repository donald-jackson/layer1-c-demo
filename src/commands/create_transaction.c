#include "commands/create_transaction.h"
#include "arg_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Command create_transaction_command = {
    .name = "create-transaction",
    .description = "Create a new blockchain transaction",
    .execute = execute_create_transaction_command,
    .help = create_transaction_help
};

void register_create_transaction_command(void) {
    register_command(&create_transaction_command);
}

bool execute_create_transaction_command(Layer1Client *client, int argc, char **argv) {
    CommandArgs *args = parse_command_args(argc, argv);
    if (!args) {
        fprintf(stderr, "Error: Failed to parse arguments\n");
        return false;
    }

    const char *asset_pool_id = get_arg_value(args, "asset-pool-id");
    const char *network = get_arg_value(args, "network");
    const char *asset = get_arg_value(args, "asset");
    const char *to_address = get_arg_value(args, "to");
    const char *amount = get_arg_value(args, "amount");
    const char *reference = get_arg_value(args, "reference");

    if (!asset_pool_id || !network || !asset || !to_address || !amount) {
        fprintf(stderr, "Error: Missing required arguments\n");
        create_transaction_help();
        free_command_args(args);
        return false;
    }

    // Call the Layer1 client API
    TransactionResponse *response = layer1_create_transaction(
        client,
        asset_pool_id,
        network,
        asset,
        to_address,
        amount,
        reference
    );

    if (!response) {
        fprintf(stderr, "Error: Failed to create transaction\n");
        free_command_args(args);
        return false;
    }

    // Print the response
    printf("Transaction created successfully:\n");
    printf("  ID: %s\n", response->id);
    printf("  Status: %s\n", response->status);
    printf("  Network: %s\n", response->network);
    printf("  Asset: %s\n", response->asset);
    printf("  Reference: %s\n", response->reference);
    printf("  Created At: %s\n", response->createdAt);

    // Clean up
    layer1_free_transaction_response(response);
    free_command_args(args);
    return true;
}

void create_transaction_help(void) {
    printf("Usage: create-transaction --asset-pool-id <id> --network <network> --asset <asset> --from <address> --to <address> --amount <amount> [--reference <reference>]\n\n");
    printf("Create a new blockchain transaction.\n\n");
    printf("Required arguments:\n");
    printf("  --asset-pool-id <id>    The ID of the asset pool\n");
    printf("  --network <network>     The network (e.g. ETHEREUM, TRON, SOLANA)\n");
    printf("  --asset <asset>         The asset (e.g. USDC, USDT)\n");
    printf("  --from <address>        The source address\n");
    printf("  --to <address>          The destination address\n");
    printf("  --amount <amount>       The amount to transfer\n\n");
    printf("Optional arguments:\n");
    printf("  --reference <reference> A reference for the transaction\n");
}
