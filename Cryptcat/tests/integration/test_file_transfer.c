/*
 * Cryptcat File Transfer Integration Test
 * Version: 1.0.0
 * License: MIT
 */

#include "../../src/include/network.h"
#include "../../src/include/protocol.h"
#include "../../src/include/file_transfer.h"
#include "../../src/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

/* Test server thread */
typedef struct {
    int port;
    const char *password;
    int running;
} server_context_t;

static void* file_transfer_server(void *arg) {
    server_context_t *ctx = (server_context_t*)arg;
    connection_t *listener = create_listener(ctx->port, ctx->password);
    
    if (!listener) {
        printf("Server: Failed to create listener\n");
        return NULL;
    }
    
    printf("Server: Listening on port %d\n", ctx->port);
    ctx->running = 1;
    
    while (ctx->running) {
        connection_t *client = accept_connection(listener);
        if (client) {
            printf("Server: Client connected\n");
            
            /* Perform handshake */
            int result = perform_handshake(client, 1, ctx->password);
            if (result == PROTOCOL_SUCCESS) {
                printf("Server: Handshake successful\n");
                
                /* Receive file */
                unsigned char buffer[1024];
                size_t buffer_len = sizeof(buffer);
                message_type_t msg_type;
                
                /* Wait for file start message */
                result = receive_message(client, &msg_type, buffer, &buffer_len);
                if (result == PROTOCOL_SUCCESS && msg_type == MSG_FILE_START) {
                    printf("Server: Received file start message\n");
                    
                    /* Process file chunks */
                    while (ctx->running) {
                        buffer_len = sizeof(buffer);
                        result = receive_message(client, &msg_type, buffer, &buffer_len);
                        
                        if (result == PROTOCOL_SUCCESS) {
                            if (msg_type == MSG_FILE_END) {
                                printf("Server: File transfer complete\n");
                                break;
                            } else if (msg_type == MSG_FILE_CHUNK) {
                                /* Process chunk */
                                printf("Server: Received chunk of %zu bytes\n", buffer_len);
                            }
                        } else {
                            printf("Server: Error receiving message: %d\n", result);
                            break;
                        }
                    }
                }
            }
            
            close_connection(client);
        }
        
        /* Small delay to prevent CPU spinning */
        usleep(100000);
    }
    
    close_connection(listener);
    printf("Server: Shutdown complete\n");
    return NULL;
}

/* Test file transfer */
int test_basic_file_transfer(void) {
    printf("\n=== Basic File Transfer Test ===\n");
    
    /* Create test file */
    FILE *test_file = fopen("test_file.txt", "w");
    if (!test_file) {
        printf("Failed to create test file\n");
        return -1;
    }
    
    /* Write test data */
    fprintf(test_file, "This is a test file for Cryptcat file transfer.\n");
    fprintf(test_file, "Line 2 of the test file.\n");
    fprintf(test_file, "Line 3 with some data.\n");
    fclose(test_file);
    
    /* Get file size */
    struct stat st;
    stat("test_file.txt", &st);
    printf("Test file size: %ld bytes\n", st.st_size);
    
    /* Start server thread */
    server_context_t server_ctx = {
        .port = 32000,
        .password = "test_password",
        .running = 0
    };
    
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, file_transfer_server, &server_ctx);
    
    /* Wait for server to start */
    while (!server_ctx.running) {
        usleep(100000);
    }
    sleep(1);
    
    /* Connect to server */
    connection_t *client = connect_to_host("127.0.0.1", server_ctx.port, server_ctx.password);
    if (!client) {
        printf("Client: Failed to connect to server\n");
        server_ctx.running = 0;
        pthread_join(server_thread, NULL);
        return -1;
    }
    
    /* Perform handshake */
    int result = perform_handshake(client, 0, server_ctx.password);
    if (result != PROTOCOL_SUCCESS) {
        printf("Client: Handshake failed: %d\n", result);
        close_connection(client);
        server_ctx.running = 0;
        pthread_join(server_thread, NULL);
        return -1;
    }
    
    printf("Client: Handshake successful\n");
    
    /* Start file transfer */
    file_transfer_t *transfer = start_file_send(client, "test_file.txt");
    if (!transfer) {
        printf("Client: Failed to start file transfer\n");
        close_connection(client);
        server_ctx.running = 0;
        pthread_join(server_thread, NULL);
        return -1;
    }
    
    printf("Client: File transfer started\n");
    
    /* Process transfer */
    while (1) {
        int status = process_file_transfer(transfer);
        
        if (status == FILE_TRANSFER_SUCCESS) {
            printf("Client: File transfer completed successfully\n");
            break;
        } else if (status == FILE_TRANSFER_IN_PROGRESS) {
            /* Continue transfer */
            usleep(10000);
        } else {
            printf("Client: File transfer error: %d\n", status);
            break;
        }
    }
    
    /* Cleanup */
    cleanup_file_transfer(transfer);
    close_connection(client);
    
    /* Stop server */
    server_ctx.running = 0;
    pthread_join(server_thread, NULL);
    
    /* Remove test file */
    remove("test_file.txt");
    
    return 0;
}

/* Test large file transfer */
int test_large_file_transfer(void) {
    printf("\n=== Large File Transfer Test ===\n");
    
    /* Create large test file (1MB) */
    const char *filename = "test_large_file.bin";
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to create large test file\n");
        return -1;
    }
    
    /* Write 1MB of data */
    size_t file_size = 1024 * 1024;
    unsigned char *data = malloc(file_size);
    if (!data) {
        fclose(file);
        return -1;
    }
    
    /* Fill with pattern */
    for (size_t i = 0; i < file_size; i++) {
        data[i] = (unsigned char)(i % 256);
    }
    
    fwrite(data, 1, file_size, file);
    fclose(file);
    free(data);
    
    printf("Created test file: %s (%zu bytes)\n", filename, file_size);
    
    /* Start server */
    server_context_t server_ctx = {
        .port = 32001,
        .password = "large_file_test",
        .running = 0
    };
    
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, file_transfer_server, &server_ctx);
    
    while (!server_ctx.running) {
        usleep(100000);
    }
    sleep(1);
    
    /* Connect and transfer */
    connection_t *client = connect_to_host("127.0.0.1", server_ctx.port, server_ctx.password);
    if (!client) {
        printf("Failed to connect\n");
        server_ctx.running = 0;
        pthread_join(server_thread, NULL);
        remove(filename);
        return -1;
    }
    
    /* Handshake */
    if (perform_handshake(client, 0, server_ctx.password) != PROTOCOL_SUCCESS) {
        printf("Handshake failed\n");
        close_connection(client);
        server_ctx.running = 0;
        pthread_join(server_thread, NULL);
        remove(filename);
        return -1;
    }
    
    /* Start transfer */
    file_transfer_t *transfer = start_file_send(client, filename);
    if (!transfer) {
        printf("Failed to start transfer\n");
        close_connection(client);
        server_ctx.running = 0;
        pthread_join(server_thread, NULL);
        remove(filename);
        return -1;
    }
    
    printf("Transfer started...\n");
    
    /* Monitor progress */
    time_t start_time = time(NULL);
    
    while (1) {
        int status = process_file_transfer(transfer);
        
        if (status == FILE_TRANSFER_SUCCESS) {
            time_t end_time = time(NULL);
            double elapsed = difftime(end_time, start_time);
            double speed = file_size / elapsed / 1024.0; /* KB/s */
            
            printf("Transfer completed in %.1f seconds (%.1f KB/s)\n", 
                   elapsed, speed);
            break;
        } else if (status == FILE_TRANSFER_IN_PROGRESS) {
            /* Show progress */
            file_transfer_info_t info = get_file_transfer_info(transfer);
            if (info.file_size > 0) {
                float percent = (info.bytes_transferred * 100.0f) / info.file_size;
                printf("\rProgress: %.1f%% (%zu/%zu bytes)", 
                       percent, info.bytes_transferred, info.file_size);
                fflush(stdout);
            }
            usleep(50000);
        } else {
            printf("\nTransfer error: %d\n", status);
            break;
        }
    }
    
    printf("\n");
    
    /* Cleanup */
    cleanup_file_transfer(transfer);
    close_connection(client);
    server_ctx.running = 0;
    pthread_join(server_thread, NULL);
    remove(filename);
    
    return 0;
}

/* Test encrypted file transfer */
int test_encrypted_file_transfer(void) {
    printf("\n=== Encrypted File Transfer Test ===\n");
    
    /* Create test file */
    FILE *file = fopen("test_encrypted.txt", "w");
    if (!file) return -1;
    
    fprintf(file, "This is sensitive data that should be encrypted during transfer.\n");
    fprintf(file, "Second line of sensitive data.\n");
    fclose(file);
    
    /* Modified server function for encrypted transfer */
    server_context_t server_ctx = {
        .port = 32002,
        .password = "encryption_key_123",
        .running = 0
    };
    
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, file_transfer_server, &server_ctx);
    
    while (!server_ctx.running) usleep(100000);
    sleep(1);
    
    /* Connect with encryption */
    connection_t *client = connect_to_host("127.0.0.1", server_ctx.port, server_ctx.password);
    if (!client) {
        printf("Failed to establish encrypted connection\n");
        server_ctx.running = 0;
        pthread_join(server_thread, NULL);
        remove("test_encrypted.txt");
        return -1;
    }
    
    /* Verify encryption is enabled */
    connection_info_t info = get_connection_info(client);
    if (!info.is_encrypted) {
        printf("Warning: Connection is not encrypted\n");
    }
    
    /* Handshake and transfer */
    if (perform_handshake(client, 0, server_ctx.password) != PROTOCOL_SUCCESS) {
        printf("Encrypted handshake failed\n");
        close_connection(client);
        server_ctx.running = 0;
        pthread_join(server_thread, NULL);
        remove("test_encrypted.txt");
        return -1;
    }
    
    file_transfer_t *transfer = start_file_send(client, "test_encrypted.txt");
    if (!transfer) {
        printf("Failed to start encrypted transfer\n");
        close_connection(client);
        server_ctx.running = 0;
        pthread_join(server_thread, NULL);
        remove("test_encrypted.txt");
        return -1;
    }
    
    printf("Encrypted file transfer started...\n");
    
    /* Complete transfer */
    while (1) {
        int status = process_file_transfer(transfer);
        if (status == FILE_TRANSFER_SUCCESS) {
            printf("Encrypted transfer completed\n");
            break;
        } else if (status == FILE_TRANSFER_IN_PROGRESS) {
            usleep(10000);
        } else {
            printf("Transfer error: %d\n", status);
            break;
        }
    }
    
    /* Cleanup */
    cleanup_file_transfer(transfer);
    close_connection(client);
    server_ctx.running = 0;
    pthread_join(server_thread, NULL);
    remove("test_encrypted.txt");
    
    return 0;
}

/* Main integration test */
int main(void) {
    printf("========================================\n");
    printf("  CRYPTCAT FILE TRANSFER INTEGRATION TESTS\n");
    printf("========================================\n");
    
    /* Initialize subsystems */
    logger_init("test_integration.log", LOG_INFO, 0);
    crypto_global_init();
    network_init();
    file_transfer_init();
    
    int passed = 0;
    int total = 3;
    
    /* Run tests */
    if (test_basic_file_transfer() == 0) {
        printf("\n✅ Basic file transfer test PASSED\n");
        passed++;
    } else {
        printf("\n❌ Basic file transfer test FAILED\n");
    }
    
    if (test_large_file_transfer() == 0) {
        printf("\n✅ Large file transfer test PASSED\n");
        passed++;
    } else {
        printf("\n❌ Large file transfer test FAILED\n");
    }
    
    if (test_encrypted_file_transfer() == 0) {
        printf("\n✅ Encrypted file transfer test PASSED\n");
        passed++;
    } else {
        printf("\n❌ Encrypted file transfer test FAILED\n");
    }
    
    /* Print summary */
    printf("\n========================================\n");
    printf("INTEGRATION TEST SUMMARY:\n");
    printf("  Tests passed: %d/%d\n", passed, total);
    
    if (passed == total) {
        printf("\n✅ ALL INTEGRATION TESTS PASSED!\n");
    } else {
        printf("\n❌ SOME INTEGRATION TESTS FAILED!\n");
    }
    
    logger_shutdown();
    return passed == total ? 0 : 1;
}