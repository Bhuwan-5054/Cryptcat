/*
 * Platform Abstraction Layer
 * Header file for platform-specific implementations
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Platform error codes */
typedef enum {
    PLATFORM_SUCCESS = 0,
    PLATFORM_ERROR_GENERIC = -1,
    PLATFORM_ERROR_NETWORK = -2,
    PLATFORM_ERROR_IO = -3,
    PLATFORM_ERROR_MEMORY = -4,
    PLATFORM_ERROR_CRYPTO = -5,
    PLATFORM_ERROR_NOT_FOUND = -6,
    PLATFORM_ERROR_ACCESS = -7,
    PLATFORM_ERROR_TIMEOUT = -8
} platform_error_t;

/* Thread and mutex types */
typedef void* platform_thread_t;
typedef void* platform_mutex_t;
typedef void* (*platform_thread_func)(void*);

/* System information structure */
typedef struct {
    char platform_name[32];
    char architecture[32];
    uint64_t total_memory;
    uint64_t free_memory;
    int num_cpus;
    uint64_t uptime;
} platform_system_info_t;

/* ========== Network Functions ========== */

/**
 * Initialize platform networking subsystem.
 * 
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_network_init(void);

/**
 * Create a socket.
 * 
 * @param domain Socket domain (AF_INET, AF_INET6)
 * @param type Socket type (SOCK_STREAM, SOCK_DGRAM)
 * @param protocol Protocol (0 for default)
 * @return Socket file descriptor, or -1 on error
 */
int platform_create_socket(int domain, int type, int protocol);

/**
 * Close a socket.
 * 
 * @param sockfd Socket file descriptor
 */
void platform_close_socket(int sockfd);

/**
 * Set socket to non-blocking mode.
 * 
 * @param sockfd Socket file descriptor
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_set_nonblocking(int sockfd);

/* ========== Cryptography Functions ========== */

/**
 * Generate cryptographically secure random bytes.
 * 
 * @param buffer Output buffer
 * @param length Number of bytes to generate
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_random_bytes(unsigned char *buffer, size_t length);

/**
 * Seed the random number generator.
 * 
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_random_seed(void);

/* ========== Time Functions ========== */

/**
 * Sleep for specified milliseconds.
 * 
 * @param milliseconds Time to sleep
 */
void platform_sleep_ms(int milliseconds);

/**
 * Get current time in milliseconds.
 * 
 * @return Current time in milliseconds
 */
uint64_t platform_get_time_ms(void);

/* ========== Network Utilities ========== */

/**
 * Get local IP address.
 * 
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_get_local_ip(char *buffer, size_t buffer_size);

/* ========== File System Functions ========== */

/**
 * Create a directory.
 * 
 * @param path Directory path
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_create_directory(const char *path);

/**
 * Check if file exists.
 * 
 * @param path File path
 * @return 1 if exists, 0 otherwise
 */
int platform_file_exists(const char *path);

/**
 * Get file size.
 * 
 * @param path File path
 * @return File size in bytes, or -1 on error
 */
int64_t platform_get_file_size(const char *path);

/* ========== Threading Functions ========== */

/**
 * Create a new thread.
 * 
 * @param func Thread function
 * @param arg Thread argument
 * @return Thread handle
 */
platform_thread_t platform_thread_create(platform_thread_func func, void *arg);

/**
 * Wait for thread to complete.
 * 
 * @param thread Thread handle
 */
void platform_thread_join(platform_thread_t thread);

/**
 * Detach a thread (don't wait for completion).
 * 
 * @param thread Thread handle
 */
void platform_thread_detach(platform_thread_t thread);

/**
 * Create a mutex.
 * 
 * @return Mutex handle, or NULL on error
 */
platform_mutex_t platform_mutex_create(void);

/**
 * Lock a mutex.
 * 
 * @param mutex Mutex handle
 */
void platform_mutex_lock(platform_mutex_t mutex);

/**
 * Unlock a mutex.
 * 
 * @param mutex Mutex handle
 */
void platform_mutex_unlock(platform_mutex_t mutex);

/**
 * Destroy a mutex.
 * 
 * @param mutex Mutex handle
 */
void platform_mutex_destroy(platform_mutex_t mutex);

/* ========== System Information ========== */

/**
 * Get platform name.
 * 
 * @return Platform name string
 */
const char* platform_get_name(void);

/**
 * Get platform architecture.
 * 
 * @return Architecture string
 */
const char* platform_get_architecture(void);

/**
 * Get system information.
 * 
 * @return System information structure
 */
platform_system_info_t platform_get_system_info(void);

/**
 * Get current working directory.
 * 
 * @param buffer Output buffer
 * @param size Buffer size
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_get_cwd(char *buffer, size_t size);

/**
 * Set current working directory.
 * 
 * @param path Directory path
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_set_cwd(const char *path);

/**
 * Get environment variable.
 * 
 * @param name Variable name
 * @return Variable value, or NULL if not found
 */
const char* platform_get_env(const char *name);

/**
 * Set environment variable.
 * 
 * @param name Variable name
 * @param value Variable value
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_set_env(const char *name, const char *value);

/**
 * Get temporary directory.
 * 
 * @param buffer Output buffer
 * @param size Buffer size
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_get_temp_dir(char *buffer, size_t size);

/**
 * Create temporary file.
 * 
 * @param template Template string (may be modified)
 * @return FILE pointer, or NULL on error
 */
FILE* platform_create_temp_file(char *template);

/**
 * Execute system command.
 * 
 * @param command Command to execute
 * @param output Output buffer
 * @param output_size Output buffer size
 * @return PLATFORM_SUCCESS on success, error code on failure
 */
int platform_execute_command(const char *command, char *output, size_t output_size);

/**
 * Check if running with elevated privileges.
 * 
 * @return 1 if elevated (admin/root), 0 otherwise
 */
int platform_is_elevated(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H */