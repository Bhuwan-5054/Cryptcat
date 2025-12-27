/*
 * Cryptcat Cryptographic Engine
 * Twofish-256-CFB with HMAC-SHA256 authentication
 * Version: 1.0.0
 * License: MIT
 */

#include "crypto.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/err.h>

/* Configuration constants */
#define KEY_SIZE 32               /* Twofish-256 key size (256 bits) */
#define IV_SIZE 16                /* CFB IV size (128 bits) */
#define SALT_SIZE 32              /* PBKDF2 salt size */
#define HMAC_SIZE 32              /* SHA256 HMAC size */
#define PBKDF2_ITERATIONS 200000  /* Key derivation iterations */
#define MAX_PASSWORD_LEN 1024     /* Maximum password length */
#define TAG_SIZE 16               /* Authentication tag size */
#define BUFFER_SIZE 65536         /* Default buffer size */

/* Error codes */
typedef enum {
    CRYPTO_SUCCESS = 0,
    CRYPTO_ERROR_INVALID_PARAM = -1,
    CRYPTO_ERROR_MEMORY = -2,
    CRYPTO_ERROR_RANDOM = -3,
    CRYPTO_ERROR_ENCRYPT = -4,
    CRYPTO_ERROR_DECRYPT = -5,
    CRYPTO_ERROR_AUTH = -6,
    CRYPTO_ERROR_KEY = -7,
    CRYPTO_ERROR_INIT = -8,
    CRYPTO_ERROR_VERSION = -9
} crypto_error_t;

/* Session context structure */
typedef struct crypto_session_s {
    unsigned char enc_key[KEY_SIZE];     /* Encryption key */
    unsigned char hmac_key[KEY_SIZE];    /* HMAC key */
    unsigned char iv[IV_SIZE];           /* Initialization vector */
    EVP_CIPHER_CTX *encrypt_ctx;         /* OpenSSL encryption context */
    EVP_CIPHER_CTX *decrypt_ctx;         /* OpenSSL decryption context */
    uint64_t seq_num_send;               /* Outgoing sequence number */
    uint64_t seq_num_recv;               /* Incoming sequence number */
    time_t created_at;                   /* Session creation time */
    time_t last_activity;                /* Last activity timestamp */
    uint32_t bytes_sent;                 /* Total bytes sent */
    uint32_t bytes_received;             /* Total bytes received */
    int is_authenticated;                /* Authentication status */
    int is_initialized;                  /* Initialization status */
} crypto_session_t;

/* Internal function prototypes */
static int derive_keys(const char *password, const unsigned char *salt,
                      unsigned char *enc_key, unsigned char *hmac_key);
static int generate_random_bytes(unsigned char *buffer, size_t length);
static int verify_hmac(const unsigned char *data, size_t data_len,
                      const unsigned char *hmac_key, const unsigned char *hmac);
static int calculate_hmac(const unsigned char *data, size_t data_len,
                         const unsigned char *hmac_key, unsigned char *hmac);

/* Initialize the cryptographic subsystem */
int crypto_global_init(void) {
    static int initialized = 0;
    
    if (initialized) {
        return CRYPTO_SUCCESS;
    }
    
    /* Initialize OpenSSL */
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CONFIG, NULL);
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    /* Seed random number generator */
    if (RAND_status() != 1) {
        /* Try to seed with platform-specific entropy */
        if (platform_random_seed() != 0) {
            fprintf(stderr, "Warning: Insufficient random entropy\n");
        }
    }
    
    initialized = 1;
    return CRYPTO_SUCCESS;
}

/* Create a new cryptographic session */
crypto_session_t* crypto_session_create(const char *password) {
    crypto_session_t *session = NULL;
    unsigned char salt[SALT_SIZE];
    
    if (!password || strlen(password) == 0) {
        fprintf(stderr, "Error: Invalid password\n");
        return NULL;
    }
    
    if (strlen(password) > MAX_PASSWORD_LEN) {
        fprintf(stderr, "Error: Password too long (max %d characters)\n", 
                MAX_PASSWORD_LEN);
        return NULL;
    }
    
    /* Allocate and initialize session structure */
    session = calloc(1, sizeof(crypto_session_t));
    if (!session) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }
    
    /* Generate random salt and IV */
    if (!generate_random_bytes(salt, SALT_SIZE) ||
        !generate_random_bytes(session->iv, IV_SIZE)) {
        fprintf(stderr, "Error: Failed to generate random bytes\n");
        free(session);
        return NULL;
    }
    
    /* Derive encryption and HMAC keys from password */
    if (derive_keys(password, salt, session->enc_key, session->hmac_key) != CRYPTO_SUCCESS) {
        fprintf(stderr, "Error: Key derivation failed\n");
        memset(session, 0, sizeof(crypto_session_t));
        free(session);
        return NULL;
    }
    
    /* Initialize OpenSSL encryption context */
    session->encrypt_ctx = EVP_CIPHER_CTX_new();
    if (!session->encrypt_ctx) {
        fprintf(stderr, "Error: Failed to create encryption context\n");
        goto error_cleanup;
    }
    
    if (EVP_EncryptInit_ex(session->encrypt_ctx, EVP_twofish_cfb128(), NULL,
                          session->enc_key, session->iv) != 1) {
        fprintf(stderr, "Error: Encryption initialization failed\n");
        goto error_cleanup;
    }
    
    /* Initialize OpenSSL decryption context */
    session->decrypt_ctx = EVP_CIPHER_CTX_new();
    if (!session->decrypt_ctx) {
        fprintf(stderr, "Error: Failed to create decryption context\n");
        goto error_cleanup;
    }
    
    if (EVP_DecryptInit_ex(session->decrypt_ctx, EVP_twofish_cfb128(), NULL,
                          session->enc_key, session->iv) != 1) {
        fprintf(stderr, "Error: Decryption initialization failed\n");
        goto error_cleanup;
    }
    
    /* Initialize session metadata */
    session->seq_num_send = 0;
    session->seq_num_recv = 0;
    session->created_at = time(NULL);
    session->last_activity = session->created_at;
    session->is_authenticated = 1;
    session->is_initialized = 1;
    
    /* Securely wipe sensitive temporary data */
    memset(salt, 0, SALT_SIZE);
    
    return session;
    
error_cleanup:
    if (session->encrypt_ctx) EVP_CIPHER_CTX_free(session->encrypt_ctx);
    if (session->decrypt_ctx) EVP_CIPHER_CTX_free(session->decrypt_ctx);
    memset(session, 0, sizeof(crypto_session_t));
    free(session);
    return NULL;
}

/* Encrypt data with authentication */
int crypto_encrypt(crypto_session_t *session, const unsigned char *plaintext,
                   size_t plaintext_len, unsigned char *ciphertext,
                   size_t *ciphertext_len) {
    int out_len, final_len;
    unsigned char encrypted_data[plaintext_len + EVP_MAX_BLOCK_LENGTH];
    unsigned char hmac[HMAC_SIZE];
    uint64_t seq_be;
    
    if (!session || !session->is_initialized || !session->is_authenticated) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    if (!plaintext || plaintext_len == 0 || !ciphertext || !ciphertext_len) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    if (plaintext_len > BUFFER_SIZE) {
        fprintf(stderr, "Error: Plaintext too large (%zu bytes)\n", plaintext_len);
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    /* Encrypt the plaintext */
    if (EVP_EncryptUpdate(session->encrypt_ctx, encrypted_data, &out_len,
                         plaintext, plaintext_len) != 1) {
        fprintf(stderr, "Error: Encryption update failed\n");
        return CRYPTO_ERROR_ENCRYPT;
    }
    
    if (EVP_EncryptFinal_ex(session->encrypt_ctx, encrypted_data + out_len,
                           &final_len) != 1) {
        fprintf(stderr, "Error: Encryption finalization failed\n");
        return CRYPTO_ERROR_ENCRYPT;
    }
    
    out_len += final_len;
    
    /* Add sequence number for replay protection (network byte order) */
    seq_be = htobe64(session->seq_num_send);
    memcpy(ciphertext, &seq_be, sizeof(uint64_t));
    
    /* Copy encrypted data */
    memcpy(ciphertext + sizeof(uint64_t), encrypted_data, out_len);
    
    /* Calculate HMAC over sequence number + encrypted data */
    if (calculate_hmac(ciphertext, sizeof(uint64_t) + out_len,
                      session->hmac_key, hmac) != CRYPTO_SUCCESS) {
        return CRYPTO_ERROR_AUTH;
    }
    
    /* Append HMAC */
    memcpy(ciphertext + sizeof(uint64_t) + out_len, hmac, HMAC_SIZE);
    
    /* Update output length */
    *ciphertext_len = sizeof(uint64_t) + out_len + HMAC_SIZE;
    
    /* Update session statistics */
    session->seq_num_send++;
    session->bytes_sent += *ciphertext_len;
    session->last_activity = time(NULL);
    
    /* Securely wipe temporary buffers */
    memset(encrypted_data, 0, plaintext_len + EVP_MAX_BLOCK_LENGTH);
    memset(hmac, 0, HMAC_SIZE);
    
    return CRYPTO_SUCCESS;
}

/* Decrypt and verify data */
int crypto_decrypt(crypto_session_t *session, const unsigned char *ciphertext,
                   size_t ciphertext_len, unsigned char *plaintext,
                   size_t *plaintext_len) {
    int out_len, final_len;
    uint64_t seq_received, seq_be;
    unsigned char calculated_hmac[HMAC_SIZE];
    const unsigned char *encrypted_data;
    size_t encrypted_len;
    
    if (!session || !session->is_initialized || !session->is_authenticated) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    if (!ciphertext || ciphertext_len == 0 || !plaintext || !plaintext_len) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    /* Minimum packet size: seq_num(8) + at least 1 byte data + HMAC(32) */
    if (ciphertext_len < sizeof(uint64_t) + 1 + HMAC_SIZE) {
        fprintf(stderr, "Error: Ciphertext too short (%zu bytes)\n", ciphertext_len);
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    /* Extract and verify sequence number */
    memcpy(&seq_be, ciphertext, sizeof(uint64_t));
    seq_received = be64toh(seq_be);
    
    /* Replay protection */
    if (seq_received <= session->seq_num_recv) {
        fprintf(stderr, "Error: Replay attack detected (seq %lu <= %lu)\n",
                seq_received, session->seq_num_recv);
        return CRYPTO_ERROR_AUTH;
    }
    
    /* Verify HMAC */
    encrypted_len = ciphertext_len - sizeof(uint64_t) - HMAC_SIZE;
    if (calculate_hmac(ciphertext, ciphertext_len - HMAC_SIZE,
                      session->hmac_key, calculated_hmac) != CRYPTO_SUCCESS) {
        return CRYPTO_ERROR_AUTH;
    }
    
    if (CRYPTO_memcmp(calculated_hmac, ciphertext + ciphertext_len - HMAC_SIZE,
                     HMAC_SIZE) != 0) {
        fprintf(stderr, "Error: HMAC verification failed\n");
        return CRYPTO_ERROR_AUTH;
    }
    
    /* Decrypt the data */
    encrypted_data = ciphertext + sizeof(uint64_t);
    
    if (EVP_DecryptUpdate(session->decrypt_ctx, plaintext, &out_len,
                         encrypted_data, encrypted_len) != 1) {
        fprintf(stderr, "Error: Decryption update failed\n");
        return CRYPTO_ERROR_DECRYPT;
    }
    
    if (EVP_DecryptFinal_ex(session->decrypt_ctx, plaintext + out_len,
                           &final_len) != 1) {
        fprintf(stderr, "Error: Decryption finalization failed\n");
        return CRYPTO_ERROR_DECRYPT;
    }
    
    out_len += final_len;
    *plaintext_len = out_len;
    
    /* Update session state */
    session->seq_num_recv = seq_received;
    session->bytes_received += ciphertext_len;
    session->last_activity = time(NULL);
    
    /* Securely wipe temporary buffer */
    memset(calculated_hmac, 0, HMAC_SIZE);
    
    return CRYPTO_SUCCESS;
}

/* Generate secure random bytes */
int crypto_random_bytes(unsigned char *buffer, size_t length) {
    if (!buffer || length == 0) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    if (RAND_bytes(buffer, length) != 1) {
        /* Fallback to platform-specific RNG */
        return generate_random_bytes(buffer, length);
    }
    
    return CRYPTO_SUCCESS;
}

/* Destroy cryptographic session */
void crypto_session_destroy(crypto_session_t *session) {
    if (!session) return;
    
    /* Securely wipe all sensitive data */
    if (session->encrypt_ctx) {
        EVP_CIPHER_CTX_free(session->encrypt_ctx);
        session->encrypt_ctx = NULL;
    }
    
    if (session->decrypt_ctx) {
        EVP_CIPHER_CTX_free(session->decrypt_ctx);
        session->decrypt_ctx = NULL;
    }
    
    /* Zero out keys and IV */
    memset(session->enc_key, 0, KEY_SIZE);
    memset(session->hmac_key, 0, KEY_SIZE);
    memset(session->iv, 0, IV_SIZE);
    
    /* Zero entire structure before freeing */
    memset(session, 0, sizeof(crypto_session_t));
    free(session);
}

/* Get session information */
crypto_info_t crypto_get_info(crypto_session_t *session) {
    crypto_info_t info = {0};
    
    if (session) {
        info.is_authenticated = session->is_authenticated;
        info.bytes_sent = session->bytes_sent;
        info.bytes_received = session->bytes_received;
        info.session_age = time(NULL) - session->created_at;
        info.last_activity = time(NULL) - session->last_activity;
    }
    
    strncpy(info.algorithm, "Twofish-256-CFB", sizeof(info.algorithm) - 1);
    strncpy(info.mode, "CFB with HMAC-SHA256", sizeof(info.mode) - 1);
    strncpy(info.key_derivation, "PBKDF2-HMAC-SHA256 (200k iterations)", 
            sizeof(info.key_derivation) - 1);
    
    return info;
}

/* Get error string */
const char* crypto_strerror(int error_code) {
    switch (error_code) {
        case CRYPTO_SUCCESS:
            return "Success";
        case CRYPTO_ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case CRYPTO_ERROR_MEMORY:
            return "Memory allocation failed";
        case CRYPTO_ERROR_RANDOM:
            return "Random number generation failed";
        case CRYPTO_ERROR_ENCRYPT:
            return "Encryption failed";
        case CRYPTO_ERROR_DECRYPT:
            return "Decryption failed";
        case CRYPTO_ERROR_AUTH:
            return "Authentication failed";
        case CRYPTO_ERROR_KEY:
            return "Key derivation failed";
        case CRYPTO_ERROR_INIT:
            return "Initialization failed";
        case CRYPTO_ERROR_VERSION:
            return "Version mismatch";
        default:
            return "Unknown error";
    }
}

/* Internal function: Derive keys using PBKDF2 */
static int derive_keys(const char *password, const unsigned char *salt,
                      unsigned char *enc_key, unsigned char *hmac_key) {
    unsigned char derived[KEY_SIZE * 2]; /* Enough for both keys */
    
    if (!password || !salt || !enc_key || !hmac_key) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    /* Use PBKDF2 with SHA256 to derive key material */
    if (PKCS5_PBKDF2_HMAC_SHA256(password, strlen(password),
                                 salt, SALT_SIZE,
                                 PBKDF2_ITERATIONS,
                                 KEY_SIZE * 2, derived) != 1) {
        return CRYPTO_ERROR_KEY;
    }
    
    /* Split derived material into encryption and HMAC keys */
    memcpy(enc_key, derived, KEY_SIZE);
    memcpy(hmac_key, derived + KEY_SIZE, KEY_SIZE);
    
    /* Securely wipe the temporary buffer */
    memset(derived, 0, KEY_SIZE * 2);
    
    return CRYPTO_SUCCESS;
}

/* Internal function: Generate random bytes */
static int generate_random_bytes(unsigned char *buffer, size_t length) {
    return platform_random_bytes(buffer, length);
}

/* Internal function: Calculate HMAC */
static int calculate_hmac(const unsigned char *data, size_t data_len,
                         const unsigned char *hmac_key, unsigned char *hmac) {
    unsigned int hmac_len;
    
    if (!data || !hmac_key || !hmac) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
    if (HMAC(EVP_sha256(), hmac_key, KEY_SIZE,
             data, data_len, hmac, &hmac_len) == NULL) {
        return CRYPTO_ERROR_AUTH;
    }
    
    return CRYPTO_SUCCESS;
}

/* Internal function: Verify HMAC */
static int verify_hmac(const unsigned char *data, size_t data_len,
                      const unsigned char *hmac_key, const unsigned char *hmac) {
    unsigned char calculated_hmac[HMAC_SIZE];
    
    if (calculate_hmac(data, data_len, hmac_key, calculated_hmac) != CRYPTO_SUCCESS) {
        return CRYPTO_ERROR_AUTH;
    }
    
    if (CRYPTO_memcmp(calculated_hmac, hmac, HMAC_SIZE) != 0) {
        memset(calculated_hmac, 0, HMAC_SIZE);
        return CRYPTO_ERROR_AUTH;
    }
    
    memset(calculated_hmac, 0, HMAC_SIZE);
    return CRYPTO_SUCCESS;
}