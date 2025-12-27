/*
 * Cryptcat File Transfer API
 * Header file for file_transfer.c
 */

#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include "network.h"
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
typedef enum {
    FILE_TRANSFER_SUCCESS = 0,
    FILE_TRANSFER_IN_PROGRESS = 1,
    FILE_TRANSFER_ERROR_PARAM = -1,
    FILE_TRANSFER_ERROR_STATE = -2,
    FILE_TRANSFER_ERROR_IO = -3,
    FILE_TRANSFER_ERROR_NETWORK = -4,
    FILE_TRANSFER_ERROR_SIZE = -5,
    FILE_TRANSFER_ERROR_ORDER = -6,
    FILE_TRANSFER_ERROR_VERIFY = -7,
    FILE_TRANSFER_ERROR_TIMEOUT = -8,
    FILE_TRANSFER_CANCELLED = -9
} file_transfer_error_t;

/* Transfer states */
typedef enum {
    TRANSFER_IDLE = 0,
    TRANSFER_PREPARING,
    TRANSFER_SENDING,
    TRANSFER_RECEIVING,
    TRANSFER_COMPLETE,
    TRANSFER_ERROR,
    TRANSFER_CANCELLED
} transfer_state_t;

/* Forward declaration */
typedef struct file_transfer_s file_transfer_t;

/* Transfer information */
typedef struct {
    transfer_state_t state;
    char filename[512];
    uint64_t bytes_transferred;
    uint64_t file_size;
    uint32_t chunks_sent;
    uint32_t chunks_received;
    uint8_t progress_percent;
    time_t elapsed_time;
    uint32_t transfer_rate; /* bytes per second */
} file_transfer_info_t;

/* ========== File Transfer Functions ========== */

/**
 * Initialize file transfer module.
 * 
 * @return FILE_TRANSFER_SUCCESS on success, error code on failure
 */
int file_transfer_init(void);

/**
 * Start sending a file.
 * 
 * @param conn Connection handle
 * @param filename Path to file to send
 * @return File transfer handle, or NULL on failure
 */
file_transfer_t* start_file_send(connection_t *conn, const char *filename);

/**
 * Start receiving a file.
 * 
 * @param conn Connection handle
 * @param file_info File information from MSG_FILE_START
 * @param info_len Length of file info
 * @return File transfer handle, or NULL on failure
 */
file_transfer_t* start_file_receive(connection_t *conn,
                                   const unsigned char *file_info,
                                   size_t info_len);

/**
 * Process file transfer (non-blocking).
 * 
 * @param transfer File transfer handle
 * @return Transfer status code
 */
int process_file_transfer(file_transfer_t *transfer);

/**
 * Receive a file chunk.
 * 
 * @param transfer File transfer handle
 * @param chunk_data Chunk data
 * @param chunk_size Chunk size
 * @param chunk_num Chunk sequence number
 * @return Transfer status code
 */
int receive_file_chunk(file_transfer_t *transfer,
                      const unsigned char *chunk_data,
                      size_t chunk_size, uint32_t chunk_num);

/**
 * Cancel file transfer.
 * 
 * @param transfer File transfer handle
 * @return FILE_TRANSFER_SUCCESS on success, error code on failure
 */
int cancel_file_transfer(file_transfer_t *transfer);

/**
 * Get file transfer information.
 * 
 * @param transfer File transfer handle
 * @return Transfer information structure
 */
file_transfer_info_t get_file_transfer_info(file_transfer_t *transfer);

/**
 * Clean up file transfer resources.
 * 
 * @param transfer File transfer handle
 */
void cleanup_file_transfer(file_transfer_t *transfer);

/**
 * Get human-readable error message for file transfer error.
 * 
 * @param error_code Error code
 * @return Error message string
 */
const char* file_transfer_strerror(int error_code);

/* ========== Utility Functions ========== */

/**
 * Calculate file size.
 * 
 * @param filename Path to file
 * @return File size in bytes, or -1 on error
 */
int64_t get_file_size(const char *filename);

/**
 * Check if file exists and is readable.
 * 
 * @param filename Path to file
 * @return 1 if file exists and is readable, 0 otherwise
 */
int file_exists(const char *filename);

/**
 * Calculate file checksum (SHA256).
 * 
 * @param filename Path to file
 * @param checksum Output buffer (32 bytes)
 * @return 0 on success, -1 on error
 */
int calculate_file_checksum(const char *filename, unsigned char *checksum);

#ifdef __cplusplus
}
#endif

#endif /* FILE_TRANSFER_H */