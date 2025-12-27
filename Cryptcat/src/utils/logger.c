/*
 * Logger Implementation
 * Version: 1.0.0
 * License: MIT
 */

#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <syslog.h>
#include <unistd.h>
#endif

/* Logger configuration */
static log_level_t current_level = LOG_INFO;
static FILE *log_file = NULL;
static int use_syslog = 0;
static char log_filename[256] = {0};
static int initialized = 0;

/* Log level strings */
static const char* level_strings[] = {
    "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
};

/* Color codes for terminal output */
#ifdef _WIN32
static WORD level_colors[] = {
    8,   /* DEBUG: gray */
    7,   /* INFO: white */
    14,  /* WARNING: yellow */
    12,  /* ERROR: red */
    64   /* CRITICAL: red background */
};
#else
static const char* level_colors[] = {
    "\033[90m",  /* DEBUG: gray */
    "\033[0m",   /* INFO: normal */
    "\033[33m",  /* WARNING: yellow */
    "\033[31m",  /* ERROR: red */
    "\033[41m"   /* CRITICAL: red background */
};
#endif

/* Initialize logger */
int logger_init(const char *filename, log_level_t level, int use_sys) {
    if (initialized) {
        return LOGGER_SUCCESS;
    }
    
    current_level = level;
    use_syslog = use_sys;
    
    /* Open log file if specified */
    if (filename && filename[0] != '\0') {
        log_file = fopen(filename, "a");
        if (!log_file) {
            fprintf(stderr, "Failed to open log file: %s\n", filename);
            return LOGGER_ERROR_IO;
        }
        
        strncpy(log_filename, filename, sizeof(log_filename) - 1);
        
        /* Write header */
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
        
        fprintf(log_file, "\n=== Cryptcat Log Started at %s ===\n", timestamp);
        fflush(log_file);
    } else {
        log_file = stderr;
    }
    
    /* Initialize syslog on Unix */
#ifndef _WIN32
    if (use_syslog) {
        openlog("cryptcat", LOG_PID | LOG_CONS, LOG_USER);
    }
#endif
    
    initialized = 1;
    return LOGGER_SUCCESS;
}

/* Set log level */
void logger_set_level(log_level_t level) {
    current_level = level;
}

/* Get current log level */
log_level_t logger_get_level(void) {
    return current_level;
}

/* Log a message */
void logger_log(log_level_t level, const char *file, int line,
               const char *function, const char *format, ...) {
    if (!initialized || level < current_level) {
        return;
    }
    
    va_list args;
    char message[1024];
    char formatted[2048];
    
    /* Format timestamp */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    /* Get filename without path */
    const char *filename = strrchr(file, '/');
    if (!filename) filename = strrchr(file, '\\');
    filename = filename ? filename + 1 : file;
    
    /* Format the message */
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    /* Create formatted log line */
    int len = snprintf(formatted, sizeof(formatted),
                      "[%s] %-8s %s:%d %s - %s\n",
                      timestamp, level_strings[level],
                      filename, line, function, message);
    
    if (len < 0 || len >= (int)sizeof(formatted)) {
        formatted[sizeof(formatted) - 1] = '\n';
        formatted[sizeof(formatted) - 2] = '\0';
    }
    
    /* Output to file */
    if (log_file) {
        fprintf(log_file, "%s", formatted);
        fflush(log_file);
    }
    
    /* Output to syslog */
#ifndef _WIN32
    if (use_syslog) {
        int priority;
        switch (level) {
            case LOG_DEBUG: priority = LOG_DEBUG; break;
            case LOG_INFO: priority = LOG_INFO; break;
            case LOG_WARNING: priority = LOG_WARNING; break;
            case LOG_ERROR: priority = LOG_ERR; break;
            case LOG_CRITICAL: priority = LOG_CRIT; break;
            default: priority = LOG_INFO;
        }
        syslog(priority, "%s:%d %s - %s", filename, line, function, message);
    }
#endif
    
    /* Output to stderr with colors */
    if (level >= LOG_WARNING || current_level <= LOG_DEBUG) {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        WORD savedAttributes;
        
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        savedAttributes = consoleInfo.wAttributes;
        
        SetConsoleTextAttribute(hConsole, level_colors[level]);
        fprintf(stderr, "%s", formatted);
        SetConsoleTextAttribute(hConsole, savedAttributes);
#else
        if (isatty(fileno(stderr))) {
            fprintf(stderr, "%s%s\033[0m", level_colors[level], formatted);
        } else {
            fprintf(stderr, "%s", formatted);
        }
#endif
    }
}

/* Log hex dump */
void logger_hex_dump(log_level_t level, const char *label,
                    const unsigned char *data, size_t len) {
    if (!initialized || level < current_level) {
        return;
    }
    
    logger_log(level, __FILE__, __LINE__, __func__,
              "%s (%zu bytes):", label, len);
    
    for (size_t i = 0; i < len; i += 16) {
        char line[128];
        char *ptr = line;
        
        /* Offset */
        ptr += sprintf(ptr, "  %04zx: ", i);
        
        /* Hex bytes */
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
        
        /* ASCII representation */
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
        
        logger_log(level, __FILE__, __LINE__, __func__, "%s", line);
    }
}

/* Rotate log file */
int logger_rotate(void) {
    if (!log_file || log_file == stderr || log_filename[0] == '\0') {
        return LOGGER_SUCCESS;
    }
    
    fclose(log_file);
    
    /* Create backup filename with timestamp */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char backup[512];
    strftime(backup, sizeof(backup), "%Y%m%d_%H%M%S", tm_info);
    
    char new_filename[768];
    snprintf(new_filename, sizeof(new_filename), "%s.%s.bak",
             log_filename, backup);
    
    /* Rename current log file */
    if (rename(log_filename, new_filename) != 0) {
        /* If rename fails, try to copy */
        FILE *src = fopen(log_filename, "rb");
        FILE *dst = fopen(new_filename, "wb");
        
        if (src && dst) {
            char buffer[8192];
            size_t bytes;
            
            while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                fwrite(buffer, 1, bytes, dst);
            }
            
            fclose(src);
            fclose(dst);
            
            /* Truncate original file */
            log_file = fopen(log_filename, "w");
            if (log_file) {
                fclose(log_file);
            }
        } else {
            if (src) fclose(src);
            if (dst) fclose(dst);
        }
    }
    
    /* Reopen log file */
    log_file = fopen(log_filename, "a");
    if (!log_file) {
        log_file = stderr;
        return LOGGER_ERROR_IO;
    }
    
    /* Write rotation marker */
    time_t now2 = time(NULL);
    struct tm *tm_info2 = localtime(&now2);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info2);
    
    fprintf(log_file, "\n=== Log rotated at %s (previous: %s) ===\n",
            timestamp, new_filename);
    fflush(log_file);
    
    return LOGGER_SUCCESS;
}

/* Get log file size */
long logger_get_file_size(void) {
    if (!log_file || log_file == stderr || log_filename[0] == '\0') {
        return -1;
    }
    
    FILE *f = fopen(log_filename, "rb");
    if (!f) {
        return -1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    return size;
}

/* Shutdown logger */
void logger_shutdown(void) {
    if (!initialized) {
        return;
    }
    
    if (log_file && log_file != stderr) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
        
        fprintf(log_file, "\n=== Cryptcat Log Ended at %s ===\n\n", timestamp);
        fclose(log_file);
    }
    
#ifndef _WIN32
    if (use_syslog) {
        closelog();
    }
#endif
    
    initialized = 0;
    log_file = NULL;
    log_filename[0] = '\0';
    use_syslog = 0;
}

/* Get logger status */
logger_status_t logger_get_status(void) {
    logger_status_t status = {0};
    
    status.initialized = initialized;
    status.level = current_level;
    status.use_syslog = use_syslog;
    
    if (log_file == stderr) {
        strcpy(status.output, "stderr");
    } else if (log_filename[0] != '\0') {
        strncpy(status.output, log_filename, sizeof(status.output) - 1);
        status.file_size = logger_get_file_size();
    } else {
        strcpy(status.output, "none");
    }
    
    return status;
}