#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration limits */
#define CONFIG_MAX_KEY_LEN 256
#define CONFIG_MAX_VALUE_LEN 1024
#define CONFIG_MAX_SECTION_LEN 256
#define CONFIG_MAX_LINE_LEN 2048
#define CONFIG_MAX_PATH 1024

/* Configuration error codes */
typedef enum {
    CONFIG_SUCCESS = 0,
    CONFIG_ERROR_PARAM = -1,
    CONFIG_ERROR_MEMORY = -2,
    CONFIG_ERROR_IO = -3,
    CONFIG_ERROR_NOTFOUND = -4,
    CONFIG_ERROR_SYNTAX = -5,
    CONFIG_ERROR_NOFILE = -6,
    CONFIG_ERROR_LOCK = -7
} config_error_t;

/* Configuration mutex type */
typedef void* config_mutex_t;

/* Configuration handle */
typedef struct config_s config_t;

/* Create/destroy configuration */
config_t* config_create(const char *filename);
void config_destroy(config_t *config);

/* Load/save configuration */
int config_load(config_t *config, const char *filename);
int config_save(config_t *config, const char *filename);

/* Get values */
const char* config_get_string(config_t *config, const char *section,
                             const char *key, const char *default_value);
int config_get_int(config_t *config, const char *section,
                  const char *key, int default_value);
int config_get_bool(config_t *config, const char *section,
                   const char *key, int default_value);
float config_get_float(config_t *config, const char *section,
                      const char *key, float default_value);

/* Set values */
int config_set_string(config_t *config, const char *section,
                     const char *key, const char *value);
int config_set_int(config_t *config, const char *section,
                  const char *key, int value);
int config_set_bool(config_t *config, const char *section,
                   const char *key, int value);
int config_set_float(config_t *config, const char *section,
                    const char *key, float value);

/* Delete keys/sections */
int config_delete_key(config_t *config, const char *section, const char *key);
int config_delete_section(config_t *config, const char *section);
void config_clear(config_t *config);

/* List sections/keys */
int config_get_sections(config_t *config, char ***sections, int *count);
int config_get_keys(config_t *config, const char *section,
                   char ***keys, int *count);

/* Configuration management */
int config_has_changed(config_t *config);
int config_reload_if_changed(config_t *config);
void config_set_auto_save(config_t *config, int auto_save);

/* Mutex functions */
config_mutex_t config_mutex_create(void);
void config_mutex_lock(config_mutex_t mutex);
void config_mutex_unlock(config_mutex_t mutex);
void config_mutex_destroy(config_mutex_t mutex);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_PARSER_H */