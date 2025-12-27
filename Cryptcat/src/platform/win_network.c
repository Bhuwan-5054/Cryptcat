/*
 * Windows Platform Implementation
 * Version: 1.0.0
 * License: MIT
 */

#include "platform.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <bcrypt.h>
#include <io.h>
#include <direct.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")

/* Initialize Windows networking */
int platform_network_init(void) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return PLATFORM_ERROR_NETWORK;
    }
    
    if (LOBYTE(wsa.wVersion) != 2 || HIBYTE(wsa.wVersion) != 2) {
        WSACleanup();
        return PLATFORM_ERROR_NETWORK;
    }
    
    return PLATFORM_SUCCESS;
}

/* Create socket */
int platform_create_socket(int domain, int type, int protocol) {
    return socket(domain, type, protocol);
}

/* Close socket */
void platform_close_socket(int sockfd) {
    closesocket(sockfd);
}

/* Set socket to non-blocking mode */
int platform_set_nonblocking(int sockfd) {
    u_long mode = 1;
    if (ioctlsocket(sockfd, FIONBIO, &mode) != 0) {
        return PLATFORM_ERROR_IO;
    }
    return PLATFORM_SUCCESS;
}

/* Generate random bytes using CryptGenRandom */
int platform_random_bytes(unsigned char *buffer, size_t length) {
    NTSTATUS status = BCryptGenRandom(
        NULL,
        buffer,
        length,
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );
    
    return (status == STATUS_SUCCESS) ? PLATFORM_SUCCESS : PLATFORM_ERROR_CRYPTO;
}

/* Seed random number generator */
int platform_random_seed(void) {
    /* Windows CryptGenRandom is automatically seeded */
    return PLATFORM_SUCCESS;
}

/* Sleep for milliseconds */
void platform_sleep_ms(int milliseconds) {
    Sleep(milliseconds);
}

/* Get system time in milliseconds */
uint64_t platform_get_time_ms(void) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    
    return (uli.QuadPart - 116444736000000000ULL) / 10000ULL;
}

/* Get local IP address */
int platform_get_local_ip(char *buffer, size_t buffer_size) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return PLATFORM_ERROR_NETWORK;
    }
    
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        WSACleanup();
        return PLATFORM_ERROR_NETWORK;
    }
    
    struct hostent *host = gethostbyname(hostname);
    if (!host) {
        WSACleanup();
        return PLATFORM_ERROR_NETWORK;
    }
    
    struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
    if (addr_list[0]) {
        strncpy(buffer, inet_ntoa(*addr_list[0]), buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
        WSACleanup();
        return PLATFORM_SUCCESS;
    }
    
    WSACleanup();
    return PLATFORM_ERROR_NETWORK;
}

/* Create directory */
int platform_create_directory(const char *path) {
    return _mkdir(path);
}

/* Check if file exists */
int platform_file_exists(const char *path) {
    return _access(path, 0) == 0;
}

/* Get file size */
int64_t platform_get_file_size(const char *path) {
    struct _stat64 st;
    if (_stat64(path, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

/* Thread functions */
platform_thread_t platform_thread_create(platform_thread_func func, void *arg) {
    return (platform_thread_t)_beginthread(func, 0, arg);
}

void platform_thread_join(platform_thread_t thread) {
    WaitForSingleObject((HANDLE)thread, INFINITE);
    CloseHandle((HANDLE)thread);
}

void platform_thread_detach(platform_thread_t thread) {
    CloseHandle((HANDLE)thread);
}

/* Mutex functions */
platform_mutex_t platform_mutex_create(void) {
    CRITICAL_SECTION *cs = malloc(sizeof(CRITICAL_SECTION));
    if (!cs) return NULL;
    
    InitializeCriticalSection(cs);
    return (platform_mutex_t)cs;
}

void platform_mutex_lock(platform_mutex_t mutex) {
    EnterCriticalSection((CRITICAL_SECTION*)mutex);
}

void platform_mutex_unlock(platform_mutex_t mutex) {
    LeaveCriticalSection((CRITICAL_SECTION*)mutex);
}

void platform_mutex_destroy(platform_mutex_t mutex) {
    DeleteCriticalSection((CRITICAL_SECTION*)mutex);
    free(mutex);
}