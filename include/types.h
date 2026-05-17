#ifndef TYPES_H
#define TYPES_H

#include <limits.h>
#include <stdint.h>

typedef enum {
    MODIFIED,
    NEW,
    DELETED
} file_status;

typedef enum {
    STAGED,
    COMMITED
} stage_status;

typedef struct {
    uint64_t mtime;
    int mode;
    char hash[41];        // hash of file
    file_status fstatus;  // 0 - modif, 1 - new, 2 - deleted
    stage_status sstatus; // 0 - staged, 1 - commited
    char path[PATH_MAX];
} indexEntry;

typedef struct {
    char name[255];
    char hash[41];
    char dir;
} TreeEntry;

#endif
