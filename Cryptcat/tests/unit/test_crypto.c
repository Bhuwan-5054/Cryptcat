/*
 * Cryptographic Unit Tests
 * Version: 1.0.0
 * License: MIT
 */

#include "test_harness.h"
#include "../../src/include/crypto.h"
#include <string.h>

/* Test data */
static const char *test_password = "TestPassword123!";
static const unsigned char test_plaintext[] = "Hello, Cryptcat! This is a test message.";
static const size_t test_plaintext_len = sizeof(test_plaintext) - 1;

/* Test: Crypto initialization */
TEST_CASE(test_crypto_init) {
    int result = crypto_global_init();
    TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
    return TEST_PASS;
}

/* Test: Session creation */
TEST_CASE(test_session_create) {
    crypto_session_t *session = crypto_session_create(test_password);
    TEST_ASSERT_NOT_NULL(session);
    
    crypto_info_t info = crypto_get_info(session);
    TEST_ASSERT_EQUAL(1, info.is_authenticated);
    
    crypto_session_destroy(session);
    return TEST_PASS;
}

/* Test: Session creation with invalid password */
TEST_CASE(test_session_create_invalid) {
    /* NULL password */
    crypto_session_t *session1 = crypto_session_create(NULL);
    TEST_ASSERT_NULL(session1);
    
    /* Empty password */
    crypto_session_t *session2 = crypto_session_create("");
    TEST_ASSERT_NULL(session2);
    
    /* Very long password */
    char long_password[2000];
    memset(long_password, 'A', sizeof(long_password) - 1);
    long_password[sizeof(long_password) - 1] = '\0';
    
    crypto_session_t *session3 = crypto_session_create(long_password);
    TEST_ASSERT_NULL(session3);
    
    return TEST_PASS;
}

/* Test: Basic encryption/decryption */
TEST_CASE(test_basic_encryption) {
    crypto_session_t *session = crypto_session_create(test_password);
    TEST_ASSERT_NOT_NULL(session);
    
    /* Encrypt */
    unsigned char ciphertext[1024];
    size_t ciphertext_len;
    
    int result = crypto_encrypt(session, test_plaintext, test_plaintext_len,
                               ciphertext, &ciphertext_len);
    TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
    TEST_ASSERT(ciphertext_len > 0);
    
    /* Decrypt */
    unsigned char decrypted[1024];
    size_t decrypted_len;
    
    result = crypto_decrypt(session, ciphertext, ciphertext_len,
                           decrypted, &decrypted_len);
    TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
    TEST_ASSERT_EQUAL(test_plaintext_len, decrypted_len);
    TEST_ASSERT_MEMORY_EQUAL(test_plaintext, decrypted, decrypted_len);
    
    crypto_session_destroy(session);
    return TEST_PASS;
}

/* Test: Multiple encryption operations */
TEST_CASE(test_multiple_encryption) {
    crypto_session_t *session = crypto_session_create(test_password);
    TEST_ASSERT_NOT_NULL(session);
    
    const char *messages[] = {
        "Short",
        "Medium length message",
        "Very long message that exceeds typical buffer sizes and tests boundary conditions",
        ""
    };
    
    for (int i = 0; i < 4; i++) {
        size_t msg_len = strlen(messages[i]);
        if (msg_len == 0) continue;
        
        unsigned char ciphertext[1024];
        size_t ciphertext_len;
        
        int result = crypto_encrypt(session, (unsigned char*)messages[i], msg_len,
                                   ciphertext, &ciphertext_len);
        TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
        
        unsigned char decrypted[1024];
        size_t decrypted_len;
        
        result = crypto_decrypt(session, ciphertext, ciphertext_len,
                               decrypted, &decrypted_len);
        TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
        TEST_ASSERT_EQUAL(msg_len, decrypted_len);
        TEST_ASSERT_MEMORY_EQUAL(messages[i], decrypted, decrypted_len);
    }
    
    crypto_session_destroy(session);
    return TEST_PASS;
}

/* Test: Tampered ciphertext detection */
TEST_CASE(test_tamper_detection) {
    crypto_session_t *session = crypto_session_create(test_password);
    TEST_ASSERT_NOT_NULL(session);
    
    /* Encrypt original message */
    unsigned char ciphertext[1024];
    size_t ciphertext_len;
    
    int result = crypto_encrypt(session, test_plaintext, test_plaintext_len,
                               ciphertext, &ciphertext_len);
    TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
    
    /* Tamper with ciphertext */
    ciphertext[10] ^= 0xFF; /* Flip some bits */
    
    /* Try to decrypt tampered data */
    unsigned char decrypted[1024];
    size_t decrypted_len;
    
    result = crypto_decrypt(session, ciphertext, ciphertext_len,
                           decrypted, &decrypted_len);
    TEST_ASSERT_NOT_EQUAL(CRYPTO_SUCCESS, result);
    TEST_ASSERT(result == CRYPTO_ERROR_AUTH);
    
    crypto_session_destroy(session);
    return TEST_PASS;
}

/* Test: Replay attack protection */
TEST_CASE(test_replay_protection) {
    crypto_session_t *session = crypto_session_create(test_password);
    TEST_ASSERT_NOT_NULL(session);
    
    /* Encrypt first message */
    unsigned char ciphertext1[1024];
    size_t ciphertext_len1;
    
    int result = crypto_encrypt(session, test_plaintext, test_plaintext_len,
                               ciphertext1, &ciphertext_len1);
    TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
    
    /* Decrypt first message */
    unsigned char decrypted1[1024];
    size_t decrypted_len1;
    
    result = crypto_decrypt(session, ciphertext1, ciphertext_len1,
                           decrypted1, &decrypted_len1);
    TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
    
    /* Try to decrypt same ciphertext again (replay attack) */
    result = crypto_decrypt(session, ciphertext1, ciphertext_len1,
                           decrypted1, &decrypted_len1);
    TEST_ASSERT_NOT_EQUAL(CRYPTO_SUCCESS, result);
    TEST_ASSERT(result == CRYPTO_ERROR_AUTH);
    
    crypto_session_destroy(session);
    return TEST_PASS;
}

/* Test: Random number generation */
TEST_CASE(test_random_generation) {
    unsigned char random1[32], random2[32];
    
    /* Generate two random sequences */
    int result = crypto_random_bytes(random1, sizeof(random1));
    TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
    
    result = crypto_random_bytes(random2, sizeof(random2));
    TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
    
    /* They should be different (very high probability) */
    int different = 0;
    for (size_t i = 0; i < sizeof(random1); i++) {
        if (random1[i] != random2[i]) {
            different = 1;
            break;
        }
    }
    TEST_ASSERT(different);
    
    /* Test with zero length */
    result = crypto_random_bytes(random1, 0);
    TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, result);
    
    /* Test with NULL buffer */
    result = crypto_random_bytes(NULL, sizeof(random1));
    TEST_ASSERT_NOT_EQUAL(CRYPTO_SUCCESS, result);
    
    return TEST_PASS;
}

/* Test: Session information */
TEST_CASE(test_session_info) {
    crypto_session_t *session = crypto_session_create(test_password);
    TEST_ASSERT_NOT_NULL(session);
    
    crypto_info_t info = crypto_get_info(session);
    
    TEST_ASSERT_EQUAL(1, info.is_authenticated);
    TEST_ASSERT(info.bytes_sent == 0);
    TEST_ASSERT(info.bytes_received == 0);
    TEST_ASSERT(info.session_age >= 0);
    TEST_ASSERT