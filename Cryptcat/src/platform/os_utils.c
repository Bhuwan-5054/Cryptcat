/*
 * OS Utilities - Common platform functions
 * Version: 1.0.0
 * License: MIT
 */

#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Get platform name */
const char* platform_get_name(void) {
#ifdef _WIN32
    return "Windows";
#elif __APPLE__
    return "macOS";
#elif __linux__
    return "Linux";
#elif __FreeBSD__
    return "FreeBSD";
#elif __OpenBSD__
    return "OpenBSD";
#elif __NetBSD__
    return "NetBSD";
#elif __DragonFly__
    return "DragonFly BSD";
#elif __sun
    return "Solaris";
#else
    return "Unknown";
#endif
}

/* Get platform architecture */
const char* platform_get_architecture(void) {
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    return "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "ARM64";
#elif defined(__arm__) || defined(_M_ARM)
    return "ARM";
#elif defined(__powerpc64__)
    return "PPC64";
#elif defined(__powerpc__)
    return "PPC";
#else
    return "Unknown";
#endif
}

/* Get current working directory */
int platform_get_cwd(char *buffer, size_t size) {
#ifdef _WIN32
    return GetCurrentDirectory(size, buffer) != 0;
#else
    return getcwd(buffer, size) != NULL;
#endif
}

/* Set current working directory */
int platform_set_cwd(const char *path) {
#ifdef _WIN32
    return SetCurrentDirectory(path) != 0;
#else
    return chdir(path) == 0;
#endif
}

/* Get environment variable */
const char* platform_get_env(const char *name) {
    return getenv(name);
}

/* Set environment variable */
int platform_set_env(const char *name, const char *value) {
#ifdef _WIN32
    return _putenv_s(name, value) == 0;
#else
    return setenv(name, value, 1) == 0;
#endif
}

/* Get temporary directory */
int platform_get_temp_dir(char *buffer, size_t size) {
#ifdef _WIN32
    DWORD len = GetTempPath(size, buffer);
    return len > 0 && len < size;
#else
    const char *tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";
    
    strncpy(buffer, tmpdir, size - 1);
    buffer[size - 1] = '\0';
    return 1;
#endif
}

/* Create temporary file */
FILE* platform_create_temp_file(char *template) {
#ifdef _WIN32
    char path[MAX_PATH];
    if (GetTempPath(MAX_PATH, path) == 0) {
        return NULL;
    }
    
    char filename[MAX_PATH];
    if (GetTempFileName(path, "cct", 0, filename) == 0) {
        return NULL;
    }
    
    if (template) {
        strncpy(template, filename, MAX_PATH - 1);
    }
    
    return fopen(filename, "w+b");
#else
    int fd = mkstemp(template);
    if (fd == -1) {
        return NULL;
    }
    
    return fdopen(fd, "w+b");
#endif
}

/* Get system information */
platform_system_info_t platform_get_system_info(void) {
    platform_system_info_t info = {0};
    
    /* Platform info */
    strncpy(info.platform_name, platform_get_name(), 
            sizeof(info.platform_name) - 1);
    strncpy(info.architecture, platform_get_architecture(),
            sizeof(info.architecture) - 1);
    
    /* Get memory info */
#ifdef _WIN32
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        info.total_memory = mem.ullTotalPhys;
        info.free_memory = mem.ullAvailPhys;
    }
    
    SYSTEM_INFO sys;
    GetSystemInfo(&sys);
    info.num_cpus = sys.dwNumberOfProcessors;
    
#else
    /* Linux/Unix memory info */
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        char line[256];
        while (fgets(line, sizeof(line), meminfo)) {
            if (strstr(line, "MemTotal:")) {
                sscanf(line, "MemTotal: %lu kB", &info.total_memory);
                info.total_memory *= 1024;
            } else if (strstr(line, "MemAvailable:")) {
                sscanf(line, "MemAvailable: %lu kB", &info.free_memory);
                info.free_memory *= 1024;
            }
        }
        fclose(meminfo);
    }
    
    info.num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    
    return info;
}

/* Execute command */
int platform_execute_command(const char *command, char *output, size_t output_size) {
#ifdef _WIN32
    FILE *pipe = _popen(command, "r");
#else
    FILE *pipe = popen(command, "r");
#endif
    
    if (!pipe) {
        return PLATFORM_ERROR_IO;
    }
    
    size_t total_read = 0;
    char buffer[256];
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL && 
           total_read < output_size - 1) {
        size_t len = strlen(buffer);
        if (total_read + len < output_size) {
            strcpy(output + total_read, buffer);
            total_read += len;
        } else {
            break;
        }
    }
    
    output[total_read] = '\0';
    
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    
    return PLATFORM_SUCCESS;
}

/* Check if running with administrator/root privileges */
int platform_is_elevated(void) {
#ifdef _WIN32
    BOOL is_elevated = FALSE;
    HANDLE token = NULL;
    
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD size = sizeof(TOKEN_ELEVATION);
        
        if (GetTokenInformation(token, TokenElevation, &elevation, 
                               sizeof(elevation), &size)) {
            is_elevated = elevation.TokenIsElevated;
        }
        
        CloseHandle(token);
    }
    
    return is_elevated;
#else
    return geteuid() == 0;
#endif
}