/*
 * String Utilities
 * Version: 1.0.0
 * License: MIT
 */

#include "string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* Safe string copy with truncation */
char* strncpy_safe(char *dest, const char *src, size_t dest_size) {
    if (!dest || dest_size == 0) {
        return NULL;
    }
    
    if (!src) {
        dest[0] = '\0';
        return dest;
    }
    
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    dest[i] = '\0';
    return dest;
}

/* Safe string concatenation with truncation */
char* strncat_safe(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return dest;
    }
    
    size_t dest_len = strlen(dest);
    if (dest_len >= dest_size) {
        return dest;
    }
    
    return strncpy_safe(dest + dest_len, src, dest_size - dest_len);
}

/* Convert string to lowercase */
char* str_tolower(char *str) {
    if (!str) return NULL;
    
    for (char *p = str; *p; p++) {
        *p = tolower(*p);
    }
    
    return str;
}

/* Convert string to uppercase */
char* str_toupper(char *str) {
    if (!str) return NULL;
    
    for (char *p = str; *p; p++) {
        *p = toupper(*p);
    }
    
    return str;
}

/* Trim leading whitespace */
char* str_ltrim(char *str) {
    if (!str) return NULL;
    
    char *p = str;
    while (*p && isspace(*p)) {
        p++;
    }
    
    if (p != str) {
        memmove(str, p, strlen(p) + 1);
    }
    
    return str;
}

/* Trim trailing whitespace */
char* str_rtrim(char *str) {
    if (!str) return NULL;
    
    char *p = str + strlen(str) - 1;
    while (p >= str && isspace(*p)) {
        p--;
    }
    
    *(p + 1) = '\0';
    return str;
}

/* Trim both leading and trailing whitespace */
char* str_trim(char *str) {
    str_ltrim(str);
    str_rtrim(str);
    return str;
}

/* Check if string starts with prefix */
int str_startswith(const char *str, const char *prefix) {
    if (!str || !prefix) return 0;
    
    size_t str_len = strlen(str);
    size_t prefix_len = strlen(prefix);
    
    if (prefix_len > str_len) return 0;
    
    return strncmp(str, prefix, prefix_len) == 0;
}

/* Check if string ends with suffix */
int str_endswith(const char *str, const char *suffix) {
    if (!str || !suffix) return 0;
    
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len) return 0;
    
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

/* Replace all occurrences of a substring */
char* str_replace(char *str, const char *old, const char *new) {
    if (!str || !old || !new) return str;
    
    size_t old_len = strlen(old);
    size_t new_len = strlen(new);
    size_t str_len = strlen(str);
    
    /* Count occurrences */
    int count = 0;
    const char *tmp = str;
    while ((tmp = strstr(tmp, old)) != NULL) {
        count++;
        tmp += old_len;
    }
    
    /* Allocate new string */
    size_t new_size = str_len + count * (new_len - old_len) + 1;
    char *result = malloc(new_size);
    if (!result) return NULL;
    
    char *current = result;
    const char *start = str;
    
    while (count--) {
        const char *found = strstr(start, old);
        size_t segment_len = found - start;
        
        memcpy(current, start, segment_len);
        current += segment_len;
        
        memcpy(current, new, new_len);
        current += new_len;
        
        start = found + old_len;
    }
    
    /* Copy remaining string */
    strcpy(current, start);
    
    return result;
}

/* Split string by delimiter */
str_split_t* str_split(const char *str, const char *delim) {
    if (!str || !delim) return NULL;
    
    str_split_t *split = malloc(sizeof(str_split_t));
    if (!split) return NULL;
    
    split->count = 0;
    split->tokens = NULL;
    
    /* Make a copy for tokenization */
    char *str_copy = strdup(str);
    if (!str_copy) {
        free(split);
        return NULL;
    }
    
    /* First pass: count tokens */
    char *token = strtok(str_copy, delim);
    while (token) {
        split->count++;
        token = strtok(NULL, delim);
    }
    
    /* Allocate token array */
    split->tokens = malloc(split->count * sizeof(char*));
    if (!split->tokens) {
        free(str_copy);
        free(split);
        return NULL;
    }
    
    /* Second pass: copy tokens */
    strcpy(str_copy, str); /* Restore original string */
    
    int i = 0;
    token = strtok(str_copy, delim);
    while (token) {
        split->tokens[i] = strdup(token);
        if (!split->tokens[i]) {
            /* Cleanup on error */
            for (int j = 0; j < i; j++) {
                free(split->tokens[j]);
            }
            free(split->tokens);
            free(str_copy);
            free(split);
            return NULL;
        }
        i++;
        token = strtok(NULL, delim);
    }
    
    free(str_copy);
    return split;
}

/* Free split result */
void str_split_free(str_split_t *split) {
    if (!split) return;
    
    for (int i = 0; i < split->count; i++) {
        free(split->tokens[i]);
    }
    
    free(split->tokens);
    free(split);
}

/* Join strings with delimiter */
char* str_join(const char **strings, int count, const char *delim) {
    if (!strings || count <= 0 || !delim) return NULL;
    
    /* Calculate total length */
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        if (strings[i]) {
            total_len += strlen(strings[i]);
        }
    }
    
    total_len += (count - 1) * strlen(delim) + 1;
    
    /* Allocate result */
    char *result = malloc(total_len);
    if (!result) return NULL;
    
    result[0] = '\0';
    
    /* Join strings */
    for (int i = 0; i < count; i++) {
        if (strings[i]) {
            strcat(result, strings[i]);
        }
        
        if (i < count - 1) {
            strcat(result, delim);
        }
    }
    
    return result;
}

/* Format string with variable arguments */
char* str_format(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    /* Get required length */
    va_list args_copy;
    va_copy(args_copy, args);
    int length = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    
    if (length < 0) {
        va_end(args);
        return NULL;
    }
    
    /* Allocate and format */
    char *str = malloc(length + 1);
    if (!str) {
        va_end(args);
        return NULL;
    }
    
    vsnprintf(str, length + 1, format, args);
    va_end(args);
    
    return str;
}

/* Escape special characters in string */
char* str_escape(const char *str, const char *special_chars, char escape_char) {
    if (!str) return NULL;
    
    /* Count special characters */
    int special_count = 0;
    for (const char *p = str; *p; p++) {
        if (strchr(special_chars, *p)) {
            special_count++;
        }
    }
    
    /* Allocate escaped string */
    size_t len = strlen(str);
    char *escaped = malloc(len + special_count + 1);
    if (!escaped) return NULL;
    
    /* Build escaped string */
    char *dst = escaped;
    for (const char *src = str; *src; src++) {
        if (strchr(special_chars, *src)) {
            *dst++ = escape_char;
        }
        *dst++ = *src;
    }
    
    *dst = '\0';
    return escaped;
}

/* Unescape string */
char* str_unescape(const char *str, char escape_char) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    char *unescaped = malloc(len + 1);
    if (!unescaped) return NULL;
    
    char *dst = unescaped;
    int escape_next = 0;
    
    for (const char *src = str; *src; src++) {
        if (!escape_next && *src == escape_char) {
            escape_next = 1;
            continue;
        }
        
        *dst++ = *src;
        escape_next = 0;
    }
    
    *dst = '\0';
    return unescaped;
}

/* Convert string to integer safely */
int str_to_int(const char *str, int *value) {
    if (!str || !value) return 0;
    
    char *endptr;
    long val = strtol(str, &endptr, 10);
    
    if (endptr == str || *endptr != '\0') {
        return 0;
    }
    
    /* Check for overflow */
    if (val > INT_MAX || val < INT_MIN) {
        return 0;
    }
    
    *value = (int)val;
    return 1;
}

/* Convert string to unsigned integer safely */
int str_to_uint(const char *str, unsigned int *value) {
    if (!str || !value) return 0;
    
    char *endptr;
    unsigned long val = strtoul(str, &endptr, 10);
    
    if (endptr == str || *endptr != '\0') {
        return 0;
    }
    
    /* Check for overflow */
    if (val > UINT_MAX) {
        return 0;
    }
    
    *value = (unsigned int)val;
    return 1;
}

/* Convert string to float safely */
int str_to_float(const char *str, float *value) {
    if (!str || !value) return 0;
    
    char *endptr;
    float val = strtof(str, &endptr);
    
    if (endptr == str || *endptr != '\0') {
        return 0;
    }
    
    *value = val;
    return 1;
}

/* Convert string to double safely */
int str_to_double(const char *str, double *value) {
    if (!str || !value) return 0;
    
    char *endptr;
    double val = strtod(str, &endptr);
    
    if (endptr == str || *endptr != '\0') {
        return 0;
    }
    
    *value = val;
    return 1;
}

/* Check if string contains only digits */
int str_is_digits(const char *str) {
    if (!str) return 0;
    
    for (; *str; str++) {
        if (!isdigit(*str)) {
            return 0;
        }
    }
    
    return 1;
}

/* Check if string contains only hexadecimal digits */
int str_is_hex(const char *str) {
    if (!str) return 0;
    
    for (; *str; str++) {
        if (!isxdigit(*str)) {
            return 0;
        }
    }
    
    return 1;
}

/* Check if string contains only alphanumeric characters */
int str_is_alnum(const char *str) {
    if (!str) return 0;
    
    for (; *str; str++) {
        if (!isalnum(*str)) {
            return 0;
        }
    }
    
    return 1;
}

/* Check if string contains only printable characters */
int str_is_printable(const char *str) {
    if (!str) return 0;
    
    for (; *str; str++) {
        if (!isprint(*str)) {
            return 0;
        }
    }
    
    return 1;
}

/* Compare strings case-insensitively */
int str_casecmp(const char *s1, const char *s2) {
#ifdef _WIN32
    return _stricmp(s1, s2);
#else
    return strcasecmp(s1, s2);
#endif
}

/* Compare strings case-insensitively with length limit */
int str_ncasecmp(const char *s1, const char *s2, size_t n) {
#ifdef _WIN32
    return _strnicmp(s1, s2, n);
#else
    return strncasecmp(s1, s2, n);
#endif
}

/* Duplicate string with length limit */
char* str_ndup(const char *str, size_t n) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    if (len > n) {
        len = n;
    }
    
    char *dup = malloc(len + 1);
    if (!dup) return NULL;
    
    memcpy(dup, str, len);
    dup[len] = '\0';
    
    return dup;
}

/* Reverse string */
char* str_reverse(char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    for (size_t i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
    
    return str;
}

/* Count occurrences of character in string */
int str_count_char(const char *str, char ch) {
    if (!str) return 0;
    
    int count = 0;
    for (; *str; str++) {
        if (*str == ch) {
            count++;
        }
    }
    
    return count;
}

/* Check if string is empty or whitespace only */
int str_is_empty(const char *str) {
    if (!str) return 1;
    
    for (; *str; str++) {
        if (!isspace(*str)) {
            return 0;
        }
    }
    
    return 1;
}