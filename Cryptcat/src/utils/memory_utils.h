#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Memory pool structure */
typedef struct memory_pool_s memory_pool_t;

/* Memory statistics */
typedef struct {
    uint64_t total;
    uint64_t available;
    uint64_t used;
    uint64_t process_used;
    uint64_t process_peak;
} memory_stats_t;

/* Secure memory allocation */
void* secure_malloc(size_t size);
void* secure_calloc(size_t nmemb, size_t size);
void* secure_realloc(void *ptr, size_t old_size, size_t new_size);
void secure_free(void **ptr, size_t size);

/* Secure string duplication */
char* secure_strdup(const char *str);

/* Secure memory comparison (constant-time) */
int secure_memcmp(const void *a, const void *b, size_t len);

/* Secure memory zeroing */
void secure_zero(void *ptr, size_t len);

/* Lock memory to prevent swapping */
int lock_memory(void *addr, size_t len);
int unlock_memory(void *addr, size_t len);

/* Allocate locked memory */
void* allocate_locked_memory(size_t size);
void free_locked_memory(void *ptr, size_t size);

/* Memory pool functions */
memory_pool_t* create_memory_pool(size_t block_size, size_t num_blocks);
void* pool_allocate(memory_pool_t *pool);
void pool_free(memory_pool_t *pool, void *ptr);
void destroy_memory_pool(memory_pool_t *pool);

/* Memory statistics */
memory_stats_t get_memory_stats(void);

/* Check for memory leaks */
size_t check_memory_leaks(void);

/* Memory sanitization */
void sanitize_memory(void *ptr, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_UTILS_H */