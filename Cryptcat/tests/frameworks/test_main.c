/*
 * Simple test runner executable for Cryptcat tests
 */

#include "test_harness.h"

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    /* Initialize test system: verbose=1, stop_on_failure=0, output_file=NULL (stdout) */
    test_init(1, 0, NULL);

    /* Run any registered suites (use test_register_suite in test files to add suites) */
    int failures = test_run_all();

    /* Cleanup and exit with failure count */
    test_cleanup();
    return failures;
}
