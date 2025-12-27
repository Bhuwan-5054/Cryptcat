#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* String split result */
typedef struct {
    char **tokens;
    int count;
} str_split_t;

/* Safe string operations */
char* strncpy_safe(char *dest, const char *src, size_t dest_size);
char* strncat_safe(char *dest, const char *src, size_t dest_size);

/* Case conversion */
char* str_tolower(char *str);
char* str_toupper(char *str);

/* String trimming */
char* str_ltrim(char *str);
char* str_rtrim(char *str);
char* str_trim(char *str);

/* String checking */
int str_startswith(const char *str, const char *prefix);
int str_endswith(const char *str, const char *suffix);
int str_is_digits(const char *str);
int str_is_hex(const char *str);
int str_is_alnum(const char *str);
int str_is_printable(const char *str);
int str_is_empty(const char *str);

/* String manipulation */
char* str_replace(char *str, const char *old, const char *new);
str_split_t* str_split(const char *str, const char *delim);
void str_split_free(str_split_t *split);
char* str_join(const char **strings, int count, const char *delim);
char* str_format(const char *format, ...);
char* str_escape(const char *str, const char *special_chars, char escape_char);
char* str_unescape(const char *str, char escape_char);
char* str_reverse(char *str);

/* String conversion */
int str_to_int(const char *str, int *value);
int str_to_uint(const char *str, unsigned int *value);
int str_to_float(const char *str, float *value);
int str_to_double(const char *str, double *value);

/* String comparison */
int str_casecmp(const char *s1, const char *s2);
int str_ncasecmp(const char *s1, const char *s2, size_t n);

/* String duplication */
char* str_ndup(const char *str, size_t n);

/* Character counting */
int str_count_char(const char *str, char ch);

#ifdef __cplusplus
}
#endif

#endif /* STRING_UTILS_H */