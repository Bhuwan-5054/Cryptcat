#ifndef HEX_UTILS_H
#define HEX_UTILS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hex conversion */
char* bin_to_hex(const unsigned char *data, size_t len);
unsigned char* hex_to_bin(const char *hex, size_t *len);

/* Character conversion */
int hex_char_to_value(char c);
char value_to_hex_char(int value);

/* Validation */
int is_valid_hex(const char *str);

/* Hex dump formatting */
char* hex_dump(const unsigned char *data, size_t len, int bytes_per_line);

/* Base64 encoding/decoding */
char* bin_to_base64(const unsigned char *data, size_t len);
unsigned char* base64_to_bin(const char *base64, size_t *len);

/* Checksums */
uint32_t crc32(const unsigned char *data, size_t len);
uint16_t simple_checksum(const unsigned char *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* HEX_UTILS_H */