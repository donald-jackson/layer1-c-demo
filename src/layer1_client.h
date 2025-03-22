#ifndef LAYER1_CLIENT_H
#define LAYER1_CLIENT_H

#include <curl/curl.h>
#include <stdbool.h>

typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

typedef struct {
    char *base_url;
    char *client_id;
    char *private_key;
    CURL *curl;
} Layer1Client;

typedef struct Command {
    const char *name;
    const char *description;
    bool (*execute)(Layer1Client *client, int argc, char **argv);
    void (*help)(void);
} Command;

typedef struct {
    char *address;
    char *network;
    char *asset;
    char *reference;
    char *assetPoolId;
    char *id;
    char *status;
    char *createdAt;
} AddressResponse;

typedef struct {
    AddressResponse **content;
    int contentCount;
    int pageNumber;
    int pageSize;
    long totalElements;
} AddressListResponse;

typedef struct {
    char *id;
    char *status;
    char *network;
    char *asset;
    char *reference;
    char *createdAt;
    char *amount;
} Transaction;

typedef struct {
    Transaction *transactions;
    int count;
} TransactionListResponse;

typedef struct {
    char *id;
    char *status;
    char *network;
    char *asset;
    char *reference;
    char *createdAt;
} TransactionResponse;

// Client management
Layer1Client *layer1_client_create(const char *base_url, const char *client_id, const char *private_key_path);
void layer1_client_destroy(Layer1Client *client);

// Command management
void register_command(Command *command);
Command *get_command(const char *name);

// Utility functions
char *read_file_to_string(const char *filename);
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);

// Address operations
AddressResponse *layer1_create_address(Layer1Client *client, const char *asset_pool_id, const char *network, const char *asset, const char *reference);
AddressResponse *layer1_create_address_by_asset(Layer1Client *client, const char *asset_pool_id, const char *asset, const char *reference);
AddressListResponse *layer1_list_addresses(Layer1Client *client, const char *asset_pool_id, const char *reference);
void layer1_free_address_response(AddressResponse *response);
void layer1_free_address_list_response(AddressListResponse *response);

// Transaction operations
TransactionResponse *layer1_create_transaction(Layer1Client *client, const char *asset_pool_id, const char *network, const char *asset, const char *to_address, const char *amount, const char *reference);
void layer1_free_transaction_response(TransactionResponse *response);
TransactionListResponse *layer1_list_transactions(Layer1Client *client, const char *asset_pool_id, const char *query);
void layer1_free_transaction_list_response(TransactionListResponse *response);

#endif /* LAYER1_CLIENT_H */ 
