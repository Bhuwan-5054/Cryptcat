/*
 * Cryptcat Protocol API
 * Header file for protocol.c
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "network.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
typedef enum {
    PROTOCOL_SUCCESS = 0,
    PROTOCOL_ERROR_PARAM = -1,
    PROTOCOL_ERROR_STATE = -2,
    PROTOCOL_ERROR_NETWORK = -3,
    PROTOCOL_ERROR_MALFORMED = -4,
    PROTOCOL_ERROR_CORRUPT = -5,
    PROTOCOL_ERROR_VERSION = -6,
    PROTOCOL_ERROR_SIZE = -7,
    PROTOCOL_ERROR_BUFFER = -8,
    PROTOCOL_ERROR_CLOSED = -9,
    PROTOCOL_ERROR_PEER = -10,
    PROTOCOL_ERROR_AUTH = -11,
    PROTOCOL_ERROR_TIMEOUT = -12
} protocol_error_t;

/* Message types */
typedef enum {
    MSG_HANDSHAKE_INIT = 0x01,
    MSG_HANDSHAKE_RESPONSE = 0x02,
    MSG_HANDSHAKE_COMPLETE = 0x03,
    MSG_DATA = 0x10,
    MSG_FILE_START = 0x20,
    MSG_FILE_CHUNK = 0x21,
    MSG_FILE_END = 0x22,
    MSG_KEEPALIVE = 0x30,
    MSG_DISCONNECT = 0x40,
    MSG_ERROR = 0xFF
} message_type_t;

/* ========== Protocol Functions ========== */

/**
 * Perform protocol handshake.
 * 
 * @param conn Connection handle
 * @param is_server 1 if server, 0 if client
 * @param password Encryption password
 * @return PROTOCOL_SUCCESS on success, error code on failure
 */
int perform_handshake(connection_t *conn, int is_server, const char *password);

/**
 * Send a protocol message.
 * 
 * @param conn Connection handle
 * @param type Message type
 * @param payload Message payload (can be NULL)
 * @param payload_len Payload length
 * @return PROTOCOL_SUCCESS on success, error code on failure
 */
int send_message(connection_t *conn, message_type_t type,
                const unsigned char *payload, size_t payload_len);

/**
 * Receive a protocol message.
 * 
 * @param conn Connection handle
 * @param type Output: Message type
 * @param buffer Output buffer for payload
 * @param buffer_len Input: buffer size, Output: payload length
 * @return PROTOCOL_SUCCESS on success, error code on failure
 */
int receive_message(connection_t *conn, message_type_t *type,
                   unsigned char *buffer, size_t *buffer_len);

/**
 * Send a simple data message.
 * 
 * @param conn Connection handle
 * @param data Data to send
 * @param len Data length
 * @return PROTOCOL_SUCCESS on success, error code on failure
 */
int send_data_message(connection_t *conn, const unsigned char *data, size_t len);

/**
 * Send file transfer start message.
 * 
 * @param conn Connection handle
 * @param filename File name
 * @param file_size File size in bytes
 * @return PROTOCOL_SUCCESS on success, error code on failure
 */
int send_file_start(connection_t *conn, const char *filename, uint64_t file_size);

/**
 * Send file chunk message.
 * 
 * @param conn Connection handle
 * @param chunk_data File chunk data
 * @param chunk_size Chunk size
 * @param chunk_number Chunk sequence number
 * @return PROTOCOL_SUCCESS on success, error code on failure
 */
int send_file_chunk(connection_t *conn, const unsigned char *chunk_data,
                   size_t chunk_size, uint32_t chunk_number);

/**
 * Send file transfer complete message.
 * 
 * @param conn Connection handle
 * @param checksum File checksum (SHA256)
 * @return PROTOCOL_SUCCESS on success, error code on failure
 */
int send_file_end(connection_t *conn, const unsigned char *checksum);

/**
 * Send keepalive message.
 * 
 * @param conn Connection handle
 * @return PROTOCOL_SUCCESS on success, error code on failure
 */
int send_keepalive(connection_t *conn);

/**
 * Send disconnect message.
 * 
 * @param conn Connection handle
 * @param reason Disconnect reason
 * @return PROTOCOL_SUCCESS on success, error code on failure
 */
int send_disconnect(connection_t *conn, const char *reason);

/**
 * Get human-readable error message for protocol error.
 * 
 * @param error_code Error code
 * @return Error message string
 */
const char* protocol_strerror(int error_code);

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_H */