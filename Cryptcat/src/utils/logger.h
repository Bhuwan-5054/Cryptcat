#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Log levels */
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
} log_level_t;

/* Logger status */
typedef struct {
    int initialized;
    log_level_t level;
    char output[256];
    long file_size;
    int use_syslog;
} logger_status_t;

/* Initialize logger */
int logger_init(const char *filename, log_level_t level, int use_syslog);

/* Set log level */
void logger_set_level(log_level_t level);

/* Get current log level */
log_level_t logger_get_level(void);

/* Log a message */
void logger_log(log_level_t level, const char *file, int line,
               const char *function, const char *format, ...);

/* Log hex dump */
void logger_hex_dump(log_level_t level, const char *label,
                    const unsigned char *data, size_t len);

/* Rotate log file */
int logger_rotate(void);

/* Get log file size */
long logger_get_file_size(void);

/* Shutdown logger */
void logger_shutdown(void);

/* Get logger status */
logger_status_t logger_get_status(void);

/* Convenience macros */
#define LOG_DEBUG(...) logger_log(LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...) logger_log(LOG_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARNING(...) logger_log(LOG_WARNING, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...) logger_log(LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_CRITICAL(...) logger_log(LOG_CRITICAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */