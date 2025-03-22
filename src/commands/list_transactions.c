#include "commands/list_transactions.h"
#include "layer1_client.h"
#include "arg_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Command list_transactions_command = {
    .name = "list-transactions",
    .description = "List transactions by reference",
    .execute = execute_list_transactions_command,
    .help = list_transactions_help
};

void register_list_transactions_command(void) {
    register_command(&list_transactions_command);
}

bool execute_list_transactions_command(Layer1Client *client, int argc, char **argv) {
    CommandArgs *args = parse_command_args(argc, argv);
    if (!args) {
        fprintf(stderr, "Error: Failed to parse arguments\n");
        return false;
    }

    const char *asset_pool_id = get_arg_value(args, "asset-pool-id");
    const char *reference = get_arg_value(args, "reference");

    if (!asset_pool_id || !reference) {
        fprintf(stderr, "Error: Missing required arguments\n");
        list_transactions_help();
        free_command_args(args);
        return false;
    }

    // Prepare the query parameter in the format reference:REF-12a1
    char query[256];
    snprintf(query, sizeof(query), "reference:%s+type:(deposit+withdrawal)", reference);

    // Call the Layer1 client API
    TransactionListResponse *response = layer1_list_transactions(
        client,
        asset_pool_id,
        query
    );

    if (!response) {
        fprintf(stderr, "Error: Failed to list transactions\n");
        free_command_args(args);
        return false;
    }

    // Print the response
    printf("Transactions found:\n");
    for (int i = 0; i < response->count; i++) {
        Transaction *tx = &response->transactions[i];
        printf("\nTransaction %d:\n", i + 1);
        printf("  ID: %s\n", tx->id);
        printf("  Status: %s\n", tx->status);
        printf("  Network: %s\n", tx->network);
        printf("  Asset: %s\n", tx->asset);
        printf("  Reference: %s\n", tx->reference);
        printf("  Created At: %s\n", tx->createdAt);
        printf("  Amount: %s\n", tx->amount);
    }

    // Clean up
    layer1_free_transaction_list_response(response);
    free_command_args(args);
    return true;
}

void list_transactions_help(void) {
    printf("Usage: list-transactions --asset-pool-id <id> --reference <reference>\n\n");
    printf("List transactions by reference.\n\n");
    printf("Required arguments:\n");
    printf("  --asset-pool-id <id>    The ID of the asset pool\n");
    printf("  --reference <reference>  The reference to search for\n");
} 
