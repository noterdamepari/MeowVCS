#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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
