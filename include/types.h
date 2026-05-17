#ifndef TYPES_H
#define TYPES_H

#include <limits.h>
#include <stdint.h>

typedef struct {
    uint64_t mtime;
    char hash[41];  // hash of file
    uint8_t status; // 0 - modif, 1 - new, 2 - deleted
    char path[PATH_MAX];
} indexEntry;

typedef struct {
    char name[255];
    char hash[41];
    char dir;
} TreeEntry;

#endif
