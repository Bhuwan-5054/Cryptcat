/*
 * Cryptcat Cryptographic API
 * Header file for crypto_engine.c
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque session handle */
typedef struct crypto_session_s crypto_session_t;

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

/* Session information structure */
typedef struct {
    char algorithm[32];
    char mode[32];
    char key_derivation[64];
    int is_authenticated;
    uint32_t bytes_sent;
    uint32_t bytes_received;
    time_t session_age;
    time_t last_activity;
} crypto_info_t;

/* ========== Core API Functions ========== */

/**
 * Initialize the cryptographic subsystem.
 * Must be called before any other crypto functions.
 * 
 * @return CRYPTO_SUCCESS on success, error code on failure
 */
int crypto_global_init(void);

/**
 * Create a new cryptographic session with the given password.
 * 
 * @param password The password for key derivation
 * @return Pointer to new session, or NULL on failure
 */
crypto_session_t* crypto_session_create(const char *password);

/**
 * Encrypt data with authentication.
 * 
 * @param session Cryptographic session
 * @param plaintext Data to encrypt
 * @param plaintext_len Length of plaintext
 * @param ciphertext Output buffer for ciphertext
 * @param ciphertext_len Output parameter for ciphertext length
 * @return CRYPTO_SUCCESS on success, error code on failure
 */
int crypto_encrypt(crypto_session_t *session, const unsigned char *plaintext,
                   size_t plaintext_len, unsigned char *ciphertext,
                   size_t *ciphertext_len);

/**
 * Decrypt and verify data.
 * 
 * @param session Cryptographic session
 * @param ciphertext Data to decrypt
 * @param ciphertext_len Length of ciphertext
 * @param plaintext Output buffer for plaintext
 * @param plaintext_len Output parameter for plaintext length
 * @return CRYPTO_SUCCESS on success, error code on failure
 */
int crypto_decrypt(crypto_session_t *session, const unsigned char *ciphertext,
                   size_t ciphertext_len, unsigned char *plaintext,
                   size_t *plaintext_len);

/**
 * Generate cryptographically secure random bytes.
 * 
 * @param buffer Output buffer
 * @param length Number of bytes to generate
 * @return CRYPTO_SUCCESS on success, error code on failure
 */
int crypto_random_bytes(unsigned char *buffer, size_t length);

/**
 * Destroy a cryptographic session and securely wipe all keys.
 * 
 * @param session Session to destroy
 */
void crypto_session_destroy(crypto_session_t *session);

/**
 * Get information about a cryptographic session.
 * 
 * @param session Cryptographic session
 * @return Session information structure
 */
crypto_info_t crypto_get_info(crypto_session_t *session);

/**
 * Get a human-readable error message for an error code.
 * 
 * @param error_code Error code
 * @return Error message string
 */
const char* crypto_strerror(int error_code);

/* ========== Utility Functions ========== */

/**
 * Secure memory comparison (constant-time).
 * 
 * @param a First buffer
 * @param b Second buffer
 * @param len Number of bytes to compare
 * @return 0 if equal, non-zero otherwise
 */
int crypto_memcmp(const void *a, const void *b, size_t len);

/**
 * Securely wipe memory.
 * 
 * @param ptr Pointer to memory
 * @param len Number of bytes to wipe
 */
void crypto_memzero(void *ptr, size_t len);

/**
 * Generate a cryptographic hash (SHA256).
 * 
 * @param data Input data
 * @param data_len Length of input data
 * @param hash Output buffer (32 bytes)
 * @return CRYPTO_SUCCESS on success, error code on failure
 */
int crypto_hash(const unsigned char *data, size_t data_len, unsigned char *hash);

/**
 * Derive a key from a password using PBKDF2.
 * 
 * @param password The password
 * @param salt Salt value
 * @param salt_len Salt length
 * @param iterations Number of iterations
 * @param key Output buffer for derived key
 * @param key_len Desired key length
 * @return CRYPTO_SUCCESS on success, error code on failure
 */
int crypto_pbkdf2(const char *password, const unsigned char *salt, size_t salt_len,
                  int iterations, unsigned char *key, size_t key_len);

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_H */