#include "layer1_client.h"
#include "commands/create_address.h"
#include "commands/create_address_by_asset.h"
#include "commands/create_transaction.h"
#include "commands/list_transactions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

void init_commands(void) {
    register_create_address_command();
    register_create_address_by_asset_command();
    register_create_transaction_command();
    register_list_transactions_command();
    // Register other commands here
}

void print_usage(void) {
    printf("Usage: layer1_cli [--base-url <url>] --client-id <id> --key-file <path> <command> [args...]\n");
    printf("\n");
    printf("Options:\n");
    printf("  --base-url <url>    Base URL for the API (default: https://api.sandbox.layer1.com)\n");
    printf("  --client-id <id>    OAuth2 Client ID\n");
    printf("  --key-file <path>   Path to the private key file\n");
    printf("\n");
    printf("Commands:\n");
    printf("  create-address            Create a new address\n");
    printf("  create-address-by-asset   Create a new address for a specific asset\n");
    printf("  create-transaction        Create a new transaction\n");
    printf("  list-transactions         List transactions by reference\n");
    printf("\n");
    printf("Run 'layer1_cli <command> --help' for more information on a command.\n");
}

int main(int argc, char *argv[]) {
    // Default values
    const char *base_url = "https://api.sandbox.layer1.com";
    const char *client_id = NULL;
    const char *key_file = NULL;
    
    // Parse command line arguments
    int arg_index = 1;
    while (arg_index < argc) {
        if (strcmp(argv[arg_index], "--base-url") == 0) {
            if (arg_index + 1 < argc) {
                base_url = argv[arg_index + 1];
                arg_index += 2;
            } else {
                fprintf(stderr, "Error: Missing value for --base-url\n");
                print_usage();
                return 1;
            }
        } else if (strcmp(argv[arg_index], "--client-id") == 0) {
            if (arg_index + 1 < argc) {
                client_id = argv[arg_index + 1];
                arg_index += 2;
            } else {
                fprintf(stderr, "Error: Missing value for --client-id\n");
                print_usage();
                return 1;
            }
        } else if (strcmp(argv[arg_index], "--key-file") == 0) {
            if (arg_index + 1 < argc) {
                key_file = argv[arg_index + 1];
                arg_index += 2;
            } else {
                fprintf(stderr, "Error: Missing value for --key-file\n");
                print_usage();
                return 1;
            }
        } else {
            // This must be the command
            break;
        }
    }
    
    // Check required arguments
    if (!client_id) {
        fprintf(stderr, "Error: --client-id is required\n");
        print_usage();
        return 1;
    }
    
    if (!key_file) {
        fprintf(stderr, "Error: --key-file is required\n");
        print_usage();
        return 1;
    }
    
    // Check if a command was provided
    if (arg_index >= argc) {
        fprintf(stderr, "Error: No command specified\n");
        print_usage();
        return 1;
    }
    
    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Initialize commands
    init_commands();
    
    // Get the command
    const char *command_name = argv[arg_index++];
    Command *command = get_command(command_name);
    
    if (!command) {
        fprintf(stderr, "Error: Unknown command '%s'\n", command_name);
        print_usage();
        curl_global_cleanup();
        return 1;
    }
    
    // Check for help flag
    if (arg_index < argc && strcmp(argv[arg_index], "--help") == 0) {
        command->help();
        curl_global_cleanup();
        return 0;
    }
    
    // Create client
    Layer1Client *client = layer1_client_create(base_url, client_id, key_file);
    if (!client) {
        fprintf(stderr, "Error: Failed to create Layer1 client\n");
        curl_global_cleanup();
        return 1;
    }
    
    // Execute command
    bool success = command->execute(client, argc - arg_index + 1, argv + arg_index - 1);
    
    // Clean up
    layer1_client_destroy(client);
    curl_global_cleanup();
    
    return success ? 0 : 1;
}
