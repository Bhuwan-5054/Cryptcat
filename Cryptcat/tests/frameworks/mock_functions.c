/*
 * Mock Functions for Testing
 * Version: 1.0.0
 * License: MIT
 */

#include "test_harness.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* Mock network functions */
typedef struct mock_socket_s {
    int fd;
    int domain;
    int type;
    int protocol;
    int connected;
    struct sockaddr_storage addr;
    socklen_t addr_len;
    char buffer[8192];
    size_t buffer_len;
    size_t buffer_pos;
} mock_socket_t;

static mock_socket_t mock_sockets[1024];
static int next_mock_fd = 1000;

/* Mock socket creation */
int mock_socket(int domain, int type, int protocol) {
    int fd = next_mock_fd++;
    
    if (fd >= 1000 + (int)(sizeof(mock_sockets)/sizeof(mock_sockets[0]))) {
        errno = EMFILE;
        return -1;
    }
    
    mock_socket_t *sock = &mock_sockets[fd - 1000];
    memset(sock, 0, sizeof(mock_socket_t));
    
    sock->fd = fd;
    sock->domain = domain;
    sock->type = type;
    sock->protocol = protocol;
    sock->connected = 0;
    
    return fd;
}

/* Mock socket close */
int mock_close(int fd) {
    if (fd < 1000 || fd >= 1000 + (int)(sizeof(mock_sockets)/sizeof(mock_sockets[0]))) {
        errno = EBADF;
        return -1;
    }
    
    mock_socket_t *sock = &mock_sockets[fd - 1000];
    memset(sock, 0, sizeof(mock_socket_t));
    
    return 0;
}

/* Mock socket connect */
int mock_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (sockfd < 1000 || sockfd >= 1000 + (int)(sizeof(mock_sockets)/sizeof(mock_sockets[0]))) {
        errno = EBADF;
        return -1;
    }
    
    mock_socket_t *sock = &mock_sockets[sockfd - 1000];
    
    if (sock->connected) {
        errno = EISCONN;
        return -1;
    }
    
    memcpy(&sock->addr, addr, addrlen);
    sock->addr_len = addrlen;
    sock->connected = 1;
    
    return 0;
}

/* Mock socket bind */
int mock_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (sockfd < 1000 || sockfd >= 1000 + (int)(sizeof(mock_sockets)/sizeof(mock_sockets[0]))) {
        errno = EBADF;
        return -1;
    }
    
    mock_socket_t *sock = &mock_sockets[sockfd - 1000];
    
    memcpy(&sock->addr, addr, addrlen);
    sock->addr_len = addrlen;
    
    return 0;
}

/* Mock socket listen */
int mock_listen(int sockfd, int backlog) {
    (void)backlog;
    
    if (sockfd < 1000 || sockfd >= 1000 + (int)(sizeof(mock_sockets)/sizeof(mock_sockets[0]))) {
        errno = EBADF;
        return -1;
    }
    
    return 0;
}

/* Mock socket accept */
int mock_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    (void)sockfd;
    (void)addr;
    (void)addrlen;
    
    /* Return a new mock socket */
    int new_fd = mock_socket(AF_INET, SOCK_STREAM, 0);
    if (new_fd < 0) {
        return -1;
    }
    
    mock_socket_t *new_sock = &mock_sockets[new_fd - 1000];
    new_sock->connected = 1;
    
    return new_fd;
}

/* Mock socket send */
ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags) {
    (void)flags;
    
    if (sockfd < 1000 || sockfd >= 1000 + (int)(sizeof(mock_sockets)/sizeof(mock_sockets[0]))) {
        errno = EBADF;
        return -1;
    }
    
    mock_socket_t *sock = &mock_sockets[sockfd - 1000];
    
    if (!sock->connected) {
        errno = ENOTCONN;
        return -1;
    }
    
    /* Store sent data in buffer */
    if (sock->buffer_pos + len > sizeof(sock->buffer)) {
        errno = ENOBUFS;
        return -1;
    }
    
    memcpy(sock->buffer + sock->buffer_pos, buf, len);
    sock->buffer_pos += len;
    
    return len;
}

/* Mock socket receive */
ssize_t mock_recv(int sockfd, void *buf, size_t len, int flags) {
    (void)flags;
    
    if (sockfd < 1000 || sockfd >= 1000 + (int)(sizeof(mock_sockets)/sizeof(mock_sockets[0]))) {
        errno = EBADF;
        return -1;
    }
    
    mock_socket_t *sock = &mock_sockets[sockfd - 1000];
    
    if (!sock->connected) {
        errno = ENOTCONN;
        return -1;
    }
    
    if (sock->buffer_len == 0) {
        /* No data available */
        errno = EAGAIN;
        return -1;
    }
    
    size_t to_copy = (len < sock->buffer_len) ? len : sock->buffer_len;
    memcpy(buf, sock->buffer, to_copy);
    
    /* Shift remaining data */
    memmove(sock->buffer, sock->buffer + to_copy, sock->buffer_len - to_copy);
    sock->buffer_len -= to_copy;
    
    return to_copy;
}

/* Mock file operations */
typedef struct mock_file_s {
    int fd;
    char *filename;
    char *content;
    size_t content_size;
    size_t position;
    int flags;
} mock_file_t;

static mock_file_t mock_files[256];
static int next_mock_file_fd = 2000;

/* Mock file open */
int mock_open(const char *pathname, int flags, ...) {
    int fd = next_mock_file_fd++;
    
    if (fd >= 2000 + (int)(sizeof(mock_files)/sizeof(mock_files[0]))) {
        errno = EMFILE;
        return -1;
    }
    
    mock_file_t *file = &mock_files[fd - 2000];
    memset(file, 0, sizeof(mock_file_t));
    
    file->fd = fd;
    file->filename = strdup(pathname);
    file->flags = flags;
    file->position = 0;
    
    /* For testing, preload some files */
    if (strcmp(pathname, "/etc/hosts") == 0) {
        file->content = strdup("127.0.0.1 localhost\n");
        file->content_size = strlen(file->content);
    } else if (strcmp(pathname, "/dev/urandom") == 0) {
        /* Mock urandom with predictable data */
        file->content = malloc(1024);
        for (int i = 0; i < 1024; i++) {
            file->content[i] = i % 256;
        }
        file->content_size = 1024;
    }
    
    return fd;
}

/* Mock file close */
int mock_close_file(int fd) {
    if (fd < 2000 || fd >= 2000 + (int)(sizeof(mock_files)/sizeof(mock_files[0]))) {
        errno = EBADF;
        return -1;
    }
    
    mock_file_t *file = &mock_files[fd - 2000];
    
    free(file->filename);
    free(file->content);
    memset(file, 0, sizeof(mock_file_t));
    
    return 0;
}

/* Mock file read */
ssize_t mock_read(int fd, void *buf, size_t count) {
    if (fd < 2000 || fd >= 2000 + (int)(sizeof(mock_files)/sizeof(mock_files[0]))) {
        errno = EBADF;
        return -1;
    }
    
    mock_file_t *file = &mock_files[fd - 2000];
    
    if (!file->content) {
        errno = EIO;
        return -1;
    }
    
    size_t remaining = file->content_size - file->position;
    size_t to_read = (count < remaining) ? count : remaining;
    
    if (to_read == 0) {
        return 0; /* EOF */
    }
    
    memcpy(buf, file->content + file->position, to_read);
    file->position += to_read;
    
    return to_read;
}

/* Mock file write */
ssize_t mock_write(int fd, const void *buf, size_t count) {
    if (fd < 2000 || fd >= 2000 + (int)(sizeof(mock_files)/sizeof(mock_files[0]))) {
        errno = EBADF;
        return -1;
    }
    
    mock_file_t *file = &mock_files[fd - 2000];
    
    /* Resize buffer if needed */
    size_t new_size = file->position + count;
    if (new_size > file->content_size) {
        char *new_content = realloc(file->content, new_size);
        if (!new_content) {
            errno = ENOSPC;
            return -1;
        }
        file->content = new_content;
        file->content_size = new_size;
    }
    
    memcpy(file->content + file->position, buf, count);
    file->position += count;
    
    return count;
}

/* Mock file seek */
off_t mock_lseek(int fd, off_t offset, int whence) {
    if (fd < 2000 || fd >= 2000 + (int)(sizeof(mock_files)/sizeof(mock_files[0]))) {
        errno = EBADF;
        return -1;
    }
    
    mock_file_t *file = &mock_files[fd - 2000];
    
    switch (whence) {
        case SEEK_SET:
            file->position = offset;
            break;
        case SEEK_CUR:
            file->position += offset;
            break;
        case SEEK_END:
            file->position = file->content_size + offset;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    
    if (file->position > file->content_size) {
        file->position = file->content_size;
    }
    
    return file->position;
}

/* Mock time function */
time_t mock_time(time_t *tloc) {
    static time_t current_time = 1609459200; /* 2021-01-01 00:00:00 UTC */
    
    if (tloc) {
        *tloc = current_time;
    }
    
    return current_time;
}

/* Mock random functions */
int mock_rand(void) {
    static unsigned int seed = 12345;
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

void mock_srand(unsigned int seed) {
    (void)seed;
    /* Mock doesn't use seed for predictability */
}