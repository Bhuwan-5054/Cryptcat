/*
 * Memory Utilities - Secure memory handling
 * Version: 1.0.0
 * License: MIT
 */

#include "memory_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

/* Secure memory allocation */
void* secure_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

/* Secure calloc */
void* secure_calloc(size_t nmemb, size_t size) {
    return secure_malloc(nmemb * size);
}

/* Secure realloc */
void* secure_realloc(void *ptr, size_t old_size, size_t new_size) {
    void *new_ptr = realloc(ptr, new_size);
    if (new_ptr) {
        /* Zero new memory if growing */
        if (new_size > old_size) {
            memset((char*)new_ptr + old_size, 0, new_size - old_size);
        }
    }
    return new_ptr;
}

/* Secure free with memory zeroing */
void secure_free(void **ptr, size_t size) {
    if (ptr && *ptr) {
        memset(*ptr, 0, size);
        free(*ptr);
        *ptr = NULL;
    }
}

/* Secure string duplication */
char* secure_strdup(const char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str) + 1;
    char *new_str = secure_malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

/* Secure memory comparison (constant-time) */
int secure_memcmp(const void *a, const void *b, size_t len) {
    const unsigned char *pa = (const unsigned char*)a;
    const unsigned char *pb = (const unsigned char*)b;
    unsigned char result = 0;
    
    for (size_t i = 0; i < len; i++) {
        result |= pa[i] ^ pb[i];
    }
    
    return result;
}

/* Secure memory zeroing (guaranteed not to be optimized away) */
void secure_zero(void *ptr, size_t len) {
    if (!ptr || len == 0) return;
    
    volatile unsigned char *p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Lock memory to prevent swapping (mlock) */
int lock_memory(void *addr, size_t len) {
#ifdef _WIN32
    return VirtualLock(addr, len) ? 0 : -1;
#else
    return mlock(addr, len);
#endif
}

/* Unlock memory */
int unlock_memory(void *addr, size_t len) {
#ifdef _WIN32
    return VirtualUnlock(addr, len) ? 0 : -1;
#else
    return munlock(addr, len);
#endif
}

/* Allocate locked memory (prevents swapping to disk) */
void* allocate_locked_memory(size_t size) {
#ifdef _WIN32
    void *ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (ptr && lock_memory(ptr, size) != 0) {
        VirtualFree(ptr, 0, MEM_RELEASE);
        return NULL;
    }
    return ptr;
#else
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    
    if (lock_memory(ptr, size) != 0) {
        munmap(ptr, size);
        return NULL;
    }
    
    return ptr;
#endif
}

/* Free locked memory */
void free_locked_memory(void *ptr, size_t size) {
    if (!ptr) return;
    
    unlock_memory(ptr, size);
    
#ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
    munmap(ptr, size);
#endif
}

/* Memory pool for secure allocations */
memory_pool_t* create_memory_pool(size_t block_size, size_t num_blocks) {
    memory_pool_t *pool = malloc(sizeof(memory_pool_t));
    if (!pool) return NULL;
    
    pool->block_size = block_size;
    pool->num_blocks = num_blocks;
    pool->blocks = calloc(num_blocks, sizeof(void*));
    pool->in_use = calloc(num_blocks, sizeof(int));
    
    if (!pool->blocks || !pool->in_use) {
        free(pool->blocks);
        free(pool->in_use);
        free(pool);
        return NULL;
    }
    
    /* Allocate all blocks as locked memory */
    for (size_t i = 0; i < num_blocks; i++) {
        pool->blocks[i] = allocate_locked_memory(block_size);
        if (!pool->blocks[i]) {
            /* Cleanup on failure */
            for (size_t j = 0; j < i; j++) {
                free_locked_memory(pool->blocks[j], block_size);
            }
            free(pool->blocks);
            free(pool->in_use);
            free(pool);
            return NULL;
        }
    }
    
    return pool;
}

/* Allocate from memory pool */
void* pool_allocate(memory_pool_t *pool) {
    if (!pool) return NULL;
    
    for (size_t i = 0; i < pool->num_blocks; i++) {
        if (!pool->in_use[i]) {
            pool->in_use[i] = 1;
            memset(pool->blocks[i], 0, pool->block_size);
            return pool->blocks[i];
        }
    }
    
    return NULL; /* No free blocks */
}

/* Free to memory pool */
void pool_free(memory_pool_t *pool, void *ptr) {
    if (!pool || !ptr) return;
    
    for (size_t i = 0; i < pool->num_blocks; i++) {
        if (pool->blocks[i] == ptr && pool->in_use[i]) {
            secure_zero(ptr, pool->block_size);
            pool->in_use[i] = 0;
            return;
        }
    }
}

/* Destroy memory pool */
void destroy_memory_pool(memory_pool_t *pool) {
    if (!pool) return;
    
    for (size_t i = 0; i < pool->num_blocks; i++) {
        if (pool->blocks[i]) {
            secure_zero(pool->blocks[i], pool->block_size);
            free_locked_memory(pool->blocks[i], pool->block_size);
        }
    }
    
    free(pool->blocks);
    free(pool->in_use);
    free(pool);
}

/* Memory usage statistics */
memory_stats_t get_memory_stats(void) {
    memory_stats_t stats = {0};
    
#ifdef _WIN32
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        stats.total = mem.ullTotalPhys;
        stats.available = mem.ullAvailPhys;
        stats.used = mem.ullTotalPhys - mem.ullAvailPhys;
    }
    
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        stats.process_used = pmc.WorkingSetSize;
        stats.process_peak = pmc.PeakWorkingSetSize;
    }
#else
    /* Linux memory info */
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        char line[256];
        while (fgets(line, sizeof(line), meminfo)) {
            if (strstr(line, "MemTotal:")) {
                sscanf(line, "MemTotal: %lu kB", &stats.total);
                stats.total *= 1024;
            } else if (strstr(line, "MemAvailable:")) {
                sscanf(line, "MemAvailable: %lu kB", &stats.available);
                stats.available *= 1024;
            }
        }
        fclose(meminfo);
        stats.used = stats.total - stats.available;
    }
    
    /* Process memory info */
    FILE *status = fopen("/proc/self/status", "r");
    if (status) {
        char line[256];
        while (fgets(line, sizeof(line), status)) {
            if (strstr(line, "VmRSS:")) {
                sscanf(line, "VmRSS: %lu kB", &stats.process_used);
                stats.process_used *= 1024;
            } else if (strstr(line, "VmPeak:")) {
                sscanf(line, "VmPeak: %lu kB", &stats.process_peak);
                stats.process_peak *= 1024;
            }
        }
        fclose(status);
    }
#endif
    
    return stats;
}

/* Check for memory leaks (simple implementation) */
size_t check_memory_leaks(void) {
    /* In a real application, you would use a memory debugging library
       or implement a custom allocator with tracking */
    return 0;
}

/* Memory sanitization (for sensitive data) */
void sanitize_memory(void *ptr, size_t size) {
    if (!ptr || size == 0) return;
    
    /* Multiple overwrites for sensitive data */
    memset(ptr, 0xFF, size);        /* Fill with 1s */
    memset(ptr, 0x00, size);        /* Fill with 0s */
    memset(ptr, 0xAA, size);        /* Fill with alternating bits */
    memset(ptr, 0x55, size);        /* Fill with alternating bits */
    memset(ptr, 0x00, size);        /* Final zero fill */
    
    /* Force memory barrier */
#ifdef _WIN32
    MemoryBarrier();
#else
    __sync_synchronize();
#endif
}