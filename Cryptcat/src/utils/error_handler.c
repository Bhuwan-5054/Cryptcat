/*
 * Error Handler Implementation
 * Version: 1.0.0
 * License: MIT
 */

#include "error_handler.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Error callback function */
static error_callback_t error_callback = NULL;
static void *error_callback_data = NULL;

/* Last error information */
static error_info_t last_error = {0};

/* Initialize error handler */
int error_handler_init(void) {
    memset(&last_error, 0, sizeof(last_error));
    return ERROR_SUCCESS;
}

/* Set error callback */
void error_handler_set_callback(error_callback_t callback, void *data) {
    error_callback = callback;
    error_callback_data = data;
}

/* Handle an error */
void error_handler_handle(int error_code, const char *context,
                         const char *file, int line, const char *function) {
    const char *error_msg = error_handler_strerror(error_code);
    
    /* Store error information */
    last_error.code = error_code;
    last_error.timestamp = time(NULL);
    strncpy(last_error.context, context, sizeof(last_error.context) - 1);
    strncpy(last_error.file, file, sizeof(last_error.file) - 1);
    last_error.line = line;
    strncpy(last_error.function, function, sizeof(last_error.function) - 1);
    strncpy(last_error.message, error_msg, sizeof(last_error.message) - 1);
    
    /* Log the error */
    LOG_ERROR("[%s] %s (in %s:%d %s)", context, error_msg, file, line, function);
    
    /* Call error callback if set */
    if (error_callback) {
        error_callback(&last_error, error_callback_data);
    }
    
    /* For critical errors, we might want to take additional action */
    if (error_code >= ERROR_CRITICAL) {
        LOG_CRITICAL("Critical error detected: %s", error_msg);
        
        /* In production, you might want to save state or send alert */
        error_handler_save_state();
    }
}

/* Get last error */
error_info_t error_handler_get_last(void) {
    return last_error;
}

/* Clear last error */
void error_handler_clear(void) {
    memset(&last_error, 0, sizeof(last_error));
}

/* Get error string */
const char* error_handler_strerror(int error_code) {
    switch (error_code) {
        case ERROR_SUCCESS:
            return "Success";
        case ERROR_GENERIC:
            return "General error";
        case ERROR_MEMORY:
            return "Memory allocation failed";
        case ERROR_IO:
            return strerror(errno);
        case ERROR_NETWORK:
            return "Network error";
        case ERROR_CRYPTO:
            return "Cryptographic error";
        case ERROR_AUTH:
            return "Authentication failed";
        case ERROR_TIMEOUT:
            return "Operation timed out";
        case ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case ERROR_NOT_FOUND:
            return "Resource not found";
        case ERROR_ACCESS_DENIED:
            return "Access denied";
        case ERROR_OUT_OF_RANGE:
            return "Value out of range";
        case ERROR_NOT_IMPLEMENTED:
            return "Not implemented";
        case ERROR_SYSTEM:
            return "System error";
        case ERROR_CONFIG:
            return "Configuration error";
        case ERROR_PROTOCOL:
            return "Protocol violation";
        case ERROR_BUFFER_OVERFLOW:
            return "Buffer overflow";
        case ERROR_FILE_CORRUPT:
            return "File corrupt";
        case ERROR_DISK_FULL:
            return "Disk full";
        case ERROR_CONNECTION_LOST:
            return "Connection lost";
        case ERROR_SESSION_EXPIRED:
            return "Session expired";
        case ERROR_RATE_LIMIT:
            return "Rate limit exceeded";
        case ERROR_QUOTA_EXCEEDED:
            return "Quota exceeded";
        case ERROR_VERSION_MISMATCH:
            return "Version mismatch";
        case ERROR_FORMAT:
            return "Invalid format";
        case ERROR_CRC:
            return "CRC check failed";
        case ERROR_CHECKSUM:
            return "Checksum mismatch";
        case ERROR_SIGNATURE:
            return "Invalid signature";
        case ERROR_CERTIFICATE:
            return "Certificate error";
        case ERROR_KEY:
            return "Key error";
        case ERROR_CIPHER:
            return "Cipher error";
        case ERROR_DIGEST:
            return "Digest error";
        case ERROR_RANDOM:
            return "Random number generation failed";
        case ERROR_UNSUPPORTED:
            return "Unsupported operation";
        case ERROR_DEPRECATED:
            return "Deprecated feature";
        case ERROR_TEST_FAILED:
            return "Test failed";
        case ERROR_VALIDATION:
            return "Validation failed";
        case ERROR_PARSING:
            return "Parsing error";
        case ERROR_SERIALIZATION:
            return "Serialization error";
        case ERROR_DESERIALIZATION:
            return "Deserialization error";
        case ERROR_COMPRESSION:
            return "Compression error";
        case ERROR_DECOMPRESSION:
            return "Decompression error";
        case ERROR_ENCODING:
            return "Encoding error";
        case ERROR_DECODING:
            return "Decoding error";
        case ERROR_THREAD:
            return "Thread error";
        case ERROR_MUTEX:
            return "Mutex error";
        case ERROR_SEMAPHORE:
            return "Semaphore error";
        case ERROR_CONDITION:
            return "Condition variable error";
        case ERROR_DEADLOCK:
            return "Deadlock detected";
        case ERROR_STALE:
            return "Stale data";
        case ERROR_BUSY:
            return "Resource busy";
        case ERROR_INTERRUPTED:
            return "Operation interrupted";
        case ERROR_WOULD_BLOCK:
            return "Operation would block";
        case ERROR_AGAIN:
            return "Try again";
        case ERROR_IN_PROGRESS:
            return "Operation in progress";
        case ERROR_ALREADY:
            return "Already exists";
        case ERROR_NOT_DIR:
            return "Not a directory";
        case ERROR_IS_DIR:
            return "Is a directory";
        case ERROR_LOOP:
            return "Symbolic link loop";
        case ERROR_NAMETOOLONG:
            return "Name too long";
        case ERROR_NOENT:
            return "No such file or directory";
        case ERROR_NOEXEC:
            return "Exec format error";
        case ERROR_NOMEM:
            return "Out of memory";
        case ERROR_NOSPC:
            return "No space left on device";
        case ERROR_NOSYS:
            return "Function not implemented";
        case ERROR_NOTEMPTY:
            return "Directory not empty";
        case ERROR_NOTTY:
            return "Not a terminal";
        case ERROR_NXIO:
            return "No such device or address";
        case ERROR_PERM:
            return "Operation not permitted";
        case ERROR_PIPE:
            return "Broken pipe";
        case ERROR_ROFS:
            return "Read-only file system";
        case ERROR_SPIPE:
            return "Invalid seek";
        case ERROR_SRCH:
            return "No such process";
        case ERROR_TOOBIG:
            return "Argument list too long";
        case ERROR_XDEV:
            return "Cross-device link";
        case ERROR_CRITICAL:
            return "Critical error";
        default:
            return "Unknown error";
    }
}

/* Get system error string */
const char* error_handler_system_strerror(void) {
#ifdef _WIN32
    static char buffer[256];
    DWORD error = GetLastError();
    
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, error,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  buffer, sizeof(buffer), NULL);
    
    return buffer;
#else
    return strerror(errno);
#endif
}

/* Save error state for debugging */
static void error_handler_save_state(void) {
    /* In a real application, you might want to:
       1. Save core dump
       2. Write error log to secure location
       3. Send error report
       4. Save application state
    */
    
    char filename[256];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    strftime(filename, sizeof(filename), "cryptcat_error_%Y%m%d_%H%M%S.log", tm_info);
    
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "=== Cryptcat Error Report ===\n");
        fprintf(f, "Time: %s", ctime(&now));
        fprintf(f, "Error Code: %d\n", last_error.code);
        fprintf(f, "Error Message: %s\n", last_error.message);
        fprintf(f, "Context: %s\n", last_error.context);
        fprintf(f, "Location: %s:%d\n", last_error.file, last_error.line);
        fprintf(f, "Function: %s\n", last_error.function);
        
        /* Add system information */
#ifdef _WIN32
        fprintf(f, "Platform: Windows\n");
#else
        fprintf(f, "Platform: Unix/Linux\n");
#endif
        
        fclose(f);
        
        LOG_INFO("Error state saved to %s", filename);
    }
}

/* Check for fatal error */
int error_handler_is_fatal(int error_code) {
    return (error_code >= ERROR_CRITICAL);
}

/* Reset error handler */
void error_handler_reset(void) {
    error_handler_clear();
    error_callback = NULL;
    error_callback_data = NULL;
}

/* Format error information */
void error_handler_format(const error_info_t *error, char *buffer, size_t size) {
    if (!error || !buffer || size == 0) {
        return;
    }
    
    char timestamp[64];
    struct tm *tm_info = localtime(&error->timestamp);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    snprintf(buffer, size,
             "[%s] Error %d: %s\n"
             "  Context: %s\n"
             "  Location: %s:%d in %s\n",
             timestamp, error->code, error->message,
             error->context, error->file, error->line, error->function);
}