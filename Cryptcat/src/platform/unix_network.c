/*
 * Unix/Linux Platform Implementation
 * Version: 1.0.0
 * License: MIT
 */

#include "platform.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>

/* Initialize Unix networking */
int platform_network_init(void) {
    /* Unix systems don't need special initialization */
    return PLATFORM_SUCCESS;
}

/* Create socket */
int platform_create_socket(int domain, int type, int protocol) {
    return socket(domain, type, protocol);
}

/* Close socket */
void platform_close_socket(int sockfd) {
    close(sockfd);
}

/* Set socket to non-blocking mode */
int platform_set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) return PLATFORM_ERROR_IO;
    
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return PLATFORM_ERROR_IO;
    }
    
    return PLATFORM_SUCCESS;
}

/* Generate random bytes using /dev/urandom */
int platform_random_bytes(unsigned char *buffer, size_t length) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return PLATFORM_ERROR_IO;
    }
    
    ssize_t bytes_read = read(fd, buffer, length);
    close(fd);
    
    return (bytes_read == length) ? PLATFORM_SUCCESS : PLATFORM_ERROR_IO;
}

/* Seed random number generator */
int platform_random_seed(void) {
    /* /dev/urandom is already seeded on Unix systems */
    return PLATFORM_SUCCESS;
}

/* Sleep for milliseconds */
void platform_sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

/* Get system time in milliseconds */
uint64_t platform_get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/* Get local IP address */
int platform_get_local_ip(char *buffer, size_t buffer_size) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return PLATFORM_ERROR_NETWORK;
    }
    
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &serv.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        close(sock);
        return PLATFORM_ERROR_NETWORK;
    }
    
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    getsockname(sock, (struct sockaddr*)&name, &namelen);
    
    inet_ntop(AF_INET, &name.sin_addr, buffer, buffer_size);
    close(sock);
    
    return PLATFORM_SUCCESS;
}

/* Create directory */
int platform_create_directory(const char *path) {
    return mkdir(path, 0755);
}

/* Check if file exists */
int platform_file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

/* Get file size */
int64_t platform_get_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

/* Thread functions */
platform_thread_t platform_thread_create(platform_thread_func func, void *arg) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, func, arg) != 0) {
        return NULL;
    }
    return (platform_thread_t)thread;
}

void platform_thread_join(platform_thread_t thread) {
    pthread_join((pthread_t)thread, NULL);
}

void platform_thread_detach(platform_thread_t thread) {
    pthread_detach((pthread_t)thread);
}

/* Mutex functions */
platform_mutex_t platform_mutex_create(void) {
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if (!mutex) return NULL;
    
    pthread_mutex_init(mutex, NULL);
    return (platform_mutex_t)mutex;
}

void platform_mutex_lock(platform_mutex_t mutex) {
    pthread_mutex_lock((pthread_mutex_t*)mutex);
}

void platform_mutex_unlock(platform_mutex_t mutex) {
    pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

void platform_mutex_destroy(platform_mutex_t mutex) {
    pthread_mutex_destroy((pthread_mutex_t*)mutex);
    free(mutex);
}