/*
 * Cryptcat Utilities Unit Tests
 * Version: 1.0.0
 * License: MIT
 */

#include "../../src/utils/logger.h"
#include "../../src/utils/error_handler.h"
#include "../../src/utils/memory_utils.h"
#include "../../src/utils/config_parser.h"
#include "../../src/utils/string_utils.h"
#include "../../src/utils/hex_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counters */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(expr) \
    do { \
        if (expr) { \
            printf("✓ %s\n", #expr); \
            tests_passed++; \
        } else { \
            printf("✗ %s\n", #expr); \
            tests_failed++; \
        } \
    } while(0)

#define TEST_STR_EQ(a, b) \
    do { \
        if (strcmp(a, b) == 0) { \
            printf("✓ %s == %s\n", #a, #b); \
            tests_passed++; \
        } else { \
            printf("✗ %s != %s\n", #a, #b); \
            tests_failed++; \
        } \
    } while(0)

/* Test 1: Logger functionality */
void test_logger(void) {
    printf("\n=== Test 1: Logger ===\n");
    
    /* Initialize logger */
    int result = logger_init("test_utils.log", LOG_INFO, 0);
    TEST(result == 0);
    
    /* Test different log levels */
    LOG_DEBUG("This debug message should not appear (level is INFO)");
    LOG_INFO("This info message should appear");
    LOG_WARNING("This warning message should appear");
    LOG_ERROR("This error message should appear");
    
    /* Change log level */
    logger_set_level(LOG_DEBUG);
    LOG_DEBUG("This debug message should appear now");
    
    /* Get logger status */
    logger_status_t status = logger_get_status();
    TEST(status.initialized == 1);
    TEST(status.level == LOG_DEBUG);
    TEST(strstr(status.output, "test_utils.log") != NULL);
    
    /* Test hex dump */
    unsigned char test_data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    logger_hex_dump(LOG_DEBUG, "Test data", test_data, sizeof(test_data));
    
    /* Shutdown logger */
    logger_shutdown();
    status = logger_get_status();
    TEST(status.initialized == 0);
}

/* Test 2: Error handler */
void test_error_handler(void) {
    printf("\n=== Test 2: Error Handler ===\n");
    
    /* Initialize */
    error_handler_init();
    
    /* Handle some errors */
    ERROR_HANDLE(ERROR_SUCCESS, "Test success");
    ERROR_HANDLE(ERROR_MEMORY, "Memory test");
    ERROR_HANDLE(ERROR_IO, "IO test");
    
    /* Get last error */
    error_info_t last_error = error_handler_get_last();
    TEST(last_error.code == ERROR_IO);
    TEST(strstr(last_error.context, "IO test") != NULL);
    
    /* Test error strings */
    TEST(strcmp(error_handler_strerror(ERROR_SUCCESS), "Success") == 0);
    TEST(strcmp(error_handler_strerror(ERROR_MEMORY), "Memory allocation failed") == 0);
    TEST(strcmp(error_handler_strerror(999), "Unknown error") == 0);
    
    /* Test system error */
    const char *sys_error = error_handler_system_strerror();
    TEST(sys_error != NULL);
    
    /* Test fatal error detection */
    TEST(error_handler_is_fatal(ERROR_CRITICAL) == 1);
    TEST(error_handler_is_fatal(ERROR_SUCCESS) == 0);
    
    /* Clear errors */
    error_handler_clear();
    last_error = error_handler_get_last();
    TEST(last_error.code == 0);
    
    /* Reset handler */
    error_handler_reset();
}

/* Test 3: Memory utilities */
void test_memory_utils(void) {
    printf("\n=== Test 3: Memory Utilities ===\n");
    
    /* Secure allocation */
    void *mem = secure_malloc(100);
    TEST(mem != NULL);
    
    /* Check it's zeroed */
    unsigned char *check = (unsigned char*)mem;
    int all_zero = 1;
    for (int i = 0; i < 100; i++) {
        if (check[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    TEST(all_zero == 1);
    
    /* Secure realloc */
    void *new_mem = secure_realloc(mem, 100, 200);
    TEST(new_mem != NULL);
    
    /* Secure free */
    secure_free(&new_mem, 200);
    TEST(new_mem == NULL);
    
    /* Secure string duplication */
    char *str = secure_strdup("Hello, World!");
    TEST(str != NULL);
    TEST_STR_EQ(str, "Hello, World!");
    
    /* Secure memory comparison */
    unsigned char data1[] = {1, 2, 3, 4, 5};
    unsigned char data2[] = {1, 2, 3, 4, 5};
    unsigned char data3[] = {1, 2, 3, 4, 6};
    
    TEST(secure_memcmp(data1, data2, 5) == 0);
    TEST(secure_memcmp(data1, data3, 5) != 0);
    
    /* Memory zeroing */
    unsigned char sensitive[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    secure_zero(sensitive, 10);
    
    all_zero = 1;
    for (int i = 0; i < 10; i++) {
        if (sensitive[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    TEST(all_zero == 1);
    
    free(str);
    
    /* Memory statistics */
    memory_stats_t stats = get_memory_stats();
    TEST(stats.total > 0);
    TEST(stats.process_used > 0);
}

/* Test 4: Configuration parser */
void test_config_parser(void) {
    printf("\n=== Test 4: Configuration Parser ===\n");
    
    /* Create config */
    config_t *config = config_create("test_config.ini");
    TEST(config != NULL);
    
    /* Set values */
    TEST(config_set_string(config, "network", "host", "127.0.0.1") == CONFIG_SUCCESS);
    TEST(config_set_int(config, "network", "port", 4444) == CONFIG_SUCCESS);
    TEST(config_set_bool(config, "network", "encrypted", 1) == CONFIG_SUCCESS);
    TEST(config_set_float(config, "network", "timeout", 30.5) == CONFIG_SUCCESS);
    
    TEST(config_set_string(config, "logging", "level", "debug") == CONFIG_SUCCESS);
    TEST(config_set_string(config, "logging", "file", "cryptcat.log") == CONFIG_SUCCESS);
    
    /* Get values */
    TEST_STR_EQ(config_get_string(config, "network", "host", ""), "127.0.0.1");
    TEST(config_get_int(config, "network", "port", 0) == 4444);
    TEST(config_get_bool(config, "network", "encrypted", 0) == 1);
    TEST(config_get_float(config, "network", "timeout", 0) == 30.5f);
    
    TEST_STR_EQ(config_get_string(config, "logging", "level", ""), "debug");
    TEST_STR_EQ(config_get_string(config, "logging", "file", ""), "cryptcat.log");
    
    /* Test default values */
    TEST_STR_EQ(config_get_string(config, "nonexistent", "key", "default"), "default");
    TEST(config_get_int(config, "nonexistent", "key", 999) == 999);
    
    /* List sections */
    char **sections;
    int section_count;
    TEST(config_get_sections(config, &sections, &section_count) == CONFIG_SUCCESS);
    TEST(section_count == 2); /* network and logging */
    
    /* Free sections array */
    for (int i = 0; i < section_count; i++) {
        free(sections[i]);
    }
    free(sections);
    
    /* List keys in section */
    char **keys;
    int key_count;
    TEST(config_get_keys(config, "network", &keys, &key_count) == CONFIG_SUCCESS);
    TEST(key_count == 4); /* host, port, encrypted, timeout */
    
    for (int i = 0; i < key_count; i++) {
        free(keys[i]);
    }
    free(keys);
    
    /* Delete key */
    TEST(config_delete_key(config, "network", "timeout") == CONFIG_SUCCESS);
    TEST(config_get_string(config, "network", "timeout", "missing") == "missing");
    
    /* Delete section */
    TEST(config_delete_section(config, "logging") == CONFIG_SUCCESS);
    
    /* Save config to file */
    TEST(config_save(config, "test_config_saved.ini") == CONFIG_SUCCESS);
    
    /* Clear config */
    config_clear(config);
    
    /* Load from file */
    TEST(config_load(config, "test_config_saved.ini") == CONFIG_SUCCESS);
    TEST_STR_EQ(config_get_string(config, "network", "host", ""), "127.0.0.1");
    
    /* Cleanup */
    config_destroy(config);
    
    /* Remove test files */
    remove("test_config.ini");
    remove("test_config_saved.ini");
}

/* Test 5: String utilities */
void test_string_utils(void) {
    printf("\n=== Test 5: String Utilities ===\n");
    
    /* Safe string copy */
    char dest[10];
    TEST(strncpy_safe(dest, "Hello", sizeof(dest)) == dest);
    TEST_STR_EQ(dest, "Hello");
    
    TEST(strncpy_safe(dest, "Hello, World!", sizeof(dest)) == dest);
    TEST(strlen(dest) == 9); /* Truncated */
    
    /* Safe string concatenation */
    strcpy(dest, "Hello");
    TEST(strncat_safe(dest, ", World!", sizeof(dest)) == dest);
    TEST(strlen(dest) == 9); /* Truncated */
    
    /* Case conversion */
    char mixed[] = "Hello World!";
    TEST_STR_EQ(str_tolower(mixed), "hello world!");
    TEST_STR_EQ(str_toupper(mixed), "HELLO WORLD!");
    
    /* Trimming */
    char with_spaces[] = "  Hello World!  ";
    TEST_STR_EQ(str_ltrim(with_spaces), "Hello World!  ");
    
    strcpy(with_spaces, "  Hello World!  ");
    TEST_STR_EQ(str_rtrim(with_spaces), "  Hello World!");
    
    strcpy(with_spaces, "  Hello World!  ");
    TEST_STR_EQ(str_trim(with_spaces), "Hello World!");
    
    /* String checking */
    TEST(str_startswith("Hello World", "Hello") == 1);
    TEST(str_startswith("Hello World", "World") == 0);
    
    TEST(str_endswith("Hello World", "World") == 1);
    TEST(str_endswith("Hello World", "Hello") == 0);
    
    TEST(str_is_digits("12345") == 1);
    TEST(str_is_digits("123a45") == 0);
    
    TEST(str_is_hex("123abc") == 1);
    TEST(str_is_hex("123xyz") == 0);
    
    TEST(str_is_alnum("Hello123") == 1);
    TEST(str_is_alnum("Hello 123") == 0);
    
    TEST(str_is_empty("") == 1);
    TEST(str_is_empty("   ") == 1);
    TEST(str_is_empty("Hello") == 0);
    
    /* String replacement */
    char *replaced = str_replace("Hello World", "World", "Universe");
    TEST_STR_EQ(replaced, "Hello Universe");
    free(replaced);
    
    /* String splitting */
    str_split_t *split = str_split("one,two,three", ",");
    TEST(split != NULL);
    TEST(split->count == 3);
    TEST_STR_EQ(split->tokens[0], "one");
    TEST_STR_EQ(split->tokens[1], "two");
    TEST_STR_EQ(split->tokens[2], "three");
    str_split_free(split);
    
    /* String joining */
    const char *words[] = {"Hello", "World", "!"};
    char *joined = str_join(words, 3, " ");
    TEST_STR_EQ(joined, "Hello World !");
    free(joined);
    
    /* String formatting */
    char *formatted = str_format("Number: %d, String: %s", 42, "test");
    TEST_STR_EQ(formatted, "Number: 42, String: test");
    free(formatted);
    
    /* Escape/unescape */
    char *escaped = str_escape("Hello,World!", ",!", '\\');
    TEST_STR_EQ(escaped, "Hello\\,World\\!");
    free(escaped);
    
    char *unescaped = str_unescape("Hello\\,World\\!", '\\');
    TEST_STR_EQ(unescaped, "Hello,World!");
    free(unescaped);
    
    /* String to number conversion */
    int int_val;
    TEST(str_to_int("123", &int_val) == 1);
    TEST(int_val == 123);
    TEST(str_to_int("123abc", &int_val) == 0);
    
    unsigned int uint_val;
    TEST(str_to_uint("456", &uint_val) == 1);
    TEST(uint_val == 456);
    
    float float_val;
    TEST(str_to_float("3.14", &float_val) == 1);
    TEST(float_val > 3.13 && float_val < 3.15);
    
    /* Character counting */
    TEST(str_count_char("Hello World", 'o') == 2);
    TEST(str_count_char("Hello World", 'z') == 0);
}

/* Test 6: Hex utilities */
void test_hex_utils(void) {
    printf("\n=== Test 6: Hex Utilities ===\n");
    
    /* Binary to hex */
    unsigned char bin_data[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    char *hex = bin_to_hex(bin_data, sizeof(bin_data));
    TEST(hex != NULL);
    TEST_STR_EQ(hex, "0011223344556677");
    free(hex);
    
    /* Hex to binary */
    size_t bin_len;
    unsigned char *bin = hex_to_bin("0011223344556677", &bin_len);
    TEST(bin != NULL);
    TEST(bin_len == 8);
    TEST(memcmp(bin, bin_data, 8) == 0);
    free(bin);
    
    /* Invalid hex */
    bin = hex_to_bin("001122334455667", &bin_len); /* Odd length */
    TEST(bin == NULL);
    
    bin = hex_to_bin("00112233GG556677", &bin_len); /* Invalid char */
    TEST(bin == NULL);
    
    /* Hex character conversion */
    TEST(hex_char_to_value('0') == 0);
    TEST(hex_char_to_value('9') == 9);
    TEST(hex_char_to_value('a') == 10);
    TEST(hex_char_to_value('f') == 15);
    TEST(hex_char_to_value('A') == 10);
    TEST(hex_char_to_value('F') == 15);
    TEST(hex_char_to_value('g') == -1);
    
    TEST(value_to_hex_char(0) == '0');
    TEST(value_to_hex_char(9) == '9');
    TEST(value_to_hex_char(10) == 'a');
    TEST(value_to_hex_char(15) == 'f');
    TEST(value_to_hex_char(16) == '\0');
    
    /* Hex validation */
    TEST(is_valid_hex("123abc") == 1);
    TEST(is_valid_hex("123ABC") == 1);
    TEST(is_valid_hex("123xyz") == 0);
    TEST(is_valid_hex("") == 1);
    
    /* Hex dump */
    char *dump = hex_dump(bin_data, sizeof(bin_data), 8);
    TEST(dump != NULL);
    TEST(strstr(dump, "00112233") != NULL);
    free(dump);
    
    /* Base64 encoding/decoding */
    unsigned char test_data[] = "Hello, Base64!";
    char *base64 = bin_to_base64(test_data, strlen((char*)test_data));
    TEST(base64 != NULL);
    
    size_t decoded_len;
    unsigned char *decoded = base64_to_bin(base64, &decoded_len);
    TEST(decoded != NULL);
    TEST(decoded_len == strlen((char*)test_data));
    TEST(memcmp(test_data, decoded, decoded_len) == 0);
    
    free(base64);
    free(decoded);
    
    /* Checksums */
    uint32_t crc = crc32(bin_data, sizeof(bin_data));
    TEST(crc != 0);
    
    uint16_t checksum = simple_checksum(bin_data, sizeof(bin_data));
    TEST(checksum != 0);
}

/* Main test runner */
int main(void) {
    printf("========================================\n");
    printf("     CRYPTCAT UTILITIES UNIT TESTS\n");
    printf("========================================\n");
    
    /* Run tests */
    test_logger();
    test_error_handler();
    test_memory_utils();
    test_config_parser();
    test_string_utils();
    test_hex_utils();
    
    /* Print summary */
    printf("\n========================================\n");
    printf("TEST SUMMARY:\n");
    printf("  Tests passed: %d\n", tests_passed);
    printf("  Tests failed: %d\n", tests_failed);
    printf("  Total tests:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✅ ALL UTILITIES TESTS PASSED!\n");
    } else {
        printf("\n❌ SOME UTILITIES TESTS FAILED!\n");
    }
    
    /* Cleanup test files */
    remove("test_utils.log");
    
    return tests_failed == 0 ? 0 : 1;
}