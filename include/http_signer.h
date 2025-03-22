#ifndef HTTP_SIGNER_H
#define HTTP_SIGNER_H

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <curl/curl.h>
#include <stdbool.h>

typedef struct {
    EVP_PKEY *signing_key;
    char *client_id;
} HttpSigner;

// Initialize the HTTP signer with a private key and client ID
HttpSigner *http_signer_create(const char *private_key, const char *client_id);

// Free resources used by the HTTP signer
void http_signer_destroy(HttpSigner *signer);

// Add authentication headers to a CURL handle
bool http_signer_add_headers(HttpSigner *signer, CURL *curl, const char *url, 
                            const char *payload, const char *method, 
                            struct curl_slist **headers);

// Helper functions
char *create_digest(const char *algorithm, const char *data);
char *create_signature_parameters(const char *client_id, const char *content_digest);
char *sign_request(EVP_PKEY *private_key, const char *signature_base);
char *prepare_key(const char *raw_key);

#endif // HTTP_SIGNER_H
