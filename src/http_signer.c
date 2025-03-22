#include "http_signer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/core_names.h>
#include <openssl/decoder.h>

// Base64 encoding function
static char *base64_encode(const unsigned char *input, int length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    char *result = (char *)malloc(bufferPtr->length + 1);
    if (!result) {
        BIO_free_all(bio);
        return NULL;
    }

    memcpy(result, bufferPtr->data, bufferPtr->length);
    result[bufferPtr->length] = 0;

    BIO_free_all(bio);
    return result;
}

HttpSigner *http_signer_create(const char *private_key, const char *client_id) {
    if (!private_key || !client_id) {
        return NULL;
    }

    HttpSigner *signer = (HttpSigner *)malloc(sizeof(HttpSigner));
    if (!signer) {
        return NULL;
    }

    // Initialize with NULL values
    signer->signing_key = NULL;
    signer->client_id = NULL;

    // Prepare the private key
    char *prepared_key = prepare_key(private_key);
    if (!prepared_key) {
        free(signer);
        return NULL;
    }

    // Initialize OpenSSL error strings
    ERR_load_crypto_strings();
    
    // Load the private key using the OSSL decoder
    BIO *bio = BIO_new_mem_buf(prepared_key, -1);
    if (!bio) {
        free(prepared_key);
        free(signer);
        return NULL;
    }

    // Try to read the private key directly first
    signer->signing_key = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    
    if (!signer->signing_key) {
        // Reset BIO position
        BIO_reset(bio);
        
        // Create a decoder context
        OSSL_DECODER_CTX *dctx = OSSL_DECODER_CTX_new_for_pkey(&signer->signing_key,
                                                              "PEM",   // Input format
                                                              NULL,    // Input type
                                                              "RSA",   // Key type
                                                              OSSL_KEYMGMT_SELECT_PRIVATE_KEY,
                                                              NULL,    // Selection criteria
                                                              NULL);   // Library context
        
        if (dctx) {
            // Attempt to decode the private key
            if (!OSSL_DECODER_from_bio(dctx, bio)) {
                EVP_PKEY_free(signer->signing_key);
                signer->signing_key = NULL;
            }
            OSSL_DECODER_CTX_free(dctx);
        }
    }
    
    BIO_free(bio);
    
    if (!signer->signing_key) {
        free(prepared_key);
        free(signer);
        return NULL;
    }
    
    free(prepared_key);

    // Copy client ID
    signer->client_id = strdup(client_id);
    if (!signer->client_id) {
        EVP_PKEY_free(signer->signing_key);
        free(signer);
        return NULL;
    }

    return signer;
}

void http_signer_destroy(HttpSigner *signer) {
    if (!signer) {
        return;
    }

    if (signer->signing_key) {
        EVP_PKEY_free(signer->signing_key);
    }

    free(signer->client_id);
    free(signer);
}

char *create_digest(const char *algorithm, const char *data) {
    if (!data) {
        return NULL;
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        return NULL;
    }
    
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        return NULL;
    }
    
    if (EVP_DigestUpdate(mdctx, data, strlen(data)) != 1) {
        EVP_MD_CTX_free(mdctx);
        return NULL;
    }
    
    unsigned int digest_len = SHA256_DIGEST_LENGTH;
    if (EVP_DigestFinal_ex(mdctx, hash, &digest_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        return NULL;
    }
    
    EVP_MD_CTX_free(mdctx);

    char *base64_hash = base64_encode(hash, SHA256_DIGEST_LENGTH);
    if (!base64_hash) {
        return NULL;
    }

    // Format: sha-256=:base64_hash:
    size_t result_len = strlen(algorithm) + 3 + strlen(base64_hash) + 2;
    char *result = (char *)malloc(result_len);
    if (!result) {
        free(base64_hash);
        return NULL;
    }

    snprintf(result, result_len, "%s=:%s:", algorithm, base64_hash);
    free(base64_hash);
    return result;
}

char *create_signature_parameters(const char *client_id, const char *content_digest) {
    time_t now = time(NULL);
    
    // Format: ("@method" "@target-uri" "content-digest");created=timestamp;keyid="client_id";alg="rsa-v1_5-sha256"
    size_t result_len = 100 + (content_digest ? 20 : 0) + strlen(client_id);
    char *result = (char *)malloc(result_len);
    if (!result) {
        return NULL;
    }

    if (content_digest) {
        snprintf(result, result_len, "(\"@method\" \"@target-uri\" \"content-digest\");created=%ld;keyid=\"%s\";alg=\"rsa-v1_5-sha256\"", 
                 now, client_id);
    } else {
        snprintf(result, result_len, "(\"@method\" \"@target-uri\");created=%ld;keyid=\"%s\";alg=\"rsa-v1_5-sha256\"", 
                 now, client_id);
    }

    return result;
}

char *sign_request(EVP_PKEY *private_key, const char *signature_base) {
    if (!private_key || !signature_base) {
        return NULL;
    }

    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) {
        return NULL;
    }

    if (EVP_DigestSignInit(md_ctx, NULL, EVP_sha256(), NULL, private_key) != 1) {
        EVP_MD_CTX_free(md_ctx);
        return NULL;
    }

    if (EVP_DigestSignUpdate(md_ctx, signature_base, strlen(signature_base)) != 1) {
        EVP_MD_CTX_free(md_ctx);
        return NULL;
    }

    size_t sig_len = 0;
    if (EVP_DigestSignFinal(md_ctx, NULL, &sig_len) != 1) {
        EVP_MD_CTX_free(md_ctx);
        return NULL;
    }

    unsigned char *sig = (unsigned char *)malloc(sig_len);
    if (!sig) {
        EVP_MD_CTX_free(md_ctx);
        return NULL;
    }

    if (EVP_DigestSignFinal(md_ctx, sig, &sig_len) != 1) {
        free(sig);
        EVP_MD_CTX_free(md_ctx);
        return NULL;
    }

    EVP_MD_CTX_free(md_ctx);

    char *base64_sig = base64_encode(sig, sig_len);
    free(sig);
    return base64_sig;
}

char *prepare_key(const char *raw_key) {
    if (!raw_key) {
        return NULL;
    }

    // Instead of trying to parse the key ourselves, let's return the full key
    // and let OpenSSL handle the parsing
    return strdup(raw_key);
}

bool http_signer_add_headers(HttpSigner *signer, CURL *curl, const char *url, 
                            const char *payload, const char *method, 
                            struct curl_slist **headers) {
    if (!signer || !curl || !url || !method || !headers) {
        return false;
    }

    char *content_digest = NULL;
    if (payload && strlen(payload) > 0) {
        content_digest = create_digest("sha-256", payload);
        if (!content_digest) {
            return false;
        }
        
        char digest_header[1024];
        snprintf(digest_header, sizeof(digest_header), "Content-Digest: %s", content_digest);
        *headers = curl_slist_append(*headers, digest_header);
    }

    // Create signature parameters
    char *sig_params = create_signature_parameters(signer->client_id, content_digest);
    if (!sig_params) {
        free(content_digest);
        return false;
    }

    // Add Signature-Input header
    char sig_input_header[2048];
    snprintf(sig_input_header, sizeof(sig_input_header), "Signature-Input: sig=%s", sig_params);
    *headers = curl_slist_append(*headers, sig_input_header);

    // Create signature base
    char *signature_base;
    if (content_digest) {
        size_t sig_base_len = strlen(method) + strlen(url) + strlen(content_digest) + strlen(sig_params) + 100;
        signature_base = (char *)malloc(sig_base_len);
        if (!signature_base) {
            free(content_digest);
            free(sig_params);
            return false;
        }
        snprintf(signature_base, sig_base_len, 
                "\"@method\": %s\n\"@target-uri\": %s\n\"content-digest\": %s\n\"@signature-params\": %s",
                method, url, content_digest, sig_params);
    } else {
        size_t sig_base_len = strlen(method) + strlen(url) + strlen(sig_params) + 100;
        signature_base = (char *)malloc(sig_base_len);
        if (!signature_base) {
            free(sig_params);
            return false;
        }
        snprintf(signature_base, sig_base_len, 
                "\"@method\": %s\n\"@target-uri\": %s\n\"@signature-params\": %s",
                method, url, sig_params);
    }

    // Sign the request
    char *signature = sign_request(signer->signing_key, signature_base);
    if (!signature) {
        free(content_digest);
        free(sig_params);
        free(signature_base);
        return false;
    }

    // Add Signature header
    char sig_header[4096];
    snprintf(sig_header, sizeof(sig_header), "Signature: sig=:%s:", signature);
    *headers = curl_slist_append(*headers, sig_header);

    // Clean up
    free(content_digest);
    free(sig_params);
    free(signature_base);
    free(signature);

    return true;
}
