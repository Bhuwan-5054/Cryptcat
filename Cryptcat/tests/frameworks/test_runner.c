/*
 * Test Runner Implementation
 * Version: 1.0.0
 * License: MIT
 */

#include "test_harness.h"
#include <stdarg.h>
#include <sys/time.h>

/* Global test context */
test_context_t test_ctx = {0};

/* Registered test suites */
static test_suite_t **test_suites = NULL;
static int num_suites = 0;
static int max_suites = 0;

/* Signal handler for timeout */
static void timeout_handler(int sig) {
    (void)sig;
    test_log("Test timeout reached!");
    longjmp(test_ctx.jump_buffer, 1);
}

/* Create a new test suite */
test_suite_t* test_suite_create(const char *name) {
    test_suite_t *suite = malloc(sizeof(test_suite_t));
    if (!suite) {
        return NULL;
    }
    
    memset(suite, 0, sizeof(test_suite_t));
    suite->name = strdup(name);
    suite->tests = NULL;
    suite->total_tests = 0;
    suite->passed_tests = 0;
    suite->failed_tests = 0;
    suite->skipped_tests = 0;
    suite->error_tests = 0;
    
    return suite;
}

/* Destroy a test suite */
void test_suite_destroy(test_suite_t *suite) {
    if (!suite) return;
    
    test_case_t *test = suite->tests;
    while (test) {
        test_case_t *next = test->next;
        free((void*)test->name);
        free(test);
        test = next;
    }
    
    free((void*)suite->name);
    free(suite);
}

/* Add a test to a suite */
void test_suite_add_test(test_suite_t *suite, const char *name, test_result_t (*func)(void)) {
    test_case_t *test = malloc(sizeof(test_case_t));
    if (!test) return;
    
    test->name = strdup(name);
    test->func = func;
    test->next = suite->tests;
    suite->tests = test;
    suite->total_tests++;
}

/* Run a single test case */
static test_result_t run_test_case(test_case_t *test) {
    test_result_t result = TEST_ERROR;
    
    test_ctx.current_test = test->name;
    test_ctx.error_message[0] = '\0';
    
    test_log("Running test: %s", test->name);
    
    if (setjmp(test_ctx.jump_buffer) == 0) {
        result = test->func();
    } else {
        result = TEST_FAIL;
        if (test_ctx.error_message[0] != '\0') {
            test_log("  FAIL: %s", test_ctx.error_message);
        }
    }
    
    switch (result) {
        case TEST_PASS:
            test_log("  PASS");
            break;
        case TEST_FAIL:
            test_log("  FAIL");
            break;
        case TEST_SKIP:
            test_log("  SKIP");
            break;
        case TEST_ERROR:
            test_log("  ERROR");
            break;
    }
    
    return result;
}

/* Run all tests in a suite */
int test_suite_run(test_suite_t *suite) {
    if (!suite) return -1;
    
    test_ctx.current_suite = suite;
    suite->start_time = time(NULL);
    
    test_log("\n=== Running test suite: %s ===", suite->name);
    
    test_case_t *test = suite->tests;
    while (test) {
        test_result_t result = run_test_case(test);
        
        switch (result) {
            case TEST_PASS:
                suite->passed_tests++;
                break;
            case TEST_FAIL:
                suite->failed_tests++;
                if (test_ctx.stop_on_failure) {
                    test_log("Stopping on first failure");
                    goto done;
                }
                break;
            case TEST_SKIP:
                suite->skipped_tests++;
                break;
            case TEST_ERROR:
                suite->error_tests++;
                break;
        }
        
        test = test->next;
    }
    
done:
    suite->end_time = time(NULL);
    return suite->failed_tests + suite->error_tests;
}

/* Print test suite results */
void test_suite_print_results(test_suite_t *suite, FILE *stream) {
    if (!suite || !stream) return;
    
    double elapsed = difftime(suite->end_time, suite->start_time);
    
    fprintf(stream, "\n=== Test Suite: %s ===\n", suite->name);
    fprintf(stream, "Total tests:  %d\n", suite->total_tests);
    fprintf(stream, "Passed:       %d\n", suite->passed_tests);
    fprintf(stream, "Failed:       %d\n", suite->failed_tests);
    fprintf(stream, "Skipped:      %d\n", suite->skipped_tests);
    fprintf(stream, "Errors:       %d\n", suite->error_tests);
    fprintf(stream, "Time elapsed: %.2f seconds\n", elapsed);
    
    if (suite->failed_tests == 0 && suite->error_tests == 0) {
        fprintf(stream, "RESULT: PASS\n");
    } else {
        fprintf(stream, "RESULT: FAIL\n");
    }
}

/* Initialize test system */
void test_init(int verbose, int stop_on_failure, const char *output_file) {
    memset(&test_ctx, 0, sizeof(test_ctx));
    test_ctx.verbose = verbose;
    test_ctx.stop_on_failure = stop_on_failure;
    
    if (output_file) {
        test_ctx.output_file = fopen(output_file, "w");
        if (!test_ctx.output_file) {
            test_ctx.output_file = stdout;
        }
    } else {
        test_ctx.output_file = stdout;
    }
    
    /* Allocate space for test suites */
    max_suites = 10;
    test_suites = malloc(max_suites * sizeof(test_suite_t*));
    num_suites = 0;
    
    /* Install signal handler for timeout */
    signal(SIGALRM, timeout_handler);
}

/* Cleanup test system */
void test_cleanup(void) {
    for (int i = 0; i < num_suites; i++) {
        test_suite_destroy(test_suites[i]);
    }
    
    free(test_suites);
    
    if (test_ctx.output_file && test_ctx.output_file != stdout) {
        fclose(test_ctx.output_file);
    }
}

/* Register a test suite */
void test_register_suite(test_suite_t *suite) {
    if (num_suites >= max_suites) {
        max_suites *= 2;
        test_suites = realloc(test_suites, max_suites * sizeof(test_suite_t*));
    }
    
    test_suites[num_suites++] = suite;
}

/* Run all registered test suites */
int test_run_all(void) {
    int total_failures = 0;
    time_t start_time = time(NULL);
    
    test_log("Starting all test suites...\n");
    
    for (int i = 0; i < num_suites; i++) {
        int failures = test_suite_run(test_suites[i]);
        total_failures += failures;
        
        test_suite_print_results(test_suites[i], test_ctx.output_file);
        
        if (failures > 0 && test_ctx.stop_on_failure) {
            test_log("Stopping all tests due to failure");
            break;
        }
    }
    
    time_t end_time = time(NULL);
    double total_elapsed = difftime(end_time, start_time);
    
    fprintf(test_ctx.output_file, "\n=== FINAL RESULTS ===\n");
    fprintf(test_ctx.output_file, "Total suites: %d\n", num_suites);
    fprintf(test_ctx.output_file, "Total failures: %d\n", total_failures);
    fprintf(test_ctx.output_file, "Total time: %.2f seconds\n", total_elapsed);
    
    if (total_failures == 0) {
        fprintf(test_ctx.output_file, "OVERALL RESULT: ALL TESTS PASSED\n");
    } else {
        fprintf(test_ctx.output_file, "OVERALL RESULT: %d TEST(S) FAILED\n", total_failures);
    }
    
    return total_failures;
}

/* Log a test message */
void test_log(const char *format, ...) {
    if (!test_ctx.verbose && test_ctx.output_file != stdout) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vfprintf(test_ctx.output_file, format, args);
    fprintf(test_ctx.output_file, "\n");
    va_end(args);
}

/* Hex dump for debugging */
void test_hex_dump(const char *label, const unsigned char *data, size_t len) {
    if (!test_ctx.verbose) return;
    
    test_log("%s (%zu bytes):", label, len);
    
    for (size_t i = 0; i < len; i += 16) {
        char line[128];
        char *ptr = line;
        
        ptr += sprintf(ptr, "  %04zx: ", i);
        
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                ptr += sprintf(ptr, "%02x ", data[i + j]);
            } else {
                ptr += sprintf(ptr, "   ");
            }
            
            if (j == 7) {
                ptr += sprintf(ptr, " ");
            }
        }
        
        ptr += sprintf(ptr, " |");
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                unsigned char c = data[i + j];
                ptr += sprintf(ptr, "%c", (c >= 32 && c < 127) ? c : '.');
            } else {
                ptr += sprintf(ptr, " ");
            }
        }
        ptr += sprintf(ptr, "|");
        
        test_log("%s", line);
    }
}

/* Set timeout for tests */
void test_set_timeout(int seconds) {
    struct itimerval timer;
    
    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    
    setitimer(ITIMER_REAL, &timer, NULL);
}