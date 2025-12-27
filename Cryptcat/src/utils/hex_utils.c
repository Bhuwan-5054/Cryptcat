/*
 * Hexadecimal Utilities
 * Version: 1.0.0
 * License: MIT
 */

#include "hex_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Convert binary data to hex string */
char* bin_to_hex(const unsigned char *data, size_t len) {
    if (!data || len == 0) {
        return NULL;
    }
    
    char *hex = malloc(len * 2 + 1);
    if (!hex) {
        return NULL;
    }
    
    for (size_t i = 0; i < len; i++) {
        sprintf(hex + i * 2, "%02x", data[i]);
    }
    
    hex[len * 2] = '\0';
    return hex;
}

/* Convert hex string to binary data */
unsigned char* hex_to_bin(const char *hex, size_t *len) {
    if (!hex || !len) {
        return NULL;
    }
    
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0) {
        return NULL;
    }
    
    *len = hex_len / 2;
    unsigned char *bin = malloc(*len);
    if (!bin) {
        return NULL;
    }
    
    for (size_t i = 0; i < *len; i++) {
        char byte_str[3] = {hex[i * 2], hex[i * 2 + 1], '\0'};
        bin[i] = (unsigned char)strtol(byte_str, NULL, 16);
    }
    
    return bin;
}

/* Convert single hex character to value */
int hex_char_to_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return -1;
    }
}

/* Convert value to hex character */
char value_to_hex_char(int value) {
    if (value >= 0 && value <= 9) {
        return '0' + value;
    } else if (value >= 10 && value <= 15) {
        return 'a' + (value - 10);
    } else {
        return '\0';
    }
}

/* Check if string is valid hex */
int is_valid_hex(const char *str) {
    if (!str) {
        return 0;
    }
    
    for (; *str; str++) {
        if (!isxdigit(*str)) {
            return 0;
        }
    }
    
    return 1;
}

/* Format hex dump with address and ASCII */
char* hex_dump(const unsigned char *data, size_t len, int bytes_per_line) {
    if (!data || len == 0 || bytes_per_line <= 0) {
        return NULL;
    }
    
    /* Calculate output size:
       Address (8 chars) + space (1) + hex bytes (bytes_per_line * 3)
       + space (1) + ASCII (bytes_per_line) + newline (1) */
    size_t lines = (len + bytes_per_line - 1) / bytes_per_line;
    size_t line_size = 8 + 1 + (bytes_per_line * 3) + 1 + bytes_per_line + 1;
    size_t total_size = lines * line_size + 1;
    
    char *dump = malloc(total_size);
    if (!dump) {
        return NULL;
    }
    
    char *ptr = dump;
    
    for (size_t offset = 0; offset < len; offset += bytes_per_line) {
        /* Address */
        ptr += sprintf(ptr, "%08zx: ", offset);
        
        /* Hex bytes */
        for (int i = 0; i < bytes_per_line; i++) {
            if (offset + i < len) {
                ptr += sprintf(ptr, "%02x ", data[offset + i]);
            } else {
                ptr += sprintf(ptr, "   ");
            }
            
            /* Extra space after 8 bytes */
            if (i == 7) {
                *ptr++ = ' ';
            }
        }
        
        /* ASCII representation */
        *ptr++ = ' ';
        *ptr++ = '|';
        
        for (int i = 0; i < bytes_per_line && offset + i < len; i++) {
            unsigned char c = data[offset + i];
            *ptr++ = (c >= 32 && c < 127) ? c : '.';
        }
        
        *ptr++ = '|';
        *ptr++ = '\n';
    }
    
    *ptr = '\0';
    return dump;
}

/* Encode binary data as Base64 */
char* bin_to_base64(const unsigned char *data, size_t len) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    if (!data || len == 0) {
        return NULL;
    }
    
    size_t out_len = 4 * ((len + 2) / 3);
    char *base64 = malloc(out_len + 1);
    if (!base64) {
        return NULL;
    }
    
    size_t i = 0, j = 0;
    
    while (len--) {
        unsigned char octet_a = i < len ? data[i++] : 0;
        unsigned char octet_b = i < len ? data[i++] : 0;
        unsigned char octet_c = i < len ? data[i++] : 0;
        
        unsigned int triple = (octet_a << 16) | (octet_b << 8) | octet_c;
        
        base64[j++] = base64_chars[(triple >> 18) & 0x3F];
        base64[j++] = base64_chars[(triple >> 12) & 0x3F];
        base64[j++] = base64_chars[(triple >> 6) & 0x3F];
        base64[j++] = base64_chars[triple & 0x3F];
    }
    
    /* Add padding */
    for (size_t k = 0; k < (3 - (i % 3)) % 3; k++) {
        base64[out_len - 1 - k] = '=';
    }
    
    base64[out_len] = '\0';
    return base64;
}

/* Decode Base64 string to binary */
unsigned char* base64_to_bin(const char *base64, size_t *len) {
    static const int base64_index[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };
    
    if (!base64 || !len) {
        return NULL;
    }
    
    size_t base64_len = strlen(base64);
    if (base64_len % 4 != 0) {
        return NULL;
    }
    
    /* Count padding characters */
    size_t padding = 0;
    if (base64_len > 0 && base64[base64_len - 1] == '=') {
        padding++;
        if (base64_len > 1 && base64[base64_len - 2] == '=') {
            padding++;
        }
    }
    
    *len = (base64_len / 4) * 3 - padding;
    unsigned char *bin = malloc(*len);
    if (!bin) {
        return NULL;
    }
    
    size_t i = 0, j = 0;
    
    while (base64_len--) {
        int c = base64_index[(unsigned char)*base64++];
        if (c == -1) {
            continue;
        }
        
        switch (i % 4) {
            case 0:
                bin[j] = c << 2;
                break;
            case 1:
                bin[j++] |= c >> 4;
                bin[j] = (c & 0x0F) << 4;
                break;
            case 2:
                bin[j++] |= c >> 2;
                bin[j] = (c & 0x03) << 6;
                break;
            case 3:
                bin[j++] |= c;
                break;
        }
        
        i++;
    }
    
    return bin;
}

/* Calculate CRC32 checksum */
uint32_t crc32(const unsigned char *data, size_t len) {
    static const uint32_t crc_table[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
        0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
        0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
        0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
        0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
        0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
        0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
        0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
        0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
        0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
        0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
        0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
        0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
        0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
        0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
        0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
        0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
        0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
        0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
        0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
        0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
        0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
        0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
        0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
        0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
        0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
        0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
        0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
        0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
        0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
        0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
        0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
        0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };
    
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < len; i++) {
        crc = (crc >> 8) ^ crc_table[(crc ^ data[i]) & 0xFF];
    }
    
    return ~crc;
}

/* Calculate simple checksum (for non-cryptographic purposes) */
uint16_t simple_checksum(const unsigned char *data, size_t len) {
    uint32_t sum = 0;
    
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    
    /* Fold 32-bit sum to 16 bits */
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return (uint16_t)~sum;
}