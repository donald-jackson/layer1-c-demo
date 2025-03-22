#include "layer1_client.h"
#include "http_signer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../lib/cJSON/cJSON.h"

#define MAX_COMMANDS 10

static Command *commands[MAX_COMMANDS];
static int command_count = 0;

Layer1Client *layer1_client_create(const char *base_url, const char *client_id, const char *private_key_path) {
    Layer1Client *client = (Layer1Client *)malloc(sizeof(Layer1Client));
    if (!client) {
        fprintf(stderr, "Failed to allocate memory for Layer1Client\n");
        return NULL;
    }

    // Initialize with NULL values
    client->base_url = NULL;
    client->client_id = NULL;
    client->private_key = NULL;
    client->curl = NULL;

    // Copy base URL
    if (base_url) {
        client->base_url = strdup(base_url);
        if (!client->base_url) {
            fprintf(stderr, "Failed to allocate memory for base_url\n");
            layer1_client_destroy(client);
            return NULL;
        }
    }

    // Copy client ID
    if (client_id) {
        client->client_id = strdup(client_id);
        if (!client->client_id) {
            fprintf(stderr, "Failed to allocate memory for client_id\n");
            layer1_client_destroy(client);
            return NULL;
        }
    }

    // Read private key from file
    if (private_key_path) {
        client->private_key = read_file_to_string(private_key_path);
        if (!client->private_key) {
            fprintf(stderr, "Failed to read private key from %s\n", private_key_path);
            layer1_client_destroy(client);
            return NULL;
        }
    }

    // Initialize curl
    client->curl = curl_easy_init();
    if (!client->curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        layer1_client_destroy(client);
        return NULL;
    }

    return client;
}

void layer1_client_destroy(Layer1Client *client) {
    if (!client) {
        return;
    }

    free(client->base_url);
    free(client->client_id);
    free(client->private_key);
    
    if (client->curl) {
        curl_easy_cleanup(client->curl);
    }
    
    free(client);
}

void register_command(Command *command) {
    if (command_count < MAX_COMMANDS) {
        commands[command_count++] = command;
    } else {
        fprintf(stderr, "Maximum number of commands reached\n");
    }
}

Command *get_command(const char *name) {
    for (int i = 0; i < command_count; i++) {
        if (strcmp(commands[i]->name, name) == 0) {
            return commands[i];
        }
    }
    return NULL;
}

char *read_file_to_string(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for file content
    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for file content\n");
        fclose(file);
        return NULL;
    }

    // Read file content
    size_t read_size = fread(buffer, 1, file_size, file);
    fclose(file);

    if (read_size != (size_t)file_size) {
        fprintf(stderr, "Failed to read file: %s\n", filename);
        free(buffer);
        return NULL;
    }

    // Null terminate the string
    buffer[file_size] = '\0';
    return buffer;
}

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Failed to allocate memory in write_callback\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static struct curl_slist *add_common_headers(struct curl_slist *headers, bool include_content_type) {
    if (include_content_type) {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }
    headers = curl_slist_append(headers, "Accept: application/json");
    return headers;
}

static AddressResponse *parse_address_response(const char *json_str) {
    if (!json_str) {
        return NULL;
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        return NULL;
    }

    AddressResponse *response = calloc(1, sizeof(AddressResponse));
    if (!response) {
        cJSON_Delete(root);
        return NULL;
    }

    // Extract fields from JSON
    cJSON *address = cJSON_GetObjectItem(root, "address");
    cJSON *network = cJSON_GetObjectItem(root, "network");
    cJSON *asset = cJSON_GetObjectItem(root, "asset");
    cJSON *reference = cJSON_GetObjectItem(root, "reference");
    cJSON *assetPoolId = cJSON_GetObjectItem(root, "assetPoolId");
    cJSON *id = cJSON_GetObjectItem(root, "id");
    cJSON *status = cJSON_GetObjectItem(root, "status");
    cJSON *createdAt = cJSON_GetObjectItem(root, "createdAt");

    // Copy values if they exist
    if (cJSON_IsString(address)) response->address = strdup(address->valuestring);
    if (cJSON_IsString(network)) response->network = strdup(network->valuestring);
    if (cJSON_IsString(asset)) response->asset = strdup(asset->valuestring);
    if (cJSON_IsString(reference)) response->reference = strdup(reference->valuestring);
    if (cJSON_IsString(assetPoolId)) response->assetPoolId = strdup(assetPoolId->valuestring);
    if (cJSON_IsString(id)) response->id = strdup(id->valuestring);
    if (cJSON_IsString(status)) response->status = strdup(status->valuestring);
    if (cJSON_IsString(createdAt)) response->createdAt = strdup(createdAt->valuestring);

    cJSON_Delete(root);
    return response;
}

static AddressListResponse *parse_address_list_response(const char *json_str) {
    if (!json_str) {
        return NULL;
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        return NULL;
    }

    AddressListResponse *response = calloc(1, sizeof(AddressListResponse));
    if (!response) {
        cJSON_Delete(root);
        return NULL;
    }

    // Extract pagination info
    cJSON *pageNumber = cJSON_GetObjectItem(root, "pageNumber");
    cJSON *pageSize = cJSON_GetObjectItem(root, "pageSize");
    cJSON *totalElements = cJSON_GetObjectItem(root, "totalElements");
    cJSON *content = cJSON_GetObjectItem(root, "content");

    if (cJSON_IsNumber(pageNumber)) response->pageNumber = pageNumber->valueint;
    if (cJSON_IsNumber(pageSize)) response->pageSize = pageSize->valueint;
    if (cJSON_IsNumber(totalElements)) response->totalElements = totalElements->valueint;

    if (content && cJSON_IsArray(content)) {
        int contentSize = cJSON_GetArraySize(content);
        response->content = calloc(contentSize, sizeof(AddressResponse*));
        if (!response->content) {
            cJSON_Delete(root);
            free(response);
            return NULL;
        }

        for (int i = 0; i < contentSize; i++) {
            cJSON *item = cJSON_GetArrayItem(content, i);
            char *item_str = cJSON_Print(item);
            if (item_str) {
                response->content[response->contentCount] = parse_address_response(item_str);
                if (response->content[response->contentCount]) {
                    response->contentCount++;
                }
                free(item_str);
            }
        }
    }

    cJSON_Delete(root);
    return response;
}

void layer1_free_address_response(AddressResponse *response) {
    if (!response) {
        return;
    }

    free(response->address);
    free(response->network);
    free(response->asset);
    free(response->reference);
    free(response->assetPoolId);
    free(response->id);
    free(response->status);
    free(response->createdAt);
    free(response);
}

void layer1_free_address_list_response(AddressListResponse *response) {
    if (!response) {
        return;
    }

    if (response->content) {
        for (int i = 0; i < response->contentCount; i++) {
            layer1_free_address_response(response->content[i]);
        }
        free(response->content);
    }

    free(response);
}

AddressListResponse *layer1_list_addresses(
    Layer1Client *client,
    const char *asset_pool_id,
    const char *reference
) {
    if (!client || !asset_pool_id || !reference) {
        return NULL;
    }

    // Prepare URL with query parameters
    char url[2048];
    snprintf(url, sizeof(url), "%s/digital/v1/addresses?assetPoolId=%s&q=reference:%s",
             client->base_url, asset_pool_id, reference);

    // Create HTTP signer
    HttpSigner *signer = http_signer_create(client->private_key, client->client_id);
    if (!signer) {
        return NULL;
    }

    // Set up CURL
    CURL *curl = client->curl;
    curl_easy_reset(curl);
    
    // Set URL and method
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    
    // Set up response buffer
    MemoryStruct chunk = {
        .memory = malloc(1),
        .size = 0
    };
    
    if (!chunk.memory) {
        http_signer_destroy(signer);
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    
    // Set headers
    struct curl_slist *headers = NULL;
    headers = add_common_headers(headers, false);

    // Add signature headers
    if (!http_signer_add_headers(signer, curl, url, NULL, "GET", &headers)) {
        http_signer_destroy(signer);
        free(chunk.memory);
        curl_slist_free_all(headers);
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    // Clean up
    curl_slist_free_all(headers);
    http_signer_destroy(signer);

    if (res != CURLE_OK) {
        free(chunk.memory);
        return NULL;
    }

    // Parse response
    AddressListResponse *response = parse_address_list_response(chunk.memory);
    free(chunk.memory);

    return response;
}

AddressResponse *layer1_create_address(
    Layer1Client *client,
    const char *asset_pool_id,
    const char *network,
    const char *asset,
    const char *reference
) {
    if (!client || !asset_pool_id || (!network && !asset) || !reference) {
        return NULL;
    }

    // Create JSON payload
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "assetPoolId", asset_pool_id);
    cJSON_AddStringToObject(root, "network", network);
    cJSON_AddStringToObject(root, "asset", asset);
    cJSON_AddStringToObject(root, "reference", reference);
    
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!payload) {
        return NULL;
    }

    // Create HTTP signer
    HttpSigner *signer = http_signer_create(client->private_key, client->client_id);
    if (!signer) {
        free(payload);
        return NULL;
    }

    // Prepare URL
    char url[1024];
    snprintf(url, sizeof(url), "%s/digital/v1/addresses", client->base_url);

    // Set up CURL
    CURL *curl = client->curl;
    curl_easy_reset(curl);
    
    // Set URL and method
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    
    // Set up response buffer
    MemoryStruct chunk = {
        .memory = malloc(1),
        .size = 0
    };
    
    if (!chunk.memory) {
        http_signer_destroy(signer);
        free(payload);
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    
    // Set headers
    struct curl_slist *headers = NULL;
    headers = add_common_headers(headers, true);

    // Add signature headers
    if (!http_signer_add_headers(signer, curl, url, payload, "POST", &headers)) {
        http_signer_destroy(signer);
        free(payload);
        free(chunk.memory);
        curl_slist_free_all(headers);
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    // Clean up
    curl_slist_free_all(headers);
    http_signer_destroy(signer);
    free(payload);

    if (res != CURLE_OK) {
        free(chunk.memory);
        return NULL;
    }

    // Parse response
    AddressResponse *response = parse_address_response(chunk.memory);
    free(chunk.memory);

    return response;
}

AddressResponse *layer1_create_address_by_asset(
    Layer1Client *client,
    const char *asset_pool_id,
    const char *asset,
    const char *reference
) {
    if (!client || !asset_pool_id || !asset || !reference) {
        return NULL;
    }

    // Create JSON payload
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "assetPoolId", asset_pool_id);
    cJSON_AddStringToObject(root, "asset", asset);
    cJSON_AddStringToObject(root, "reference", reference);
    
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!payload) {
        return NULL;
    }

    // Create HTTP signer
    HttpSigner *signer = http_signer_create(client->private_key, client->client_id);
    if (!signer) {
        free(payload);
        return NULL;
    }

    // Prepare URL
    char url[1024];
    snprintf(url, sizeof(url), "%s/digital/v1/addresses", client->base_url);

    // Set up CURL
    CURL *curl = client->curl;
    curl_easy_reset(curl);
    
    // Set URL and method
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    
    // Set up response buffer
    MemoryStruct chunk = {
        .memory = malloc(1),
        .size = 0
    };
    
    if (!chunk.memory) {
        http_signer_destroy(signer);
        free(payload);
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    
    // Set headers
    struct curl_slist *headers = NULL;
    headers = add_common_headers(headers, true);

    // Add signature headers
    if (!http_signer_add_headers(signer, curl, url, payload, "POST", &headers)) {
        http_signer_destroy(signer);
        free(payload);
        free(chunk.memory);
        curl_slist_free_all(headers);
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    // Clean up
    curl_slist_free_all(headers);
    http_signer_destroy(signer);
    free(payload);

    if (res != CURLE_OK) {
        free(chunk.memory);
        return NULL;
    }

    // Parse response
    AddressResponse *response = parse_address_response(chunk.memory);
    free(chunk.memory);

    return response;
}

TransactionResponse *layer1_create_transaction(
    Layer1Client *client,
    const char *asset_pool_id,
    const char *network,
    const char *asset,
    const char *to_address,
    const char *amount,
    const char *reference
) {
    if (!client || !client->curl || !asset_pool_id || !network || !asset || !to_address || !amount) {
        return NULL;
    }

    // Build the URL
    char url[1024];
    snprintf(url, sizeof(url), "%s/digital/v1/transaction-requests", client->base_url);

    // Create the JSON request body
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "assetPoolId", asset_pool_id);
    cJSON_AddStringToObject(json, "network", network);
    cJSON_AddStringToObject(json, "asset", asset);
    
    // Create destinations array with a single destination
    cJSON *destinations = cJSON_CreateArray();
    cJSON *destination = cJSON_CreateObject();
    cJSON_AddStringToObject(destination, "address", to_address);
    cJSON_AddStringToObject(destination, "amount", amount);
    cJSON_AddItemToArray(destinations, destination);
    cJSON_AddItemToObject(json, "destinations", destinations);
    
    if (reference) {
        cJSON_AddStringToObject(json, "reference", reference);
    }

    char *request_body = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (!request_body) {
        return NULL;
    }

    // Create HTTP signer
    HttpSigner *signer = http_signer_create(client->private_key, client->client_id);
    if (!signer) {
        free(request_body);
        return NULL;
    }

    // Set up headers
    struct curl_slist *headers = NULL;
    headers = add_common_headers(headers, true);

    // Add signature headers
    if (!http_signer_add_headers(signer, client->curl, url, request_body, "POST", &headers)) {
        http_signer_destroy(signer);
        free(request_body);
        curl_slist_free_all(headers);
        return NULL;
    }

    // Set up the request
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, request_body);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);

    // Prepare response buffer
    MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    if (!chunk.memory) {
        http_signer_destroy(signer);
        free(request_body);
        curl_slist_free_all(headers);
        return NULL;
    }

    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Perform the request
    CURLcode res = curl_easy_perform(client->curl);

    // Clean up request resources
    curl_slist_free_all(headers);
    http_signer_destroy(signer);
    free(request_body);

    if (res != CURLE_OK) {
        free(chunk.memory);
        return NULL;
    }
    
    // Parse the response
    cJSON *response_json = cJSON_Parse(chunk.memory);
    free(chunk.memory);

    if (!response_json) {
        return NULL;
    }

    // Create the response structure
    TransactionResponse *response = (TransactionResponse *)malloc(sizeof(TransactionResponse));
    if (!response) {
        cJSON_Delete(response_json);
        return NULL;
    }

    // Initialize all fields to NULL
    memset(response, 0, sizeof(TransactionResponse));

    // Extract fields from JSON
    cJSON *id = cJSON_GetObjectItem(response_json, "requestId");
    cJSON *status = cJSON_GetObjectItem(response_json, "status");
    cJSON *network_json = cJSON_GetObjectItem(response_json, "network");
    cJSON *asset_json = cJSON_GetObjectItem(response_json, "asset");
    cJSON *reference_json = cJSON_GetObjectItem(response_json, "reference");
    cJSON *created_at = cJSON_GetObjectItem(response_json, "createdAt");

    // Copy values if they exist
    if (cJSON_IsString(id)) response->id = strdup(id->valuestring);
    if (cJSON_IsString(status)) response->status = strdup(status->valuestring);
    if (cJSON_IsString(network_json)) response->network = strdup(network_json->valuestring);
    if (cJSON_IsString(asset_json)) response->asset = strdup(asset_json->valuestring);
    if (cJSON_IsString(reference_json)) response->reference = strdup(reference_json->valuestring);
    if (cJSON_IsString(created_at)) response->createdAt = strdup(created_at->valuestring);

    cJSON_Delete(response_json);
    return response;
}

void layer1_free_transaction_response(TransactionResponse *response) {
    if (!response) {
        return;
    }

    free(response->id);
    free(response->status);
    free(response->network);
    free(response->asset);
    free(response->reference);
    free(response->createdAt);
    free(response);
}

TransactionListResponse *layer1_list_transactions(Layer1Client *client, const char *asset_pool_id, const char *query) {
    if (!client || !asset_pool_id || !query) {
        return NULL;
    }

    // Initialize CURL if not already initialized
    if (!client->curl) {
        client->curl = curl_easy_init();
        if (!client->curl) {
            fprintf(stderr, "Failed to initialize CURL\n");
            return NULL;
        }
    }

    // Build the URL
    char url[1024];
    snprintf(url, sizeof(url), "%s/digital/v1/transactions?assetPoolId=%s&q=%s", 
             client->base_url, asset_pool_id, query);
            
    // Set up the request
    curl_easy_reset(client->curl);
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);

    // Set up response buffer
    MemoryStruct chunk = {0};
    chunk.memory = malloc(1);
    chunk.size = 0;

    if (!chunk.memory) {
        fprintf(stderr, "Failed to allocate memory for response buffer\n");
        return NULL;
    }

    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, &chunk);

    // Create HTTP signer
    HttpSigner *signer = http_signer_create(client->private_key, client->client_id);
    if (!signer) {
        free(chunk.memory);
        return NULL;
    }

    // Set up headers
    struct curl_slist *headers = NULL;
    headers = add_common_headers(headers, true);

    // Add signature headers
    if (!http_signer_add_headers(signer, client->curl, url, NULL, "GET", &headers)) {
        http_signer_destroy(signer);
        free(chunk.memory);
        curl_slist_free_all(headers);
        return NULL;
    }

    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, headers);

    // Make the request
    CURLcode res = curl_easy_perform(client->curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        http_signer_destroy(signer);
        curl_slist_free_all(headers);
        free(chunk.memory);
        return NULL;
    }
    
    // Parse the response
    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        fprintf(stderr, "Failed to parse JSON response\n");
        http_signer_destroy(signer);
        curl_slist_free_all(headers);
        free(chunk.memory);
        return NULL;
    }

    // Create the response structure
    TransactionListResponse *list_response = (TransactionListResponse *)malloc(sizeof(TransactionListResponse));
    if (!list_response) {
        fprintf(stderr, "Failed to allocate memory for response\n");
        cJSON_Delete(json);
        http_signer_destroy(signer);
        curl_slist_free_all(headers);
        free(chunk.memory);
        return NULL;
    }

    // Parse the transactions array
    cJSON *content = cJSON_GetObjectItem(json, "content");
    if (!content || !cJSON_IsArray(content)) {
        fprintf(stderr, "Invalid response format: missing content array\n");
        free(list_response);
        cJSON_Delete(json);
        http_signer_destroy(signer);
        curl_slist_free_all(headers);
        free(chunk.memory);
        return NULL;
    }

    list_response->count = cJSON_GetArraySize(content);
    list_response->transactions = (Transaction *)malloc(sizeof(Transaction) * list_response->count);
    if (!list_response->transactions) {
        fprintf(stderr, "Failed to allocate memory for transactions\n");
        free(list_response);
        cJSON_Delete(json);
        http_signer_destroy(signer);
        curl_slist_free_all(headers);
        free(chunk.memory);
        return NULL;
    }

    // Parse each transaction
    for (int i = 0; i < list_response->count; i++) {
        cJSON *item = cJSON_GetArrayItem(content, i);
        Transaction *tx = &list_response->transactions[i];

        cJSON *id = cJSON_GetObjectItem(item, "id");
        cJSON *status = cJSON_GetObjectItem(item, "status");
        cJSON *asset = cJSON_GetObjectItem(item, "asset");
        cJSON *createdAt = cJSON_GetObjectItem(item, "createdAt");
        cJSON *amount = cJSON_GetObjectItem(item, "amount");

        cJSON *address = cJSON_GetObjectItem(item, "address");
        cJSON *reference = address ? cJSON_GetObjectItem(address, "reference") : NULL;
        cJSON *network = address ? cJSON_GetObjectItem(address, "network") : NULL;
        
        tx->id = id ? strdup(id->valuestring) : NULL;
        tx->status = status ? strdup(status->valuestring) : NULL;
        tx->network = network ? strdup(network->valuestring) : NULL;
        tx->asset = asset ? strdup(asset->valuestring) : NULL;
        tx->reference = reference ? strdup(reference->valuestring) : NULL;
        tx->createdAt = createdAt ? strdup(createdAt->valuestring) : NULL;
        tx->amount = amount ? strdup(amount->valuestring) : NULL;
    }

    // Clean up
    cJSON_Delete(json);
    http_signer_destroy(signer);
    curl_slist_free_all(headers);
    free(chunk.memory);

    return list_response;
}

void layer1_free_transaction_list_response(TransactionListResponse *response) {
    if (!response) {
        return;
    }

    for (int i = 0; i < response->count; i++) {
        Transaction *tx = &response->transactions[i];
        free(tx->id);
        free(tx->status);
        free(tx->network);
        free(tx->asset);
        free(tx->reference);
        free(tx->createdAt);
        free(tx->amount);
    }

    free(response->transactions);
    free(response);
}
