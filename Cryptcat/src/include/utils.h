#ifndef UTILS_H
#define UTILS_H

/* This is a master header that includes all utility headers */

#include "logger.h"
#include "error_handler.h"
#include "memory_utils.h"
#include "config_parser.h"
#include "string_utils.h"
#include "hex_utils.h"

/* Utility macros */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Byte order macros */
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <endian.h>
#define htobe16(x) htons(x)
#define htobe32(x) htonl(x)
#define htobe64(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define be16toh(x) ntohs(x)
#define be32toh(x) ntohl(x)
#define be64toh(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#elif defined(_WIN32)
#include <winsock2.h>
#define htobe16(x) htons(x)
#define htobe32(x) htonl(x)
#define htobe64(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define be16toh(x) ntohs(x)
#define be32toh(x) ntohl(x)
#define be64toh(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

#endif /* UTILS_H */