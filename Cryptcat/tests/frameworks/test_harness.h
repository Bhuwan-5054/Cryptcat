/*
 * Test Harness Framework for Cryptcat
 * Version: 1.0.0
 * License: MIT
 */

#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <setjmp.h>
#include <signal.h>

/* Test result codes */
typedef enum {
    TEST_PASS = 0,
    TEST_FAIL = 1,
    TEST_SKIP = 2,
    TEST_ERROR = 3
} test_result_t;

/* Test case structure */
typedef struct test_case_s {
    const char *name;
    test_result_t (*func)(void);
    struct test_case_s *next;
} test_case_t;

/* Test suite structure */
typedef struct test_suite_s {
    const char *name;
    test_case_t *tests;
    int total_tests;
    int passed_tests;
    int failed_tests;
    int skipped_tests;
    int error_tests;
    time_t start_time;
    time_t end_time;
} test_suite_t;

/* Global test context */
typedef struct {
    test_suite_t *current_suite;
    const char *current_test;
    int verbose;
    int stop_on_failure;
    FILE *output_file;
    jmp_buf jump_buffer;
    char error_message[1024];
} test_context_t;

/* Assertion macros */
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            snprintf(test_ctx.error_message, sizeof(test_ctx.error_message), \
                    "Assertion failed: %s at %s:%d", #condition, __FILE__, __LINE__); \
            longjmp(test_ctx.jump_buffer, 1); \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            snprintf(test_ctx.error_message, sizeof(test_ctx.error_message), \
                    "Assertion failed: expected %ld, got %ld at %s:%d", \
                    (long)(expected), (long)(actual), __FILE__, __LINE__); \
            longjmp(test_ctx.jump_buffer, 1); \
        } \
    } while (0)

#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            snprintf(test_ctx.error_message, sizeof(test_ctx.error_message), \
                    "Assertion failed: values equal at %s:%d", __FILE__, __LINE__); \
            longjmp(test_ctx.jump_buffer, 1); \
        } \
    } while (0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            snprintf(test_ctx.error_message, sizeof(test_ctx.error_message), \
                    "Assertion failed: pointer not NULL at %s:%d", __FILE__, __LINE__); \
            longjmp(test_ctx.jump_buffer, 1); \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            snprintf(test_ctx.error_message, sizeof(test_ctx.error_message), \
                    "Assertion failed: pointer is NULL at %s:%d", __FILE__, __LINE__); \
            longjmp(test_ctx.jump_buffer, 1); \
        } \
    } while (0)

#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            snprintf(test_ctx.error_message, sizeof(test_ctx.error_message), \
                    "Assertion failed: strings differ at %s:%d\nExpected: %s\nGot: %s", \
                    __FILE__, __LINE__, (expected), (actual)); \
            longjmp(test_ctx.jump_buffer, 1); \
        } \
    } while (0)

#define TEST_ASSERT_MEMORY_EQUAL(expected, actual, size) \
    do { \
        if (memcmp((expected), (actual), (size)) != 0) { \
            snprintf(test_ctx.error_message, sizeof(test_ctx.error_message), \
                    "Assertion failed: memory differs at %s:%d", __FILE__, __LINE__); \
            longjmp(test_ctx.jump_buffer, 1); \
        } \
    } while (0)

#define TEST_ASSERT_IN_RANGE(min, max, value) \
    do { \
        if ((value) < (min) || (value) > (max)) { \
            snprintf(test_ctx.error_message, sizeof(test_ctx.error_message), \
                    "Assertion failed: %ld not in range [%ld, %ld] at %s:%d", \
                    (long)(value), (long)(min), (long)(max), __FILE__, __LINE__); \
            longjmp(test_ctx.jump_buffer, 1); \
        } \
    } while (0)

/* Test management macros */
#define TEST_CASE(name) \
    static test_result_t name(void)

#define ADD_TEST(suite, test_func) \
    test_suite_add_test(suite, #test_func, test_func)

#define TEST_SUITE(name) \
    test_suite_t *name = test_suite_create(#name)

/* Global test context */
extern test_context_t test_ctx;

/* Test suite functions */
test_suite_t* test_suite_create(const char *name);
void test_suite_destroy(test_suite_t *suite);
void test_suite_add_test(test_suite_t *suite, const char *name, test_result_t (*func)(void));
int test_suite_run(test_suite_t *suite);
void test_suite_print_results(test_suite_t *suite, FILE *stream);

/* Test runner functions */
void test_init(int verbose, int stop_on_failure, const char *output_file);
void test_cleanup(void);
void test_register_suite(test_suite_t *suite);
int test_run_all(void);

/* Utility functions */
void test_log(const char *format, ...);
void test_hex_dump(const char *label, const unsigned char *data, size_t len);

/* Signal handler for timeout */
void test_set_timeout(int seconds);

#endif /* TEST_HARNESS_H */