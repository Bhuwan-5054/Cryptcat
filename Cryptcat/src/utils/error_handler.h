#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
typedef enum {
    ERROR_SUCCESS = 0,
    ERROR_GENERIC = -1,
    ERROR_MEMORY = -2,
    ERROR_IO = -3,
    ERROR_NETWORK = -4,
    ERROR_CRYPTO = -5,
    ERROR_AUTH = -6,
    ERROR_TIMEOUT = -7,
    ERROR_INVALID_PARAM = -8,
    ERROR_NOT_FOUND = -9,
    ERROR_ACCESS_DENIED = -10,
    ERROR_OUT_OF_RANGE = -11,
    ERROR_NOT_IMPLEMENTED = -12,
    ERROR_SYSTEM = -13,
    ERROR_CONFIG = -14,
    ERROR_PROTOCOL = -15,
    ERROR_BUFFER_OVERFLOW = -16,
    ERROR_FILE_CORRUPT = -17,
    ERROR_DISK_FULL = -18,
    ERROR_CONNECTION_LOST = -19,
    ERROR_SESSION_EXPIRED = -20,
    ERROR_RATE_LIMIT = -21,
    ERROR_QUOTA_EXCEEDED = -22,
    ERROR_VERSION_MISMATCH = -23,
    ERROR_FORMAT = -24,
    ERROR_CRC = -25,
    ERROR_CHECKSUM = -26,
    ERROR_SIGNATURE = -27,
    ERROR_CERTIFICATE = -28,
    ERROR_KEY = -29,
    ERROR_CIPHER = -30,
    ERROR_DIGEST = -31,
    ERROR_RANDOM = -32,
    ERROR_UNSUPPORTED = -33,
    ERROR_DEPRECATED = -34,
    ERROR_TEST_FAILED = -35,
    ERROR_VALIDATION = -36,
    ERROR_PARSING = -37,
    ERROR_SERIALIZATION = -38,
    ERROR_DESERIALIZATION = -39,
    ERROR_COMPRESSION = -40,
    ERROR_DECOMPRESSION = -41,
    ERROR_ENCODING = -42,
    ERROR_DECODING = -43,
    ERROR_THREAD = -44,
    ERROR_MUTEX = -45,
    ERROR_SEMAPHORE = -46,
    ERROR_CONDITION = -47,
    ERROR_DEADLOCK = -48,
    ERROR_STALE = -49,
    ERROR_BUSY = -50,
    ERROR_INTERRUPTED = -51,
    ERROR_WOULD_BLOCK = -52,
    ERROR_AGAIN = -53,
    ERROR_IN_PROGRESS = -54,
    ERROR_ALREADY = -55,
    ERROR_NOT_DIR = -56,
    ERROR_IS_DIR = -57,
    ERROR_LOOP = -58,
    ERROR_NAMETOOLONG = -59,
    ERROR_NOENT = -60,
    ERROR_NOEXEC = -61,
    ERROR_NOMEM = -62,
    ERROR_NOSPC = -63,
    ERROR_NOSYS = -64,
    ERROR_NOTEMPTY = -65,
    ERROR_NOTTY = -66,
    ERROR_NXIO = -67,
    ERROR_PERM = -68,
    ERROR_PIPE = -69,
    ERROR_ROFS = -70,
    ERROR_SPIPE = -71,
    ERROR_SRCH = -72,
    ERROR_TOOBIG = -73,
    ERROR_XDEV = -74,
    ERROR_CRITICAL = -100
} error_code_t;

/* Error information */
typedef struct {
    error_code_t code;
    time_t timestamp;
    char context[256];
    char file[256];
    int line;
    char function[128];
    char message[512];
} error_info_t;

/* Error callback function */
typedef void (*error_callback_t)(const error_info_t *error, void *user_data);

/* Initialize error handler */
int error_handler_init(void);

/* Set error callback */
void error_handler_set_callback(error_callback_t callback, void *user_data);

/* Handle an error */
void error_handler_handle(int error_code, const char *context,
                         const char *file, int line, const char *function);

/* Get last error */
error_info_t error_handler_get_last(void);

/* Clear last error */
void error_handler_clear(void);

/* Get error string */
const char* error_handler_strerror(int error_code);

/* Get system error string */
const char* error_handler_system_strerror(void);

/* Check for fatal error */
int error_handler_is_fatal(int error_code);

/* Reset error handler */
void error_handler_reset(void);

/* Format error information */
void error_handler_format(const error_info_t *error, char *buffer, size_t size);

/* Convenience macro */
#define ERROR_HANDLE(code, context) \
    error_handler_handle(code, context, __FILE__, __LINE__, __func__)

#ifdef __cplusplus
}
#endif

#endif /* ERROR_HANDLER_H */