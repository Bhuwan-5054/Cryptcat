/*
 * Cryptcat Main Application
 * Entry point and command-line interface
 * Version: 1.0.0
 * License: MIT
 */

#include "crypto.h"
#include "network.h"
#include "protocol.h"
#include "file_transfer.h"
#include "chat_mode.h"
#include "p2p_network.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

/* Application version */
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_STRING "1.0.0"

/* Default configuration */
#define DEFAULT_PORT 4444
#define DEFAULT_CHUNK_SIZE 16384
#define DEFAULT_TIMEOUT 30

/* Application modes */
typedef enum {
    MODE_NONE = 0,
    MODE_CONNECT,
    MODE_LISTEN,
    MODE_FILE_SEND,
    MODE_FILE_RECEIVE,
    MODE_CHAT,
    MODE_P2P,
    MODE_SHELL,
    MODE_PORT_FORWARD
} app_mode_t;

/* Global variables */
static volatile int running = 1;
static connection_t *current_connection = NULL;
static file_transfer_t *current_transfer = NULL;
static p2p_network_t *p2p_network = NULL;

/* Signal handler */
static void signal_handler(int sig) {
    LOG_INFO("Received signal %d, shutting down...", sig);
    running = 0;
    
    /* Cleanup */
    if (current_transfer) {
        cancel_file_transfer(current_transfer);
        cleanup_file_transfer(current_transfer);
        current_transfer = NULL;
    }
    
    if (current_connection) {
        close_connection(current_connection);
        current_connection = NULL;
    }
    
    if (p2p_network) {
        p2p_stop(p2p_network);
        p2p_network = NULL;
    }
}

/* Print usage information */
static void print_usage(void) {
    printf("Cryptcat v%s - Encrypted Netcat\n\n", VERSION_STRING);
    printf("Usage: cryptcat [OPTIONS] [host] [port]\n\n");
    printf("Options:\n");
    printf("  -l, --listen           Listen mode (default: connect)\n");
    printf("  -p, --port PORT        Port number (default: %d)\n", DEFAULT_PORT);
    printf("  -k, --key PASSWORD     Encryption password (required)\n");
    printf("  -e, --execute CMD      Execute command (remote shell)\n");
    printf("  -c, --chat             Encrypted chat mode\n");
    printf("  -f, --file FILE        Send/receive file\n");
    printf("  --p2p                  Enable P2P networking\n");
    printf("  --p2p-port PORT        P2P listening port (default: 5555)\n");
    printf("  --p2p-bootstrap HOST   P2P bootstrap node\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -q, --quiet            Quiet mode\n");
    printf("  -h, --help             Show this help\n");
    printf("  -V, --version          Show version\n\n");
    printf("Examples:\n");
    printf("  cryptcat -k password -l -p 4444\n");
    printf("  cryptcat -k password 192.168.1.100 4444\n");
    printf("  cryptcat -k secret -f document.pdf 192.168.1.100 5555\n");
    printf("  cryptcat -k secret -c 192.168.1.100 4444\n");
    printf("  cryptcat --p2p --p2p-port 5555 --key password\n\n");
}

/* Print version information */
static void print_version(void) {
    printf("Cryptcat v%s\n", VERSION_STRING);
    printf("Encryption: Twofish-256-CFB with HMAC-SHA256\n");
    printf("Protocol: Cryptcat Protocol v1.0\n");
    printf("Platform: %s\n", PLATFORM_NAME);
    printf("License: MIT\n");
    printf("GitHub: https://github.com/Bhuwan-5054/Cryptcat\n\n");
}

/* Parse command line arguments */
static int parse_arguments(int argc, char *argv[], app_mode_t *mode,
                          char **host, int *port, char **password,
                          char **filename, int *p2p_port, char **bootstrap_node) {
    static struct option long_options[] = {
        {"listen", no_argument, 0, 'l'},
        {"port", required_argument, 0, 'p'},
        {"key", required_argument, 0, 'k'},
        {"execute", required_argument, 0, 'e'},
        {"chat", no_argument, 0, 'c'},
        {"file", required_argument, 0, 'f'},
        {"p2p", no_argument, 0, 256},
        {"p2p-port", required_argument, 0, 257},
        {"p2p-bootstrap", required_argument, 0, 258},
        {"verbose", no_argument, 0, 'v'},
        {"quiet", no_argument, 0, 'q'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt, option_index = 0;
    *mode = MODE_NONE;
    *port = DEFAULT_PORT;
    *p2p_port = 5555;
    
    while ((opt = getopt_long(argc, argv, "lp:k:e:cf:vqhV",
                             long_options, &option_index)) != -1) {
        switch (opt) {
            case 'l':
                *mode = MODE_LISTEN;
                break;
            case 'p':
                *port = atoi(optarg);
                if (*port <= 0 || *port > 65535) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    return -1;
                }
                break;
            case 'k':
                *password = strdup(optarg);
                break;
            case 'e':
                *mode = MODE_SHELL;
                /* Command would be stored here */
                break;
            case 'c':
                *mode = MODE_CHAT;
                break;
            case 'f':
                *mode = MODE_FILE_SEND;
                *filename = strdup(optarg);
                break;
            case 256: /* --p2p */
                *mode = MODE_P2P;
                break;
            case 257: /* --p2p-port */
                *p2p_port = atoi(optarg);
                break;
            case 258: /* --p2p-bootstrap */
                *bootstrap_node = strdup(optarg);
                break;
            case 'v':
                log_set_level(LOG_DEBUG);
                break;
            case 'q':
                log_set_level(LOG_ERROR);
                break;
            case 'h':
                print_usage();
                exit(0);
            case 'V':
                print_version();
                exit(0);
            default:
                fprintf(stderr, "Try 'cryptcat --help' for more information.\n");
                return -1;
        }
    }
    
    /* Get host and port from positional arguments */
    if (optind < argc) {
        *host = strdup(argv[optind++]);
    }
    if (optind < argc) {
        *port = atoi(argv[optind]);
    }
    
    /* Validate required parameters */
    if (!*password) {
        fprintf(stderr, "Error: Encryption password is required (-k option)\n");
        return -1;
    }
    
    /* Set default mode if not specified */
    if (*mode == MODE_NONE) {
        *mode = *host ? MODE_CONNECT : MODE_LISTEN;
    }
    
    return 0;
}

/* Initialize application */
static int initialize_app(void) {
    int result;
    
    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#ifdef SIGHUP
    signal(SIGHUP, signal_handler);
#endif
    
    /* Initialize subsystems */
    result = crypto_global_init();
    if (result != CRYPTO_SUCCESS) {
        fprintf(stderr, "Failed to initialize cryptography: %s\n",
                crypto_strerror(result));
        return -1;
    }
    
    result = network_init();
    if (result != NETWORK_SUCCESS) {
        fprintf(stderr, "Failed to initialize network: %s\n",
                network_strerror(result));
        return -1;
    }
    
    result = file_transfer_init();
    if (result != FILE_TRANSFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize file transfer: %s\n",
                file_transfer_strerror(result));
        return -1;
    }
    
    LOG_INFO("Cryptcat v%s initialized", VERSION_STRING);
    return 0;
}

/* Connect mode */
static int run_connect_mode(const char *host, int port, const char *password,
                           app_mode_t mode, const char *filename) {
    int result;
    
    LOG_INFO("Connecting to %s:%d...", host, port);
    
    current_connection = connect_to_host(host, port, password);
    if (!current_connection) {
        fprintf(stderr, "Failed to connect to %s:%d\n", host, port);
        return -1;
    }
    
    /* Perform handshake (client side) */
    result = perform_handshake(current_connection, 0, password);
    if (result != PROTOCOL_SUCCESS) {
        fprintf(stderr, "Handshake failed: %s\n", protocol_strerror(result));
        close_connection(current_connection);
        current_connection = NULL;
        return -1;
    }
    
    LOG_INFO("Connected and authenticated to %s:%d", host, port);
    
    /* Run selected mode */
    switch (mode) {
        case MODE_CHAT:
            return run_chat_mode(current_connection);
            
        case MODE_FILE_SEND:
            if (!filename) {
                fprintf(stderr, "Error: No filename specified for send\n");
                return -1;
            }
            current_transfer = start_file_send(current_connection, filename);
            if (!current_transfer) {
                fprintf(stderr, "Failed to start file transfer\n");
                return -1;
            }
            return process_file_transfer_loop(current_transfer);
            
        case MODE_SHELL:
            /* Remote shell would be implemented here */
            fprintf(stderr, "Remote shell mode not yet implemented\n");
            return -1;
            
        default:
            /* Simple data transfer mode */
            return run_data_mode(current_connection);
    }
}

/* Listen mode */
static int run_listen_mode(int port, const char *password) {
    int result;
    connection_t *client;
    
    LOG_INFO("Starting listener on port %d...", port);
    
    current_connection = create_listener(port, password);
    if (!current_connection) {
        fprintf(stderr, "Failed to create listener on port %d\n", port);
        return -1;
    }
    
    printf("Listening on port %d (encrypted with password)\n", port);
    printf("Press Ctrl+C to stop listening\n\n");
    
    while (running) {
        client = accept_connection(current_connection);
        if (!client) {
            if (running) {
                LOG_ERROR("Failed to accept connection");
            }
            continue;
        }
        
        /* Perform handshake (server side) */
        result = perform_handshake(client, 1, password);
        if (result != PROTOCOL_SUCCESS) {
            LOG_ERROR("Handshake failed: %s", protocol_strerror(result));
            close_connection(client);
            continue;
        }
        
        LOG_INFO("Client connected from %s:%d", 
                 client->remote_host, client->remote_port);
        
        printf("Client connected: %s:%d\n", 
               client->remote_host, client->remote_port);
        
        /* Handle client connection in simple echo mode */
        run_echo_mode(client);
        
        close_connection(client);
    }
    
    return 0;
}

/* Simple data mode (stdin -> network, network -> stdout) */
static int run_data_mode(connection_t *conn) {
    unsigned char buffer[8192];
    fd_set readfds;
    int max_fd, nready;
    
    printf("Data mode started. Press Ctrl+C to exit.\n");
    printf("Type text and press Enter to send.\n\n");
    
    while (running) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(conn->sockfd, &readfds);
        
        max_fd = (conn->sockfd > STDIN_FILENO) ? conn->sockfd : STDIN_FILENO;
        
        nready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (nready < 0) {
            if (errno == EINTR) continue;
            LOG_ERROR("select failed: %s", strerror(errno));
            break;
        }
        
        /* Check stdin */
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            ssize_t read_bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (read_bytes > 0) {
                int sent = send_data(conn, buffer, read_bytes);
                if (sent < 0) {
                    LOG_ERROR("Failed to send data");
                    break;
                }
            } else if (read_bytes == 0) {
                /* EOF */
                break;
            }
        }
        
        /* Check socket */
        if (FD_ISSET(conn->sockfd, &readfds)) {
            int received = receive_data(conn, buffer, sizeof(buffer));
            if (received > 0) {
                fwrite(buffer, 1, received, stdout);
                fflush(stdout);
            } else if (received == NETWORK_ERROR_CLOSED) {
                printf("\nConnection closed by peer\n");
                break;
            } else if (received < 0) {
                LOG_ERROR("Failed to receive data");
                break;
            }
        }
    }
    
    return 0;
}

/* Echo mode for server */
static int run_echo_mode(connection_t *conn) {
    unsigned char buffer[8192];
    int received;
    
    printf("Echo mode for client %s:%d\n", 
           conn->remote_host, conn->remote_port);
    
    while (running) {
        received = receive_data(conn, buffer, sizeof(buffer));
        if (received > 0) {
            /* Echo back */
            send_data(conn, buffer, received);
            
            /* Also print to console */
            printf("Client: %.*s\n", received, buffer);
        } else if (received == NETWORK_ERROR_CLOSED) {
            printf("Client disconnected\n");
            break;
        } else if (received < 0) {
            LOG_ERROR("Error in echo mode");
            break;
        }
    }
    
    return 0;
}

/* P2P networking mode */
static int run_p2p_mode(int port, const char *password, const char *bootstrap_node) {
    LOG_INFO("Starting P2P network on port %d...", port);
    
    p2p_network = p2p_init(NULL, port);
    if (!p2p_network) {
        fprintf(stderr, "Failed to initialize P2P network\n");
        return -1;
    }
    
    /* Add bootstrap node if specified */
    if (bootstrap_node) {
        char *colon = strchr(bootstrap_node, ':');
        if (colon) {
            *colon = '\0';
            int bootstrap_port = atoi(colon + 1);
            p2p_add_peer(p2p_network, bootstrap_node, bootstrap_port, 1);
        }
    }
    
    /* Start P2P network */
    if (p2p_start(p2p_network) != 0) {
        fprintf(stderr, "Failed to start P2P network\n");
        return -1;
    }
    
    printf("P2P network started on port %d\n", port);
    printf("Node ID: %s\n", p2p_get_node_id(p2p_network));
    printf("Press Ctrl+C to stop\n\n");
    
    /* Main P2P loop */
    while (running) {
        /* Display peer list every 10 seconds */
        static time_t last_display = 0;
        time_t now = time(NULL);
        
        if (now - last_display >= 10) {
            int peer_count;
            peer_node_t **peers = p2p_get_peers(p2p_network, &peer_count);
            
            printf("\n=== P2P Network Status ===\n");
            printf("Peers: %d\n", peer_count);
            
            for (int i = 0; i < peer_count; i++) {
                printf("  %s:%d (ID: %.8s...)\n",
                       peers[i]->address, peers[i]->port,
                       peers[i]->id);
            }
            
            free(peers);
            last_display = now;
        }
        
        platform_sleep_ms(1000);
    }
    
    return 0;
}

/* Main application entry point */
int main(int argc, char *argv[]) {
    app_mode_t mode = MODE_NONE;
    char *host = NULL;
    char *password = NULL;
    char *filename = NULL;
    char *bootstrap_node = NULL;
    int port = DEFAULT_PORT;
    int p2p_port = 5555;
    int result = 0;
    
    /* Parse command line arguments */
    if (parse_arguments(argc, argv, &mode, &host, &port, &password,
                       &filename, &p2p_port, &bootstrap_node) != 0) {
        return 1;
    }
    
    /* Initialize application */
    if (initialize_app() != 0) {
        return 1;
    }
    
    /* Run selected mode */
    switch (mode) {
        case MODE_CONNECT:
            result = run_connect_mode(host, port, password, mode, filename);
            break;
            
        case MODE_LISTEN:
            result = run_listen_mode(port, password);
            break;
            
        case MODE_P2P:
            result = run_p2p_mode(p2p_port, password, bootstrap_node);
            break;
            
        case MODE_CHAT:
        case MODE_FILE_SEND:
            if (!host) {
                fprintf(stderr, "Error: Host required for this mode\n");
                result = 1;
                break;
            }
            result = run_connect_mode(host, port, password, mode, filename);
            break;
            
        default:
            fprintf(stderr, "Error: Unknown mode\n");
            result = 1;
            break;
    }
    
    /* Cleanup */
    if (current_transfer) {
        cleanup_file_transfer(current_transfer);
        current_transfer = NULL;
    }
    
    if (current_connection) {
        close_connection(current_connection);
        current_connection = NULL;
    }
    
    if (p2p_network) {
        p2p_stop(p2p_network);
        p2p_network = NULL;
    }
    
    /* Free allocated memory */
    if (host) free(host);
    if (password) {
        memset(password, 0, strlen(password));
        free(password);
    }
    if (filename) free(filename);
    if (bootstrap_node) free(bootstrap_node);
    
    LOG_INFO("Cryptcat shutdown complete");
    return result;
}