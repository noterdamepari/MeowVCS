#ifndef TYPES_H
#define TYPES_H

#include <limits.h>
#include <stdint.h>

typedef enum {
    MODIFIED,
    NEW,
    DELETED
} status;

typedef struct {
    uint64_t mtime;
    char hash[41]; // hash of file
    status status; // 0 - modif, 1 - new, 2 - deleted
    char path[PATH_MAX];
} indexEntry;

typedef struct {
    char name[255];
    char hash[41];
    char dir;
} TreeEntry;

#endif
