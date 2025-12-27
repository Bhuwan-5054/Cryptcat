/*
 * Cryptcat Network Layer
 * Platform-agnostic networking implementation
 * Version: 1.0.0
 * License: MIT
 */

#include "network.h"
#include "crypto.h"
#include "platform.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/* Network constants */
#define DEFAULT_PORT 4444
#define MAX_CONNECTIONS 128
#define RECV_TIMEOUT_SEC 30
#define SEND_TIMEOUT_SEC 30
#define KEEPALIVE_INTERVAL 60
#define MAX_RETRIES 3
#define BACKOFF_DELAY_MS 1000

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

/* Complete connection structure */
typedef struct connection_s {
    int sockfd;                     /* Socket file descriptor */
    connection_state_t state;       /* Current connection state */
    crypto_session_t *crypto;       /* Cryptographic session */
    struct sockaddr_storage addr;   /* Remote address */
    socklen_t addr_len;             /* Address length */
    char remote_host[256];          /* Remote hostname */
    int remote_port;                /* Remote port */
    time_t connected_at;            /* Connection timestamp */
    time_t last_activity;           /* Last data transfer */
    uint64_t bytes_sent;            /* Total bytes sent */
    uint64_t bytes_received;        /* Total bytes received */
    uint32_t packets_sent;          /* Total packets sent */
    uint32_t packets_received;      /* Total packets received */
    int is_listening;               /* Listening socket flag */
    int is_encrypted;               /* Encryption enabled flag */
    char *password;                 /* Encryption password */
    void *user_data;                /* User-defined data */
} connection_t;

/* Internal function prototypes */
static int create_socket(int domain, int type, int protocol);
static int set_socket_options(int sockfd);
static int connect_with_retry(const char *host, int port, int max_retries);
static int resolve_hostname(const char *host, struct sockaddr_storage *addr);
static void update_connection_stats(connection_t *conn, int is_send, size_t bytes);

/* Initialize network subsystem */
int network_init(void) {
    static int initialized = 0;
    
    if (initialized) {
        return NETWORK_SUCCESS;
    }
    
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        LOG_ERROR("WSAStartup failed: %d", WSAGetLastError());
        return NETWORK_ERROR_INIT;
    }
    
    if (LOBYTE(wsa.wVersion) != 2 || HIBYTE(wsa.wVersion) != 2) {
        LOG_ERROR("Winsock version 2.2 not available");
        WSACleanup();
        return NETWORK_ERROR_INIT;
    }
#endif
    
    LOG_INFO("Network subsystem initialized");
    initialized = 1;
    return NETWORK_SUCCESS;
}

/* Create a listening socket */
connection_t* create_listener(int port, const char *password) {
    connection_t *listener = NULL;
    struct sockaddr_in addr;
    int sockfd, opt = 1;
    
    if (port <= 0 || port > 65535) {
        LOG_ERROR("Invalid port number: %d", port);
        return NULL;
    }
    
    /* Create socket */
    sockfd = create_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG_ERROR("Failed to create socket");
        return NULL;
    }
    
    /* Set socket options */
    if (set_socket_options(sockfd) != NETWORK_SUCCESS) {
        close_socket(sockfd);
        return NULL;
    }
    
    /* Enable address reuse */
#ifdef _WIN32
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
#else
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
#endif
        LOG_ERROR("setsockopt(SO_REUSEADDR) failed: %s", strerror(errno));
        close_socket(sockfd);
        return NULL;
    }
    
#ifdef SO_REUSEPORT
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        LOG_WARNING("setsockopt(SO_REUSEPORT) failed (non-critical): %s", 
                   strerror(errno));
    }
#endif
    
    /* Bind to address */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("bind failed on port %d: %s", port, strerror(errno));
        close_socket(sockfd);
        return NULL;
    }
    
    /* Start listening */
    if (listen(sockfd, MAX_CONNECTIONS) < 0) {
        LOG_ERROR("listen failed: %s", strerror(errno));
        close_socket(sockfd);
        return NULL;
    }
    
    /* Create connection structure */
    listener = calloc(1, sizeof(connection_t));
    if (!listener) {
        LOG_ERROR("Memory allocation failed");
        close_socket(sockfd);
        return NULL;
    }
    
    listener->sockfd = sockfd;
    listener->state = STATE_READY;
    listener->is_listening = 1;
    listener->remote_port = port;
    listener->connected_at = time(NULL);
    listener->last_activity = listener->connected_at;
    
    if (password) {
        listener->password = strdup(password);
        listener->is_encrypted = 1;
    }
    
    strcpy(listener->remote_host, "0.0.0.0");
    
    LOG_INFO("Listening on port %d%s", port, 
             listener->is_encrypted ? " (encrypted)" : "");
    
    return listener;
}

/* Accept an incoming connection */
connection_t* accept_connection(connection_t *listener) {
    connection_t *client = NULL;
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char ip_str[INET6_ADDRSTRLEN];
    int client_fd;
    
    if (!listener || !listener->is_listening) {
        LOG_ERROR("Invalid listener");
        return NULL;
    }
    
    /* Accept connection */
    client_fd = accept(listener->sockfd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        LOG_ERROR("accept failed: %s", strerror(errno));
        return NULL;
    }
    
    /* Set socket options for client */
    if (set_socket_options(client_fd) != NETWORK_SUCCESS) {
        close_socket(client_fd);
        return NULL;
    }
    
    /* Get client IP address */
    if (client_addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in*)&client_addr;
        inet_ntop(AF_INET, &s->sin_addr, ip_str, sizeof(ip_str));
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6*)&client_addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ip_str, sizeof(ip_str));
    }
    
    /* Create client connection structure */
    client = calloc(1, sizeof(connection_t));
    if (!client) {
        LOG_ERROR("Memory allocation failed");
        close_socket(client_fd);
        return NULL;
    }
    
    client->sockfd = client_fd;
    client->state = STATE_CONNECTED;
    client->addr = client_addr;
    client->addr_len = addr_len;
    client->connected_at = time(NULL);
    client->last_activity = client->connected_at;
    
    strncpy(client->remote_host, ip_str, sizeof(client->remote_host) - 1);
    
    /* Get remote port */
    if (client_addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in*)&client_addr;
        client->remote_port = ntohs(s->sin_port);
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6*)&client_addr;
        client->remote_port = ntohs(s->sin6_port);
    }
    
    /* Setup encryption if listener has password */
    if (listener->is_encrypted && listener->password) {
        client->password = strdup(listener->password);
        client->is_encrypted = 1;
        
        /* Create crypto session */
        client->crypto = crypto_session_create(listener->password);
        if (!client->crypto) {
            LOG_ERROR("Failed to create crypto session for client");
            free(client->password);
            free(client);
            close_socket(client_fd);
            return NULL;
        }
        
        client->state = STATE_AUTHENTICATING;
    }
    
    LOG_INFO("Accepted connection from %s:%d%s", ip_str, client->remote_port,
             client->is_encrypted ? " (encrypted)" : "");
    
    return client;
}

/* Connect to remote host */
connection_t* connect_to_host(const char *host, int port, const char *password) {
    connection_t *conn = NULL;
    struct sockaddr_storage addr;
    int sockfd;
    
    if (!host || port <= 0 || port > 65535) {
        LOG_ERROR("Invalid parameters: host=%s, port=%d", host ? host : "NULL", port);
        return NULL;
    }
    
    /* Resolve hostname */
    if (resolve_hostname(host, &addr) != NETWORK_SUCCESS) {
        LOG_ERROR("Failed to resolve hostname: %s", host);
        return NULL;
    }
    
    /* Create socket */
    sockfd = create_socket(addr.ss_family, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG_ERROR("Failed to create socket");
        return NULL;
    }
    
    /* Set socket options */
    if (set_socket_options(sockfd) != NETWORK_SUCCESS) {
        close_socket(sockfd);
        return NULL;
    }
    
    /* Connect with retry logic */
    sockfd = connect_with_retry(host, port, MAX_RETRIES);
    if (sockfd < 0) {
        LOG_ERROR("Failed to connect to %s:%d", host, port);
        return NULL;
    }
    
    /* Create connection structure */
    conn = calloc(1, sizeof(connection_t));
    if (!conn) {
        LOG_ERROR("Memory allocation failed");
        close_socket(sockfd);
        return NULL;
    }
    
    conn->sockfd = sockfd;
    conn->state = STATE_CONNECTED;
    conn->addr = addr;
    conn->addr_len = sizeof(addr);
    conn->connected_at = time(NULL);
    conn->last_activity = conn->connected_at;
    conn->remote_port = port;
    
    strncpy(conn->remote_host, host, sizeof(conn->remote_host) - 1);
    
    /* Setup encryption if password provided */
    if (password && strlen(password) > 0) {
        conn->password = strdup(password);
        conn->is_encrypted = 1;
        
        conn->crypto = crypto_session_create(password);
        if (!conn->crypto) {
            LOG_ERROR("Failed to create crypto session");
            free(conn->password);
            free(conn);
            close_socket(sockfd);
            return NULL;
        }
        
        conn->state = STATE_AUTHENTICATING;
    }
    
    LOG_INFO("Connected to %s:%d%s", host, port, 
             conn->is_encrypted ? " (encrypted)" : "");
    
    return conn;
}

/* Send data through connection */
int send_data(connection_t *conn, const unsigned char *data, size_t len) {
    ssize_t sent;
    size_t total_sent = 0;
    
    if (!conn || conn->state != STATE_READY) {
        LOG_ERROR("Invalid connection or not ready");
        return NETWORK_ERROR_STATE;
    }
    
    if (!data || len == 0) {
        LOG_ERROR("Invalid data parameters");
        return NETWORK_ERROR_PARAM;
    }
    
    /* Encrypt data if encryption is enabled */
    if (conn->is_encrypted && conn->crypto) {
        unsigned char encrypted[len + 64]; /* Extra space for overhead */
        size_t encrypted_len;
        
        if (crypto_encrypt(conn->crypto, data, len, encrypted, &encrypted_len) 
            != CRYPTO_SUCCESS) {
            LOG_ERROR("Encryption failed");
            return NETWORK_ERROR_CRYPTO;
        }
        
        data = encrypted;
        len = encrypted_len;
    }
    
    /* Send data */
    while (total_sent < len) {
        sent = send(conn->sockfd, data + total_sent, len - total_sent, 0);
        
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                /* Would block, try again */
                continue;
            } else if (errno == EINTR) {
                /* Interrupted, try again */
                continue;
            } else {
                LOG_ERROR("send failed: %s", strerror(errno));
                conn->state = STATE_ERROR;
                return NETWORK_ERROR_IO;
            }
        } else if (sent == 0) {
            /* Connection closed */
            LOG_WARNING("Connection closed by peer during send");
            conn->state = STATE_CLOSING;
            return NETWORK_ERROR_CLOSED;
        }
        
        total_sent += sent;
    }
    
    /* Update statistics */
    update_connection_stats(conn, 1, total_sent);
    
    return total_sent;
}

/* Receive data from connection */
int receive_data(connection_t *conn, unsigned char *buffer, size_t max_len) {
    ssize_t received;
    
    if (!conn || conn->state != STATE_READY) {
        LOG_ERROR("Invalid connection or not ready");
        return NETWORK_ERROR_STATE;
    }
    
    if (!buffer || max_len == 0) {
        LOG_ERROR("Invalid buffer parameters");
        return NETWORK_ERROR_PARAM;
    }
    
    /* Receive data */
    received = recv(conn->sockfd, buffer, max_len, 0);
    
    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* No data available */
            return 0;
        } else if (errno == EINTR) {
            /* Interrupted */
            return 0;
        } else {
            LOG_ERROR("recv failed: %s", strerror(errno));
            conn->state = STATE_ERROR;
            return NETWORK_ERROR_IO;
        }
    } else if (received == 0) {
        /* Connection closed */
        LOG_INFO("Connection closed by peer");
        conn->state = STATE_CLOSING;
        return NETWORK_ERROR_CLOSED;
    }
    
    /* Update statistics */
    update_connection_stats(conn, 0, received);
    
    /* Decrypt data if encryption is enabled */
    if (conn->is_encrypted && conn->crypto && received > 0) {
        unsigned char decrypted[received];
        size_t decrypted_len;
        
        if (crypto_decrypt(conn->crypto, buffer, received, decrypted, &decrypted_len)
            != CRYPTO_SUCCESS) {
            LOG_ERROR("Decryption failed");
            return NETWORK_ERROR_CRYPTO;
        }
        
        /* Copy decrypted data back to buffer */
        if (decrypted_len > max_len) {
            LOG_ERROR("Decrypted data too large for buffer");
            return NETWORK_ERROR_BUFFER;
        }
        
        memcpy(buffer, decrypted, decrypted_len);
        received = decrypted_len;
        
        /* Securely wipe temporary buffer */
        memset(decrypted, 0, received);
    }
    
    return received;
}

/* Close connection */
void close_connection(connection_t *conn) {
    if (!conn) return;
    
    LOG_DEBUG("Closing connection to %s:%d", conn->remote_host, conn->remote_port);
    
    /* Close socket */
    if (conn->sockfd >= 0) {
        shutdown(conn->sockfd, SHUT_RDWR);
        close_socket(conn->sockfd);
        conn->sockfd = -1;
    }
    
    /* Destroy crypto session */
    if (conn->crypto) {
        crypto_session_destroy(conn->crypto);
        conn->crypto = NULL;
    }
    
    /* Free password */
    if (conn->password) {
        memset(conn->password, 0, strlen(conn->password));
        free(conn->password);
        conn->password = NULL;
    }
    
    /* Free user data */
    if (conn->user_data) {
        free(conn->user_data);
        conn->user_data = NULL;
    }
    
    /* Update state */
    conn->state = STATE_DISCONNECTED;
    
    LOG_INFO("Connection to %s:%d closed", conn->remote_host, conn->remote_port);
}

/* Get connection information */
connection_info_t get_connection_info(connection_t *conn) {
    connection_info_t info = {0};
    
    if (conn) {
        info.state = conn->state;
        info.is_encrypted = conn->is_encrypted;
        info.is_listening = conn->is_listening;
        info.bytes_sent = conn->bytes_sent;
        info.bytes_received = conn->bytes_received;
        info.packets_sent = conn->packets_sent;
        info.packets_received = conn->packets_received;
        info.connection_time = time(NULL) - conn->connected_at;
        info.idle_time = time(NULL) - conn->last_activity;
        
        strncpy(info.remote_host, conn->remote_host, sizeof(info.remote_host) - 1);
        info.remote_port = conn->remote_port;
    }
    
    return info;
}

/* Set socket options */
static int set_socket_options(int sockfd) {
    int opt;
    
    /* Set non-blocking mode */
    opt = fcntl(sockfd, F_GETFL, 0);
    if (opt < 0) {
        LOG_ERROR("fcntl(F_GETFL) failed: %s", strerror(errno));
        return NETWORK_ERROR_IO;
    }
    
    if (fcntl(sockfd, F_SETFL, opt | O_NONBLOCK) < 0) {
        LOG_ERROR("fcntl(F_SETFL) failed: %s", strerror(errno));
        return NETWORK_ERROR_IO;
    }
    
    /* Set receive timeout */
    struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT_SEC;
    tv.tv_usec = 0;
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) < 0) {
        LOG_WARNING("setsockopt(SO_RCVTIMEO) failed: %s", strerror(errno));
    }
    
    /* Set send timeout */
    tv.tv_sec = SEND_TIMEOUT_SEC;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv)) < 0) {
        LOG_WARNING("setsockopt(SO_SNDTIMEO) failed: %s", strerror(errno));
    }
    
    /* Enable TCP keepalive */
    opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt, sizeof(opt)) < 0) {
        LOG_WARNING("setsockopt(SO_KEEPALIVE) failed: %s", strerror(errno));
    }
    
    return NETWORK_SUCCESS;
}

/* Connect with retry logic */
static int connect_with_retry(const char *host, int port, int max_retries) {
    struct addrinfo hints, *res, *p;
    char port_str[16];
    int sockfd = -1;
    int retries = 0;
    
    snprintf(port_str, sizeof(port_str), "%d", port);
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    while (retries < max_retries) {
        if (getaddrinfo(host, port_str, &hints, &res) != 0) {
            LOG_ERROR("getaddrinfo failed for %s:%s", host, port_str);
            return -1;
        }
        
        /* Try each address */
        for (p = res; p != NULL; p = p->ai_next) {
            sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sockfd < 0) {
                continue;
            }
            
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) {
                /* Success */
                break;
            }
            
            close_socket(sockfd);
            sockfd = -1;
        }
        
        freeaddrinfo(res);
        
        if (sockfd >= 0) {
            /* Connection successful */
            return sockfd;
        }
        
        /* Retry after delay */
        retries++;
        if (retries < max_retries) {
            LOG_WARNING("Connection attempt %d/%d failed, retrying in %dms...",
                       retries, max_retries, BACKOFF_DELAY_MS);
            platform_sleep_ms(BACKOFF_DELAY_MS);
        }
    }
    
    LOG_ERROR("Failed to connect after %d attempts", max_retries);
    return -1;
}

/* Update connection statistics */
static void update_connection_stats(connection_t *conn, int is_send, size_t bytes) {
    if (!conn) return;
    
    conn->last_activity = time(NULL);
    
    if (is_send) {
        conn->bytes_sent += bytes;
        conn->packets_sent++;
    } else {
        conn->bytes_received += bytes;
        conn->packets_received++;
    }
}