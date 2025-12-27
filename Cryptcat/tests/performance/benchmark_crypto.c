/*
 * Cryptcat Performance Benchmarks
 * Measures encryption throughput, latency, and resource usage
 */

#include "test_harness.h"
#include "../../src/include/crypto.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

/* ===== Benchmark Utilities ===== */

typedef struct {
    double elapsed_ms;
    long long bytes_processed;
    double throughput_mbps;
} bench_result_t;

static uint64_t get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

/* ===== Benchmark Tests ===== */

/* Benchmark: Encryption throughput */
TEST_CASE(bench_encrypt_throughput) {
    crypto_global_init();
    crypto_session_t *session = crypto_session_create("bench_pwd");
    TEST_ASSERT_NOT_NULL(session);

    const size_t plaintext_size = 1024 * 1024;  /* 1MB */
    const size_t iterations = 10;

    unsigned char plaintext[4096];
    unsigned char ciphertext[4096 + 64];  /* +64 for IV+HMAC */
    size_t ciphertext_len;

    memset(plaintext, 0x42, sizeof(plaintext));

    uint64_t start_us = get_time_us();

    for (size_t i = 0; i < iterations; i++) {
        for (size_t j = 0; j < plaintext_size / sizeof(plaintext); j++) {
            int ret = crypto_encrypt(session, plaintext, sizeof(plaintext),
                                    ciphertext, &ciphertext_len);
            TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, ret);
        }
    }

    uint64_t end_us = get_time_us();
    double elapsed_ms = (double)(end_us - start_us) / 1000.0;
    double bytes = (double)plaintext_size * iterations;
    double throughput_mbps = (bytes / elapsed_ms) / 1024.0;

    test_log("Encryption throughput: %.2f MB/s (%.2f ms for %lld bytes)",
             throughput_mbps, elapsed_ms, (long long)bytes);

    crypto_session_destroy(session);
    return TEST_PASS;
}

/* Benchmark: Decryption throughput */
TEST_CASE(bench_decrypt_throughput) {
    crypto_global_init();
    crypto_session_t *session = crypto_session_create("bench_pwd");
    TEST_ASSERT_NOT_NULL(session);

    const size_t plaintext_size = 1024 * 1024;  /* 1MB */
    const size_t iterations = 10;

    unsigned char plaintext[4096];
    unsigned char ciphertext[4096 + 64];
    unsigned char decrypted[4096];
    size_t ciphertext_len, decrypted_len;

    memset(plaintext, 0x42, sizeof(plaintext));

    uint64_t start_us = get_time_us();

    for (size_t i = 0; i < iterations; i++) {
        for (size_t j = 0; j < plaintext_size / sizeof(plaintext); j++) {
            int ret = crypto_encrypt(session, plaintext, sizeof(plaintext),
                                    ciphertext, &ciphertext_len);
            TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, ret);

            ret = crypto_decrypt(session, ciphertext, ciphertext_len,
                                decrypted, &decrypted_len);
            TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, ret);
        }
    }

    uint64_t end_us = get_time_us();
    double elapsed_ms = (double)(end_us - start_us) / 1000.0;
    double bytes = (double)plaintext_size * iterations * 2;  /* encrypt + decrypt */
    double throughput_mbps = (bytes / elapsed_ms) / 1024.0;

    test_log("Encryption+Decryption throughput: %.2f MB/s (%.2f ms for %lld bytes)",
             throughput_mbps, elapsed_ms, (long long)bytes);

    crypto_session_destroy(session);
    return TEST_PASS;
}

/* Benchmark: Session creation latency */
TEST_CASE(bench_session_creation) {
    crypto_global_init();
    const int iterations = 100;

    uint64_t start_us = get_time_us();

    for (int i = 0; i < iterations; i++) {
        crypto_session_t *session = crypto_session_create("bench_pwd");
        TEST_ASSERT_NOT_NULL(session);
        crypto_session_destroy(session);
    }

    uint64_t end_us = get_time_us();
    double elapsed_ms = (double)(end_us - start_us) / 1000.0;
    double avg_latency_ms = elapsed_ms / iterations;

    test_log("Session creation latency: %.2f ms average (%d iterations, %.2f ms total)",
             avg_latency_ms, iterations, elapsed_ms);

    return TEST_PASS;
}

/* Benchmark: Random number generation */
TEST_CASE(bench_random_generation) {
    const int iterations = 1000;
    const size_t rand_size = 32;
    unsigned char buffer[rand_size];

    uint64_t start_us = get_time_us();

    for (int i = 0; i < iterations; i++) {
        int ret = crypto_random_bytes(buffer, rand_size);
        TEST_ASSERT_EQUAL(CRYPTO_SUCCESS, ret);
    }

    uint64_t end_us = get_time_us();
    double elapsed_ms = (double)(end_us - start_us) / 1000.0;
    double bytes = (double)rand_size * iterations;
    double throughput_mbps = (bytes / elapsed_ms) / 1024.0;

    test_log("Random generation: %.2f MB/s (%.2f ms for %lld bytes)",
             throughput_mbps, elapsed_ms, (long long)bytes);

    return TEST_PASS;
}

/* ===== Suite Registration ===== */

__attribute__((constructor)) static void register_benchmark_tests(void) {
    test_suite_t *suite = test_suite_create("performance_benchmarks");
    if (!suite) return;

    test_suite_add_test(suite, "bench_encrypt_throughput", bench_encrypt_throughput);
    test_suite_add_test(suite, "bench_decrypt_throughput", bench_decrypt_throughput);
    test_suite_add_test(suite, "bench_session_creation", bench_session_creation);
    test_suite_add_test(suite, "bench_random_generation", bench_random_generation);

    test_register_suite(suite);
}
