/*
 * Configuration Parser
 * Version: 1.0.0
 * License: MIT
 */

#include "config_parser.h"
#include "logger.h"
#include "error_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Configuration entry */
typedef struct config_entry_s {
    char key[CONFIG_MAX_KEY_LEN];
    char value[CONFIG_MAX_VALUE_LEN];
    struct config_entry_s *next;
} config_entry_t;

/* Configuration section */
typedef struct config_section_s {
    char name[CONFIG_MAX_SECTION_LEN];
    config_entry_t *entries;
    struct config_section_s *next;
} config_section_t;

/* Configuration structure */
struct config_s {
    config_section_t *sections;
    char filename[CONFIG_MAX_PATH];
    time_t last_modified;
    int auto_save;
    config_mutex_t mutex;
};

/* Internal functions */
static config_section_t* find_section(config_t *config, const char *section);
static config_entry_t* find_entry(config_section_t *section, const char *key);
static int parse_line(char *line, char *section, char *key, char *value);
static void trim_string(char *str);
static int save_config_to_file(config_t *config, const char *filename);

/* Create new configuration */
config_t* config_create(const char *filename) {
    config_t *config = malloc(sizeof(config_t));
    if (!config) {
        ERROR_HANDLE(ERROR_MEMORY, "config_create");
        return NULL;
    }
    
    memset(config, 0, sizeof(config_t));
    
    if (filename) {
        strncpy(config->filename, filename, CONFIG_MAX_PATH - 1);
    }
    
    config->auto_save = 1;
    config->mutex = config_mutex_create();
    
    LOG_DEBUG("Configuration created");
    return config;
}

/* Load configuration from file */
int config_load(config_t *config, const char *filename) {
    FILE *file;
    char line[CONFIG_MAX_LINE_LEN];
    char current_section[CONFIG_MAX_SECTION_LEN] = "";
    int line_num = 0;
    
    if (!config) {
        return CONFIG_ERROR_PARAM;
    }
    
    config_mutex_lock(config->mutex);
    
    if (filename) {
        strncpy(config->filename, filename, CONFIG_MAX_PATH - 1);
    }
    
    if (config->filename[0] == '\0') {
        config_mutex_unlock(config->mutex);
        return CONFIG_ERROR_NOFILE;
    }
    
    file = fopen(config->filename, "r");
    if (!file) {
        LOG_WARNING("Configuration file not found: %s", config->filename);
        config_mutex_unlock(config->mutex);
        return CONFIG_ERROR_NOFILE;
    }
    
    /* Clear existing configuration */
    config_clear(config);
    
    /* Parse file */
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        /* Remove newline */
        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';
        
        /* Skip empty lines and comments */
        trim_string(line);
        if (line[0] == '\0' || line[0] == ';' || line[0] == '#') {
            continue;
        }
        
        char section[CONFIG_MAX_SECTION_LEN] = "";
        char key[CONFIG_MAX_KEY_LEN] = "";
        char value[CONFIG_MAX_VALUE_LEN] = "";
        
        if (parse_line(line, section, key, value)) {
            if (section[0] != '\0') {
                /* New section */
                strncpy(current_section, section, sizeof(current_section) - 1);
            } else if (key[0] != '\0') {
                /* Key-value pair */
                if (current_section[0] == '\0') {
                    LOG_WARNING("Key without section at line %d: %s", line_num, line);
                    continue;
                }
                
                config_set(config, current_section, key, value);
            }
        } else {
            LOG_WARNING("Invalid configuration line %d: %s", line_num, line);
        }
    }
    
    fclose(file);
    
    /* Get file modification time */
    struct stat st;
    if (stat(config->filename, &st) == 0) {
        config->last_modified = st.st_mtime;
    }
    
    config_mutex_unlock(config->mutex);
    
    LOG_INFO("Configuration loaded from %s (%d lines)", 
             config->filename, line_num);
    return CONFIG_SUCCESS;
}

/* Save configuration to file */
int config_save(config_t *config, const char *filename) {
    if (!config) {
        return CONFIG_ERROR_PARAM;
    }
    
    config_mutex_lock(config->mutex);
    
    const char *save_filename = filename ? filename : config->filename;
    if (save_filename[0] == '\0') {
        config_mutex_unlock(config->mutex);
        return CONFIG_ERROR_NOFILE;
    }
    
    int result = save_config_to_file(config, save_filename);
    
    if (result == CONFIG_SUCCESS) {
        struct stat st;
        if (stat(save_filename, &st) == 0) {
            config->last_modified = st.st_mtime;
        }
        
        if (!filename) {
            LOG_INFO("Configuration saved to %s", config->filename);
        } else {
            LOG_INFO("Configuration saved to %s", filename);
        }
    }
    
    config_mutex_unlock(config->mutex);
    return result;
}

/* Get string value */
const char* config_get_string(config_t *config, const char *section,
                             const char *key, const char *default_value) {
    if (!config || !section || !key) {
        return default_value;
    }
    
    config_mutex_lock(config->mutex);
    
    config_section_t *sect = find_section(config, section);
    if (!sect) {
        config_mutex_unlock(config->mutex);
        return default_value;
    }
    
    config_entry_t *entry = find_entry(sect, key);
    const char *result = entry ? entry->value : default_value;
    
    config_mutex_unlock(config->mutex);
    return result;
}

/* Get integer value */
int config_get_int(config_t *config, const char *section,
                  const char *key, int default_value) {
    const char *str = config_get_string(config, section, key, NULL);
    if (!str) {
        return default_value;
    }
    
    char *endptr;
    long value = strtol(str, &endptr, 10);
    
    if (endptr == str || *endptr != '\0') {
        return default_value;
    }
    
    return (int)value;
}

/* Get boolean value */
int config_get_bool(config_t *config, const char *section,
                   const char *key, int default_value) {
    const char *str = config_get_string(config, section, key, NULL);
    if (!str) {
        return default_value;
    }
    
    /* Check for true values */
    if (strcasecmp(str, "true") == 0 ||
        strcasecmp(str, "yes") == 0 ||
        strcasecmp(str, "on") == 0 ||
        strcasecmp(str, "1") == 0) {
        return 1;
    }
    
    /* Check for false values */
    if (strcasecmp(str, "false") == 0 ||
        strcasecmp(str, "no") == 0 ||
        strcasecmp(str, "off") == 0 ||
        strcasecmp(str, "0") == 0) {
        return 0;
    }
    
    return default_value;
}

/* Get float value */
float config_get_float(config_t *config, const char *section,
                      const char *key, float default_value) {
    const char *str = config_get_string(config, section, key, NULL);
    if (!str) {
        return default_value;
    }
    
    char *endptr;
    float value = strtof(str, &endptr);
    
    if (endptr == str || *endptr != '\0') {
        return default_value;
    }
    
    return value;
}

/* Set string value */
int config_set_string(config_t *config, const char *section,
                     const char *key, const char *value) {
    if (!config || !section || !key || !value) {
        return CONFIG_ERROR_PARAM;
    }
    
    config_mutex_lock(config->mutex);
    
    /* Find or create section */
    config_section_t *sect = find_section(config, section);
    if (!sect) {
        sect = malloc(sizeof(config_section_t));
        if (!sect) {
            config_mutex_unlock(config->mutex);
            ERROR_HANDLE(ERROR_MEMORY, "config_set_string");
            return CONFIG_ERROR_MEMORY;
        }
        
        memset(sect, 0, sizeof(config_section_t));
        strncpy(sect->name, section, CONFIG_MAX_SECTION_LEN - 1);
        sect->next = config->sections;
        config->sections = sect;
    }
    
    /* Find or create entry */
    config_entry_t *entry = find_entry(sect, key);
    if (!entry) {
        entry = malloc(sizeof(config_entry_t));
        if (!entry) {
            config_mutex_unlock(config->mutex);
            ERROR_HANDLE(ERROR_MEMORY, "config_set_string");
            return CONFIG_ERROR_MEMORY;
        }
        
        memset(entry, 0, sizeof(config_entry_t));
        strncpy(entry->key, key, CONFIG_MAX_KEY_LEN - 1);
        entry->next = sect->entries;
        sect->entries = entry;
    }
    
    /* Set value */
    strncpy(entry->value, value, CONFIG_MAX_VALUE_LEN - 1);
    
    /* Auto-save if enabled */
    if (config->auto_save && config->filename[0] != '\0') {
        save_config_to_file(config, config->filename);
    }
    
    config_mutex_unlock(config->mutex);
    
    LOG_DEBUG("Configuration set: [%s] %s = %s", section, key, value);
    return CONFIG_SUCCESS;
}

/* Set integer value */
int config_set_int(config_t *config, const char *section,
                  const char *key, int value) {
    char str[32];
    snprintf(str, sizeof(str), "%d", value);
    return config_set_string(config, section, key, str);
}

/* Set boolean value */
int config_set_bool(config_t *config, const char *section,
                   const char *key, int value) {
    return config_set_string(config, section, key, value ? "true" : "false");
}

/* Set float value */
int config_set_float(config_t *config, const char *section,
                    const char *key, float value) {
    char str[32];
    snprintf(str, sizeof(str), "%.6f", value);
    return config_set_string(config, section, key, str);
}

/* Delete key */
int config_delete_key(config_t *config, const char *section, const char *key) {
    if (!config || !section || !key) {
        return CONFIG_ERROR_PARAM;
    }
    
    config_mutex_lock(config->mutex);
    
    config_section_t *sect = find_section(config, section);
    if (!sect) {
        config_mutex_unlock(config->mutex);
        return CONFIG_ERROR_NOTFOUND;
    }
    
    config_entry_t **prev = &sect->entries;
    config_entry_t *current = sect->entries;
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            *prev = current->next;
            free(current);
            
            /* Auto-save if enabled */
            if (config->auto_save && config->filename[0] != '\0') {
                save_config_to_file(config, config->filename);
            }
            
            config_mutex_unlock(config->mutex);
            LOG_DEBUG("Configuration deleted: [%s] %s", section, key);
            return CONFIG_SUCCESS;
        }
        
        prev = &current->next;
        current = current->next;
    }
    
    config_mutex_unlock(config->mutex);
    return CONFIG_ERROR_NOTFOUND;
}

/* Delete section */
int config_delete_section(config_t *config, const char *section) {
    if (!config || !section) {
        return CONFIG_ERROR_PARAM;
    }
    
    config_mutex_lock(config->mutex);
    
    config_section_t **prev = &config->sections;
    config_section_t *current = config->sections;
    
    while (current) {
        if (strcmp(current->name, section) == 0) {
            /* Free all entries in section */
            config_entry_t *entry = current->entries;
            while (entry) {
                config_entry_t *next = entry->next;
                free(entry);
                entry = next;
            }
            
            /* Remove section */
            *prev = current->next;
            free(current);
            
            /* Auto-save if enabled */
            if (config->auto_save && config->filename[0] != '\0') {
                save_config_to_file(config, config->filename);
            }
            
            config_mutex_unlock(config->mutex);
            LOG_DEBUG("Configuration section deleted: [%s]", section);
            return CONFIG_SUCCESS;
        }
        
        prev = &current->next;
        current = current->next;
    }
    
    config_mutex_unlock(config->mutex);
    return CONFIG_ERROR_NOTFOUND;
}

/* Clear all configuration */
void config_clear(config_t *config) {
    if (!config) return;
    
    config_mutex_lock(config->mutex);
    
    config_section_t *sect = config->sections;
    while (sect) {
        config_section_t *next_sect = sect->next;
        
        config_entry_t *entry = sect->entries;
        while (entry) {
            config_entry_t *next_entry = entry->next;
            free(entry);
            entry = next_entry;
        }
        
        free(sect);
        sect = next_sect;
    }
    
    config->sections = NULL;
    config_mutex_unlock(config->mutex);
    
    LOG_DEBUG("Configuration cleared");
}

/* Get all sections */
int config_get_sections(config_t *config, char ***sections, int *count) {
    if (!config || !sections || !count) {
        return CONFIG_ERROR_PARAM;
    }
    
    config_mutex_lock(config->mutex);
    
    /* Count sections */
    int section_count = 0;
    config_section_t *sect = config->sections;
    while (sect) {
        section_count++;
        sect = sect->next;
    }
    
    /* Allocate array */
    char **array = malloc(section_count * sizeof(char*));
    if (!array) {
        config_mutex_unlock(config->mutex);
        ERROR_HANDLE(ERROR_MEMORY, "config_get_sections");
        return CONFIG_ERROR_MEMORY;
    }
    
    /* Fill array */
    sect = config->sections;
    for (int i = 0; i < section_count && sect; i++) {
        array[i] = strdup(sect->name);
        if (!array[i]) {
            /* Cleanup on error */
            for (int j = 0; j < i; j++) {
                free(array[j]);
            }
            free(array);
            config_mutex_unlock(config->mutex);
            ERROR_HANDLE(ERROR_MEMORY, "config_get_sections");
            return CONFIG_ERROR_MEMORY;
        }
        sect = sect->next;
    }
    
    *sections = array;
    *count = section_count;
    
    config_mutex_unlock(config->mutex);
    return CONFIG_SUCCESS;
}

/* Get all keys in section */
int config_get_keys(config_t *config, const char *section,
                   char ***keys, int *count) {
    if (!config || !section || !keys || !count) {
        return CONFIG_ERROR_PARAM;
    }
    
    config_mutex_lock(config->mutex);
    
    config_section_t *sect = find_section(config, section);
    if (!sect) {
        config_mutex_unlock(config->mutex);
        return CONFIG_ERROR_NOTFOUND;
    }
    
    /* Count keys */
    int key_count = 0;
    config_entry_t *entry = sect->entries;
    while (entry) {
        key_count++;
        entry = entry->next;
    }
    
    /* Allocate array */
    char **array = malloc(key_count * sizeof(char*));
    if (!array) {
        config_mutex_unlock(config->mutex);
        ERROR_HANDLE(ERROR_MEMORY, "config_get_keys");
        return CONFIG_ERROR_MEMORY;
    }
    
    /* Fill array */
    entry = sect->entries;
    for (int i = 0; i < key_count && entry; i++) {
        array[i] = strdup(entry->key);
        if (!array[i]) {
            /* Cleanup on error */
            for (int j = 0; j < i; j++) {
                free(array[j]);
            }
            free(array);
            config_mutex_unlock(config->mutex);
            ERROR_HANDLE(ERROR_MEMORY, "config_get_keys");
            return CONFIG_ERROR_MEMORY;
        }
        entry = entry->next;
    }
    
    *keys = array;
    *count = key_count;
    
    config_mutex_unlock(config->mutex);
    return CONFIG_SUCCESS;
}

/* Check if configuration has changed on disk */
int config_has_changed(config_t *config) {
    if (!config || config->filename[0] == '\0') {
        return 0;
    }
    
    struct stat st;
    if (stat(config->filename, &st) != 0) {
        return 0;
    }
    
    return st.st_mtime > config->last_modified;
}

/* Reload if changed */
int config_reload_if_changed(config_t *config) {
    if (config_has_changed(config)) {
        LOG_INFO("Configuration file changed, reloading");
        return config_load(config, NULL);
    }
    return CONFIG_SUCCESS;
}

/* Set auto-save mode */
void config_set_auto_save(config_t *config, int auto_save) {
    if (config) {
        config->auto_save = auto_save;
    }
}

/* Destroy configuration */
void config_destroy(config_t *config) {
    if (!config) return;
    
    config_clear(config);
    config_mutex_destroy(config->mutex);
    free(config);
    
    LOG_DEBUG("Configuration destroyed");
}

/* Internal: Find section */
static config_section_t* find_section(config_t *config, const char *section) {
    config_section_t *sect = config->sections;
    while (sect) {
        if (strcmp(sect->name, section) == 0) {
            return sect;
        }
        sect = sect->next;
    }
    return NULL;
}

/* Internal: Find entry */
static config_entry_t* find_entry(config_section_t *section, const char *key) {
    config_entry_t *entry = section->entries;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

/* Internal: Parse line */
static int parse_line(char *line, char *section, char *key, char *value) {
    char *ptr = line;
    
    /* Skip whitespace */
    while (*ptr && isspace(*ptr)) ptr++;
    
    if (*ptr == '\0') return 0;
    
    /* Check for section */
    if (*ptr == '[') {
        ptr++;
        char *end = strchr(ptr, ']');
        if (!end) return 0;
        
        *end = '\0';
        trim_string(ptr);
        strncpy(section, ptr, CONFIG_MAX_SECTION_LEN - 1);
        return 1;
    }
    
    /* Parse key-value pair */
    char *separator = strchr(ptr, '=');
    if (!separator) return 0;
    
    *separator = '\0';
    char *key_start = ptr;
    char *value_start = separator + 1;
    
    /* Trim key and value */
    trim_string(key_start);
    trim_string(value_start);
    
    /* Remove quotes from value if present */
    if (value_start[0] == '"' && value_start[strlen(value_start) - 1] == '"') {
        value_start[strlen(value_start) - 1] = '\0';
        value_start++;
    }
    
    strncpy(key, key_start, CONFIG_MAX_KEY_LEN - 1);
    strncpy(value, value_start, CONFIG_MAX_VALUE_LEN - 1);
    
    return 1;
}

/* Internal: Trim string */
static void trim_string(char *str) {
    char *end;
    
    /* Trim leading space */
    while (isspace(*str)) str++;
    
    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    
    *(end + 1) = '\0';
}

/* Internal: Save configuration to file */
static int save_config_to_file(config_t *config, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        ERROR_HANDLE(ERROR_IO, "save_config_to_file");
        return CONFIG_ERROR_IO;
    }
    
    fprintf(file, "; Cryptcat Configuration\n");
    fprintf(file, "; Generated on %s\n\n", __DATE__);
    
    config_section_t *sect = config->sections;
    while (sect) {
        fprintf(file, "[%s]\n", sect->name);
        
        config_entry_t *entry = sect->entries;
        while (entry) {
            fprintf(file, "%s = %s\n", entry->key, entry->value);
            entry = entry->next;
        }
        
        fprintf(file, "\n");
        sect = sect->next;
    }
    
    fclose(file);
    return CONFIG_SUCCESS;
}