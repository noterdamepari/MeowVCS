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

#pragma pack(push, 1)
typedef struct {
    uint64_t mtime;       // 8 bytes
    int path_len;         // 4 bytes
    int mode;             // 4 bytes
    file_status fstatus;  // 0 - modif, 1 - new, 2 - deleted 4 bytes
    stage_status sstatus; // 0 - staged, 1 - commited 4 bytes
    char hash[41];        // hash of file 41 bytes
} indexMeta;
#pragma pack(pop)

typedef struct {
    char name[255];
    char hash[41];
    char type[10];
    int mode;
} TreeEntry;

#endif
