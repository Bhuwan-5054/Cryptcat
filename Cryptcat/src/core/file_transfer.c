/*
 * Cryptcat File Transfer Module
 * Secure encrypted file transfer implementation
 * Version: 1.0.0
 * License: MIT
 */

#include "file_transfer.h"
#include "protocol.h"
#include "crypto.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <openssl/sha.h>

/* File transfer constants */
#define DEFAULT_CHUNK_SIZE 16384     /* 16KB chunks */
#define MAX_CHUNK_SIZE 65536         /* 64KB max chunk size */
#define MAX_FILENAME_LEN 512
#define TRANSFER_TIMEOUT 30          /* seconds */
#define MAX_RETRIES 5

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

/* File transfer context */
typedef struct {
    transfer_state_t state;
    char filename[MAX_FILENAME_LEN];
    FILE *file;
    uint64_t file_size;
    uint64_t bytes_transferred;
    uint32_t chunks_sent;
    uint32_t chunks_received;
    time_t start_time;
    time_t last_activity;
    unsigned char checksum[SHA256_DIGEST_LENGTH];
    connection_t *conn;
    void *user_data;
} file_transfer_t;

/* Internal function prototypes */
static int calculate_file_checksum(const char *filename, unsigned char *checksum);
static int send_file_chunk_internal(file_transfer_t *transfer);
static int receive_file_chunk_internal(file_transfer_t *transfer, 
                                      const unsigned char *chunk_data,
                                      size_t chunk_size, uint32_t chunk_num);
static void update_transfer_progress(file_transfer_t *transfer);

/* Initialize file transfer module */
int file_transfer_init(void) {
    LOG_INFO("File transfer module initialized");
    return FILE_TRANSFER_SUCCESS;
}

/* Start sending a file */
file_transfer_t* start_file_send(connection_t *conn, const char *filename) {
    file_transfer_t *transfer = NULL;
    struct stat file_stat;
    
    if (!conn || !filename) {
        LOG_ERROR("Invalid parameters for file send");
        return NULL;
    }
    
    if (conn->state != STATE_READY) {
        LOG_ERROR("Connection not ready for file transfer");
        return NULL;
    }
    
    /* Check if file exists and is readable */
    if (stat(filename, &file_stat) < 0) {
        LOG_ERROR("Cannot access file '%s': %s", filename, strerror(errno));
        return NULL;
    }
    
    if (!S_ISREG(file_stat.st_mode)) {
        LOG_ERROR("'%s' is not a regular file", filename);
        return NULL;
    }
    
    if (file_stat.st_size == 0) {
        LOG_ERROR("File '%s' is empty", filename);
        return NULL;
    }
    
    /* Open file for reading */
    FILE *file = fopen(filename, "rb");
    if (!file) {
        LOG_ERROR("Failed to open file '%s': %s", filename, strerror(errno));
        return NULL;
    }
    
    /* Create transfer context */
    transfer = calloc(1, sizeof(file_transfer_t));
    if (!transfer) {
        LOG_ERROR("Memory allocation failed");
        fclose(file);
        return NULL;
    }
    
    transfer->state = TRANSFER_PREPARING;
    transfer->file = file;
    transfer->file_size = file_stat.st_size;
    transfer->bytes_transferred = 0;
    transfer->chunks_sent = 0;
    transfer->start_time = time(NULL);
    transfer->last_activity = transfer->start_time;
    transfer->conn = conn;
    
    strncpy(transfer->filename, filename, sizeof(transfer->filename) - 1);
    
    /* Calculate file checksum */
    if (calculate_file_checksum(filename, transfer->checksum) != 0) {
        LOG_ERROR("Failed to calculate checksum for '%s'", filename);
        free(transfer);
        fclose(file);
        return NULL;
    }
    
    /* Send file start message */
    unsigned char start_data[MAX_FILENAME_LEN + 20];
    size_t data_len = 0;
    
    /* Format: filename|filesize|checksum */
    int written = snprintf((char*)start_data, sizeof(start_data),
                          "%s|%lu|", filename, (unsigned long)transfer->file_size);
    
    /* Append checksum as hex string */
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        written += snprintf((char*)start_data + written, 
                           sizeof(start_data) - written,
                           "%02x", transfer->checksum[i]);
    }
    
    data_len = written;
    
    if (send_message(conn, MSG_FILE_START, start_data, data_len) 
        != PROTOCOL_SUCCESS) {
        LOG_ERROR("Failed to send file start message");
        free(transfer);
        fclose(file);
        return NULL;
    }
    
    transfer->state = TRANSFER_SENDING;
    LOG_INFO("Started sending file '%s' (%lu bytes)", filename, 
             (unsigned long)transfer->file_size);
    
    return transfer;
}

/* Start receiving a file */
file_transfer_t* start_file_receive(connection_t *conn, 
                                   const unsigned char *file_info, 
                                   size_t info_len) {
    file_transfer_t *transfer = NULL;
    char *filename = NULL;
    char *filesize_str = NULL;
    char *checksum_str = NULL;
    char *saveptr = NULL;
    
    if (!conn || !file_info || info_len == 0) {
        LOG_ERROR("Invalid parameters for file receive");
        return NULL;
    }
    
    /* Parse file information */
    char info_str[MAX_FILENAME_LEN + 100];
    if (info_len >= sizeof(info_str)) {
        LOG_ERROR("File info too large");
        return NULL;
    }
    
    memcpy(info_str, file_info, info_len);
    info_str[info_len] = '\0';
    
    /* Parse: filename|filesize|checksum */
    filename = strtok_r(info_str, "|", &saveptr);
    filesize_str = strtok_r(NULL, "|", &saveptr);
    checksum_str = strtok_r(NULL, "|", &saveptr);
    
    if (!filename || !filesize_str || !checksum_str) {
        LOG_ERROR("Invalid file info format");
        return NULL;
    }
    
    /* Parse file size */
    uint64_t file_size = strtoull(filesize_str, NULL, 10);
    if (file_size == 0) {
        LOG_ERROR("Invalid file size: %s", filesize_str);
        return NULL;
    }
    
    /* Parse checksum */
    unsigned char expected_checksum[SHA256_DIGEST_LENGTH];
    if (strlen(checksum_str) != SHA256_DIGEST_LENGTH * 2) {
        LOG_ERROR("Invalid checksum length");
        return NULL;
    }
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        char hex[3] = {checksum_str[i * 2], checksum_str[i * 2 + 1], '\0'};
        expected_checksum[i] = strtol(hex, NULL, 16);
    }
    
    /* Create output filename (add .part extension during transfer) */
    char output_filename[MAX_FILENAME_LEN + 10];
    snprintf(output_filename, sizeof(output_filename), "%s.part", filename);
    
    /* Open file for writing */
    FILE *file = fopen(output_filename, "wb");
    if (!file) {
        LOG_ERROR("Failed to create file '%s': %s", output_filename, 
                 strerror(errno));
        return NULL;
    }
    
    /* Create transfer context */
    transfer = calloc(1, sizeof(file_transfer_t));
    if (!transfer) {
        LOG_ERROR("Memory allocation failed");
        fclose(file);
        return NULL;
    }
    
    transfer->state = TRANSFER_RECEIVING;
    transfer->file = file;
    transfer->file_size = file_size;
    transfer->bytes_transferred = 0;
    transfer->chunks_received = 0;
    transfer->start_time = time(NULL);
    transfer->last_activity = transfer->start_time;
    transfer->conn = conn;
    
    strncpy(transfer->filename, filename, sizeof(transfer->filename) - 1);
    memcpy(transfer->checksum, expected_checksum, SHA256_DIGEST_LENGTH);
    
    LOG_INFO("Started receiving file '%s' (%lu bytes)", filename, 
             (unsigned long)file_size);
    
    return transfer;
}

/* Process file transfer */
int process_file_transfer(file_transfer_t *transfer) {
    if (!transfer) {
        return FILE_TRANSFER_ERROR_PARAM;
    }
    
    /* Check for timeout */
    if (time(NULL) - transfer->last_activity > TRANSFER_TIMEOUT) {
        LOG_ERROR("File transfer timeout");
        transfer->state = TRANSFER_ERROR;
        return FILE_TRANSFER_ERROR_TIMEOUT;
    }
    
    /* Process based on state */
    switch (transfer->state) {
        case TRANSFER_SENDING:
            return send_file_chunk_internal(transfer);
            
        case TRANSFER_RECEIVING:
            /* Receiving is handled by receive_file_chunk() */
            return FILE_TRANSFER_IN_PROGRESS;
            
        case TRANSFER_COMPLETE:
            return FILE_TRANSFER_SUCCESS;
            
        case TRANSFER_ERROR:
            return FILE_TRANSFER_ERROR_IO;
            
        case TRANSFER_CANCELLED:
            return FILE_TRANSFER_CANCELLED;
            
        default:
            return FILE_TRANSFER_ERROR_STATE;
    }
}

/* Send a file chunk */
static int send_file_chunk_internal(file_transfer_t *transfer) {
    unsigned char chunk_data[MAX_CHUNK_SIZE];
    size_t bytes_read;
    uint32_t chunk_num;
    
    if (!transfer || !transfer->file) {
        return FILE_TRANSFER_ERROR_PARAM;
    }
    
    /* Check if transfer is complete */
    if (transfer->bytes_transferred >= transfer->file_size) {
        /* Send file end message */
        if (send_message(transfer->conn, MSG_FILE_END, transfer->checksum, 
                        SHA256_DIGEST_LENGTH) != PROTOCOL_SUCCESS) {
            LOG_ERROR("Failed to send file end message");
            transfer->state = TRANSFER_ERROR;
            return FILE_TRANSFER_ERROR_NETWORK;
        }
        
        fclose(transfer->file);
        transfer->file = NULL;
        transfer->state = TRANSFER_COMPLETE;
        
        LOG_INFO("File '%s' sent successfully (%lu bytes, %u chunks)",
                 transfer->filename, (unsigned long)transfer->file_size,
                 transfer->chunks_sent);
        
        return FILE_TRANSFER_SUCCESS;
    }
    
    /* Read next chunk */
    bytes_read = fread(chunk_data, 1, DEFAULT_CHUNK_SIZE, transfer->file);
    if (bytes_read == 0) {
        if (feof(transfer->file)) {
            /* End of file reached */
            return send_file_chunk_internal(transfer);
        } else {
            LOG_ERROR("Error reading from file: %s", strerror(errno));
            transfer->state = TRANSFER_ERROR;
            return FILE_TRANSFER_ERROR_IO;
        }
    }
    
    /* Send chunk */
    chunk_num = transfer->chunks_sent;
    if (send_file_chunk(transfer->conn, chunk_data, bytes_read, chunk_num) 
        != PROTOCOL_SUCCESS) {
        LOG_ERROR("Failed to send file chunk %u", chunk_num);
        transfer->state = TRANSFER_ERROR;
        return FILE_TRANSFER_ERROR_NETWORK;
    }
    
    /* Update transfer state */
    transfer->bytes_transferred += bytes_read;
    transfer->chunks_sent++;
    transfer->last_activity = time(NULL);
    
    /* Update progress display */
    update_transfer_progress(transfer);
    
    return FILE_TRANSFER_IN_PROGRESS;
}

/* Receive a file chunk */
int receive_file_chunk(file_transfer_t *transfer, const unsigned char *chunk_data,
                      size_t chunk_size, uint32_t chunk_num) {
    return receive_file_chunk_internal(transfer, chunk_data, chunk_size, chunk_num);
}

/* Internal: Receive file chunk */
static int receive_file_chunk_internal(file_transfer_t *transfer, 
                                      const unsigned char *chunk_data,
                                      size_t chunk_size, uint32_t chunk_num) {
    size_t bytes_written;
    
    if (!transfer || !transfer->file || !chunk_data) {
        return FILE_TRANSFER_ERROR_PARAM;
    }
    
    /* Check chunk number */
    if (chunk_num != transfer->chunks_received) {
        LOG_ERROR("Out-of-order chunk: expected %u, got %u",
                 transfer->chunks_received, chunk_num);
        return FILE_TRANSFER_ERROR_ORDER;
    }
    
    /* Check if we have space for this chunk */
    if (transfer->bytes_transferred + chunk_size > transfer->file_size) {
        LOG_ERROR("Chunk exceeds file size");
        transfer->state = TRANSFER_ERROR;
        return FILE_TRANSFER_ERROR_SIZE;
    }
    
    /* Write chunk to file */
    bytes_written = fwrite(chunk_data, 1, chunk_size, transfer->file);
    if (bytes_written != chunk_size) {
        LOG_ERROR("Error writing to file: %s", strerror(errno));
        transfer->state = TRANSFER_ERROR;
        return FILE_TRANSFER_ERROR_IO;
    }
    
    /* Update transfer state */
    transfer->bytes_transferred += bytes_written;
    transfer->chunks_received++;
    transfer->last_activity = time(NULL);
    
    /* Update progress display */
    update_transfer_progress(transfer);
    
    /* Check if transfer is complete */
    if (transfer->bytes_transferred >= transfer->file_size) {
        fclose(transfer->file);
        transfer->file = NULL;
        
        /* Verify checksum */
        unsigned char actual_checksum[SHA256_DIGEST_LENGTH];
        char final_filename[MAX_FILENAME_LEN];
        
        /* Rename .part file to final name */
        snprintf(final_filename, sizeof(final_filename), "%s.part", 
                 transfer->filename);
        
        if (calculate_file_checksum(final_filename, actual_checksum) != 0) {
            LOG_ERROR("Failed to calculate received file checksum");
            transfer->state = TRANSFER_ERROR;
            return FILE_TRANSFER_ERROR_VERIFY;
        }
        
        if (memcmp(actual_checksum, transfer->checksum, SHA256_DIGEST_LENGTH) != 0) {
            LOG_ERROR("Checksum mismatch for received file");
            transfer->state = TRANSFER_ERROR;
            return FILE_TRANSFER_ERROR_VERIFY;
        }
        
        /* Rename to final filename */
        if (rename(final_filename, transfer->filename) != 0) {
            LOG_ERROR("Failed to rename file: %s", strerror(errno));
            transfer->state = TRANSFER_ERROR;
            return FILE_TRANSFER_ERROR_IO;
        }
        
        transfer->state = TRANSFER_COMPLETE;
        
        LOG_INFO("File '%s' received successfully (%lu bytes, %u chunks)",
                 transfer->filename, (unsigned long)transfer->file_size,
                 transfer->chunks_received);
        
        return FILE_TRANSFER_SUCCESS;
    }
    
    return FILE_TRANSFER_IN_PROGRESS;
}

/* Cancel file transfer */
int cancel_file_transfer(file_transfer_t *transfer) {
    if (!transfer) {
        return FILE_TRANSFER_ERROR_PARAM;
    }
    
    LOG_INFO("Cancelling file transfer: '%s'", transfer->filename);
    
    /* Close file if open */
    if (transfer->file) {
        fclose(transfer->file);
        transfer->file = NULL;
    }
    
    /* Clean up partial file for receiving transfers */
    if (transfer->state == TRANSFER_RECEIVING) {
        char part_filename[MAX_FILENAME_LEN + 10];
        snprintf(part_filename, sizeof(part_filename), "%s.part", 
                 transfer->filename);
        remove(part_filename);
    }
    
    transfer->state = TRANSFER_CANCELLED;
    
    return FILE_TRANSFER_SUCCESS;
}

/* Get file transfer information */
file_transfer_info_t get_file_transfer_info(file_transfer_t *transfer) {
    file_transfer_info_t info = {0};
    
    if (transfer) {
        info.state = transfer->state;
        info.bytes_transferred = transfer->bytes_transferred;
        info.file_size = transfer->file_size;
        info.chunks_sent = transfer->chunks_sent;
        info.chunks_received = transfer->chunks_received;
        info.elapsed_time = time(NULL) - transfer->start_time;
        info.transfer_rate = 0;
        
        if (info.elapsed_time > 0) {
            info.transfer_rate = transfer->bytes_transferred / info.elapsed_time;
        }
        
        if (transfer->file_size > 0) {
            info.progress_percent = (transfer->bytes_transferred * 100) / 
                                   transfer->file_size;
        }
        
        strncpy(info.filename, transfer->filename, sizeof(info.filename) - 1);
    }
    
    return info;
}

/* Calculate file checksum (SHA256) */
static int calculate_file_checksum(const char *filename, unsigned char *checksum) {
    FILE *file = fopen(filename, "rb");
    unsigned char buffer[8192];
    SHA256_CTX sha256;
    size_t bytes_read;
    
    if (!file || !checksum) {
        return -1;
    }
    
    SHA256_Init(&sha256);
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        SHA256_Update(&sha256, buffer, bytes_read);
    }
    
    SHA256_Final(checksum, &sha256);
    fclose(file);
    
    return 0;
}

/* Update transfer progress display */
static void update_transfer_progress(file_transfer_t *transfer) {
    static time_t last_display = 0;
    time_t now = time(NULL);
    
    /* Update display every second */
    if (now - last_display >= 1) {
        float percent = 0;
        if (transfer->file_size > 0) {
            percent = (transfer->bytes_transferred * 100.0f) / transfer->file_size;
        }
        
        float rate = 0;
        time_t elapsed = now - transfer->start_time;
        if (elapsed > 0) {
            rate = transfer->bytes_transferred / (float)elapsed / 1024.0f; /* KB/s */
        }
        
        LOG_INFO("Progress: %.1f%% (%lu/%lu bytes) @ %.1f KB/s",
                 percent, (unsigned long)transfer->bytes_transferred,
                 (unsigned long)transfer->file_size, rate);
        
        last_display = now;
    }
}

/* Clean up file transfer resources */
void cleanup_file_transfer(file_transfer_t *transfer) {
    if (!transfer) return;
    
    /* Close file if still open */
    if (transfer->file) {
        fclose(transfer->file);
        transfer->file = NULL;
    }
    
    /* Free memory */
    free(transfer);
}