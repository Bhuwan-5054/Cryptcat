/*
 * Cryptcat Network Unit Tests
 * Version: 1.0.0
 * License: MIT
 */

#include "../../src/include/network.h"
#include "../../src/include/crypto.h"
#include "../../src/utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

/* Test counters */
static int tests_passed = 0;
static int tests_failed = 0;

/* Test utilities */
#define TEST(expr) \
    do { \
        if (expr) { \
            printf("✓ %s\n", #expr); \
            tests_passed++; \
        } else { \
            printf("✗ %s\n", #expr); \
            tests_failed++; \
        } \
    } while(0)

#define TEST_MSG(expr, msg) \
    do { \
        if (expr) { \
            printf("✓ %s\n", msg); \
            tests_passed++; \
        } else { \
            printf("✗ %s\n", msg); \
            tests_failed++; \
        } \
    } while(0)

/* Global variables */
static connection_t *listener = NULL;
static connection_t *client = NULL;
static connection_t *server = NULL;
static int test_port = 0;

/* Find available port */
static int find_available_port(void) {
    /* Start from a high port number */
    return 30000 + (rand() % 10000);
}

/* Setup before tests */
static void setup(void) {
    /* Initialize subsystems */
    crypto_global_init();
    network_init();
    
    /* Find available port */
    test_port = find_available_port();
    printf("Using port %d for tests\n", test_port);
}

/* Cleanup after tests */
static void cleanup(void) {
    if (client) {
        close_connection(client);
        client = NULL;
    }
    if (server) {
        close_connection(server);
        server = NULL;
    }
    if (listener) {
        close_connection(listener);
        listener = NULL;
    }
}

/* Test 1: Network initialization */
void test_network_init(void) {
    printf("\n=== Test 1: Network Initialization ===\n");
    
    int result = network_init();
    TEST(result == NETWORK_SUCCESS);
    
    /* Test error string functions */
    TEST(strcmp(network_strerror(NETWORK_SUCCESS), "Success") == 0);
    TEST(strcmp(network_strerror(NETWORK_ERROR_PARAM), "Invalid parameter") == 0);
}

/* Test 2: Listener creation */
void test_create_listener(void) {
    printf("\n=== Test 2: Listener Creation ===\n");
    
    /* Create listener without encryption */
    listener = create_listener(test_port, NULL);
    TEST(listener != NULL);
    TEST(listener->is_listening == 1);
    TEST(listener->remote_port == test_port);
    
    /* Get connection info */
    connection_info_t info = get_connection_info(listener);
    TEST(info.is_listening == 1);
    TEST(info.remote_port == test_port);
    
    /* Try invalid port */
    connection_t *invalid = create_listener(99999, NULL);
    TEST(invalid == NULL);
    
    invalid = create_listener(-1, NULL);
    TEST(invalid == NULL);
}

/* Test 3: Local connection (client/server) */
void test_local_connection(void) {
    printf("\n=== Test 3: Local Connection ===\n");
    
    /* Create listener if not already created */
    if (!listener) {
        listener = create_listener(test_port, NULL);
        TEST(listener != NULL);
    }
    
    /* Connect client to server (in separate thread or process in real test) */
    /* For unit test, we'll simulate with localhost */
    client = connect_to_host("127.0.0.1", test_port, NULL);
    TEST(client != NULL);
    TEST(client->state == STATE_CONNECTED);
    
    /* Accept connection on server side */
    server = accept_connection(listener);
    TEST(server != NULL);
    TEST(server->state == STATE_CONNECTED);
    
    /* Verify connection info */
    connection_info_t client_info = get_connection_info(client);
    connection_info_t server_info = get_connection_info(server);
    
    TEST(client_info.state == STATE_CONNECTED);
    TEST(server_info.state == STATE_CONNECTED);
    TEST(strcmp(server_info.remote_host, "127.0.0.1") == 0);
}

/* Test 4: Data transmission */
void test_data_transmission(void) {
    printf("\n=== Test 4: Data Transmission ===\n");
    
    if (!client || !server) {
        printf("Skipping - need active connection\n");
        return;
    }
    
    /* Test data */
    const char *send_data = "Hello from client!";
    size_t data_len = strlen(send_data);
    
    /* Send data from client to server */
    int sent = send_data(client, (unsigned char*)send_data, data_len);
    TEST(sent == data_len);
    
    /* Receive on server */
    unsigned char recv_buffer[1024];
    int received = receive_data(server, recv_buffer, sizeof(recv_buffer));
    TEST(received == data_len);
    TEST(memcmp(send_data, recv_buffer, data_len) == 0);
    
    /* Send response from server to client */
    const char *response = "Hello from server!";
    sent = send_data(server, (unsigned char*)response, strlen(response));
    TEST(sent == strlen(response));
    
    /* Receive response on client */
    received = receive_data(client, recv_buffer, sizeof(recv_buffer));
    TEST(received == strlen(response));
    TEST(memcmp(response, recv_buffer, strlen(response)) == 0);
    
    /* Test with empty data */
    sent = send_data(client, NULL, 0);
    TEST(sent == NETWORK_ERROR_PARAM);
    
    sent = send_data(client, (unsigned char*)"test", 0);
    TEST(sent == NETWORK_ERROR_PARAM);
}

/* Test 5: Encrypted connection */
void test_encrypted_connection(void) {
    printf("\n=== Test 5: Encrypted Connection ===\n");
    
    /* Close existing connections */
    cleanup();
    
    /* Create encrypted listener */
    int encrypted_port = test_port + 1;
    listener = create_listener(encrypted_port, "test_password");
    TEST(listener != NULL);
    TEST(listener->is_encrypted == 1);
    
    /* Connect with encryption */
    client = connect_to_host("127.0.0.1", encrypted_port, "test_password");
    TEST(client != NULL);
    TEST(client->is_encrypted == 1);
    
    /* Accept encrypted connection */
    server = accept_connection(listener);
    TEST(server != NULL);
    TEST(server->is_encrypted == 1);
    
    /* Test encrypted data transfer */
    const char *secret = "This is secret data!";
    unsigned char buffer[1024];
    
    /* Send encrypted */
    int sent = send_data(client, (unsigned char*)secret, strlen(secret));
    TEST(sent > 0);
    
    /* Receive and decrypt */
    int received = receive_data(server, buffer, sizeof(buffer));
    TEST(received == strlen(secret));
    buffer[received] = '\0';
    TEST(strcmp(secret, (char*)buffer) == 0);
    
    /* Test with wrong password */
    cleanup();
    listener = create_listener(encrypted_port + 1, "password1");
    connection_t *wrong_client = connect_to_host("127.0.0.1", encrypted_port + 1, "password2");
    TEST(wrong_client == NULL); /* Should fail to establish encrypted connection */
}

/* Test 6: Connection states */
void test_connection_states(void) {
    printf("\n=== Test 6: Connection States ===\n");
    
    /* Create simple connection */
    cleanup();
    listener = create_listener(test_port + 2, NULL);
    TEST(listener != NULL);
    
    /* Check initial state */
    connection_info_t info = get_connection_info(listener);
    TEST(info.state == STATE_READY);
    
    /* Connect and check states */
    client = connect_to_host("127.0.0.1", test_port + 2, NULL);
    TEST(client != NULL);
    
    info = get_connection_info(client);
    TEST(info.state == STATE_CONNECTED);
    
    /* Close connection and check state */
    close_connection(client);
    client = NULL;
    
    /* Try operations on closed connection */
    unsigned char buffer[10];
    int result = send_data(client, buffer, sizeof(buffer));
    TEST(result == NETWORK_ERROR_STATE);
    
    result = receive_data(client, buffer, sizeof(buffer));
    TEST(result == NETWORK_ERROR_STATE);
}

/* Test 7: Large data transfer */
void test_large_data_transfer(void) {
    printf("\n=== Test 7: Large Data Transfer ===\n");
    
    cleanup();
    
    /* Create connection */
    int large_port = test_port + 3;
    listener = create_listener(large_port, NULL);
    client = connect_to_host("127.0.0.1", large_port, NULL);
    server = accept_connection(listener);
    
    TEST(listener != NULL);
    TEST(client != NULL);
    TEST(server != NULL);
    
    /* Generate 1MB of test data */
    size_t data_size = 1024 * 1024;
    unsigned char *send_data = malloc(data_size);
    unsigned char *recv_data = malloc(data_size);
    
    TEST(send_data != NULL);
    TEST(recv_data != NULL);
    
    /* Fill with pattern */
    for (size_t i = 0; i < data_size; i++) {
        send_data[i] = (unsigned char)(i % 256);
    }
    
    /* Send in chunks */
    size_t total_sent = 0;
    size_t chunk_size = 16384; /* 16KB chunks */
    
    while (total_sent < data_size) {
        size_t to_send = (data_size - total_sent) < chunk_size ? 
                         (data_size - total_sent) : chunk_size;
        
        int sent = send_data(client, send_data + total_sent, to_send);
        TEST(sent == to_send);
        total_sent += sent;
    }
    
    printf("  Sent %zu bytes in chunks\n", total_sent);
    
    /* Receive in chunks */
    size_t total_received = 0;
    while (total_received < data_size) {
        int received = receive_data(server, recv_data + total_received, 
                                   data_size - total_received);
        TEST(received > 0);
        total_received += received;
    }
    
    printf("  Received %zu bytes\n", total_received);
    
    /* Verify data */
    TEST(total_sent == total_received);
    TEST(memcmp(send_data, recv_data, data_size) == 0);
    
    free(send_data);
    free(recv_data);
}

/* Test 8: Timeout and error conditions */
void test_timeouts_errors(void) {
    printf("\n=== Test 8: Timeouts and Errors ===\n");
    
    /* Test invalid parameters */
    connection_t *conn = connect_to_host(NULL, 80, NULL);
    TEST(conn == NULL);
    
    conn = connect_to_host("localhost", 0, NULL);
    TEST(conn == NULL);
    
    conn = connect_to_host("localhost", 99999, NULL);
    TEST(conn == NULL);
    
    /* Test non-existent host */
    conn = connect_to_host("nonexistent.invalid", 80, NULL);
    TEST(conn == NULL);
    
    /* Test connection to closed port */
    cleanup();
    listener = create_listener(test_port + 4, NULL);
    TEST(listener != NULL);
    
    close_connection(listener);
    listener = NULL;
    
    /* Try to connect to closed port */
    conn = connect_to_host("127.0.0.1", test_port + 4, NULL);
    TEST(conn == NULL); /* Should fail */
}

/* Test 9: Multiple connections */
void test_multiple_connections(void) {
    printf("\n=== Test 9: Multiple Connections ===\n");
    
    cleanup();
    
    /* Create listener */
    int multi_port = test_port + 5;
    listener = create_listener(multi_port, NULL);
    TEST(listener != NULL);
    
    /* Create multiple clients */
    connection_t *clients[3];
    connection_t *servers[3];
    
    for (int i = 0; i < 3; i++) {
        clients[i] = connect_to_host("127.0.0.1", multi_port, NULL);
        TEST(clients[i] != NULL);
        
        servers[i] = accept_connection(listener);
        TEST(servers[i] != NULL);
        
        printf("  Connection %d established\n", i + 1);
    }
    
    /* Send data from each client */
    for (int i = 0; i < 3; i++) {
        char message[64];
        snprintf(message, sizeof(message), "Message from client %d", i);
        
        int sent = send_data(clients[i], (unsigned char*)message, strlen(message));
        TEST(sent == strlen(message));
    }
    
    /* Receive data on servers */
    for (int i = 0; i < 3; i++) {
        unsigned char buffer[64];
        int received = receive_data(servers[i], buffer, sizeof(buffer));
        TEST(received > 0);
        buffer[received] = '\0';
        
        printf("  Server %d received: %s\n", i, buffer);
    }
    
    /* Cleanup */
    for (int i = 0; i < 3; i++) {
        close_connection(clients[i]);
        close_connection(servers[i]);
    }
}

/* Main test runner */
int main(void) {
    printf("========================================\n");
    printf("     CRYPTCAT NETWORK UNIT TESTS\n");
    printf("========================================\n");
    
    /* Initialize */
    logger_init("test_network.log", LOG_INFO, 0);
    setup();
    
    /* Run tests */
    test_network_init();
    test_create_listener();
    test_local_connection();
    test_data_transmission();
    test_encrypted_connection();
    test_connection_states();
    test_large_data_transfer();
    test_timeouts_errors();
    test_multiple_connections();
    
    /* Cleanup */
    cleanup();
    
    /* Print summary */
    printf("\n========================================\n");
    printf("TEST SUMMARY:\n");
    printf("  Tests passed: %d\n", tests_passed);
    printf("  Tests failed: %d\n", tests_failed);
    printf("  Total tests:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✅ ALL NETWORK TESTS PASSED!\n");
    } else {
        printf("\n❌ SOME NETWORK TESTS FAILED!\n");
    }
    
    logger_shutdown();
    return tests_failed == 0 ? 0 : 1;
}