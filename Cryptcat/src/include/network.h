

#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct connection_s connection_t;
typedef struct crypto_session_s crypto_session_t;

/* Error codes */
typedef enum {
    NETWORK_SUCCESS = 0,
    NETWORK_ERROR_PARAM = -1,
    NETWORK_ERROR_INIT = -2,
    NETWORK_ERROR_RESOLVE = -3,
    NETWORK_ERROR_CONNECT = -4,
    NETWORK_ERROR_BIND = -5,
    NETWORK_ERROR_LISTEN = -6,
    NETWORK_ERROR_ACCEPT = -7,
    NETWORK_ERROR_IO = -8,
    NETWORK_ERROR_CLOSED = -9,
    NETWORK_ERROR_TIMEOUT = -10,
    NETWORK_ERROR_BUFFER = -11,
    NETWORK_ERROR_CRYPTO = -12,
    NETWORK_ERROR_STATE = -13,
    NETWORK_ERROR_MEMORY = -14
} network_error_t;

/* Connection states */
typedef enum {
    STATE_DISCONNECTED = 0,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_AUTHENTICATING,
    STATE_READY,
    STATE_CLOSING,
    STATE_ERROR
} connection_state_t;

/* Connection information */
typedef struct {
    connection_state_t state;
    char remote_host[256];
    int remote_port;
    int is_encrypted;
    int is_listening;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint32_t packets_sent;
    uint32_t packets_received;
    time_t connection_time;
    time_t idle_time;
} connection_info_t;

/* ========== Core Network Functions ========== */

/**
 * Initialize network subsystem.
 * Must be called before any network operations.
 * 
 * @return NETWORK_SUCCESS on success, error code on failure
 */
int network_init(void);

/**
 * Create a listening socket on the specified port.
 * 
 * @param port Port to listen on
 * @param password Optional encryption password
 * @return Pointer to listener connection, or NULL on failure
 */
connection_t* create_listener(int port, const char *password);

/**
 * Accept an incoming connection on a listening socket.
 * 
 * @param listener Listening connection
 * @return Pointer to new client connection, or NULL on failure
 */
connection_t* accept_connection(connection_t *listener);

/**
 * Connect to a remote host.
 * 
 * @param host Remote hostname or IP address
 * @param port Remote port
 * @param password Optional encryption password
 * @return Pointer to new connection, or NULL on failure
 */
connection_t* connect_to_host(const char *host, int port, const char *password);

/**
 * Send data through a connection.
 * 
 * @param conn Connection handle
 * @param data Data to send
 * @param len Length of data
 * @return Number of bytes sent, or error code on failure
 */
int send_data(connection_t *conn, const unsigned char *data, size_t len);

/**
 * Receive data from a connection.
 * 
 * @param conn Connection handle
 * @param buffer Buffer to store received data
 * @param max_len Maximum bytes to receive
 * @return Number of bytes received, or error code on failure
 */
int receive_data(connection_t *conn, unsigned char *buffer, size_t max_len);

/**
 * Close a connection and free all resources.
 * 
 * @param conn Connection to close
 */
void close_connection(connection_t *conn);

/**
 * Get information about a connection.
 * 
 * @param conn Connection handle
 * @return Connection information structure
 */
connection_info_t get_connection_info(connection_t *conn);

/* ========== Utility Functions ========== */

/**
 * Platform-specific socket close.
 * 
 * @param sockfd Socket file descriptor
 */
void close_socket(int sockfd);

/**
 * Platform-specific sleep function.
 * 
 * @param milliseconds Time to sleep in milliseconds
 */
void platform_sleep_ms(int milliseconds);

/**
 * Get human-readable error message for network error.
 * 
 * @param error_code Error code
 * @return Error message string
 */
const char* network_strerror(int error_code);

/**
 * Check if connection is ready for I/O operations.
 * 
 * @param conn Connection handle
 * @return 1 if ready, 0 otherwise
 */
int is_connection_ready(connection_t *conn);

/**
 * Set user data pointer for connection.
 * 
 * @param conn Connection handle
 * @param user_data User data pointer
 */
void set_connection_user_data(connection_t *conn, void *user_data);

/**
 * Get user data pointer from connection.
 * 
 * @param conn Connection handle
 * @return User data pointer
 */
void* get_connection_user_data(connection_t *conn);

/* ========== Advanced Network Functions ========== */

/**
 * Create a socket pair for inter-process communication.
 * 
 * @param sv Array of two socket file descriptors
 * @return NETWORK_SUCCESS on success, error code on failure
 */
int create_socket_pair(int sv[2]);

/**
 * Set socket to non-blocking mode.
 * 
 * @param sockfd Socket file descriptor
 * @return NETWORK_SUCCESS on success, error code on failure
 */
int set_nonblocking(int sockfd);

/**
 * Wait for socket to become ready for I/O.
 * 
 * @param sockfd Socket file descriptor
 * @param timeout_ms Timeout in milliseconds
 * @param wait_for_read Wait for read readiness
 * @param wait_for_write Wait for write readiness
 * @return 1 if ready, 0 if timeout, -1 on error
 */
int wait_for_socket(int sockfd, int timeout_ms, int wait_for_read, int wait_for_write);

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_H */