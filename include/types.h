#ifndef TYPES_H
#define TYPES_H

#include "time.h"
#include <limits.h>
#include <linux/limits.h>
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
    indexMeta meta;
    char path[PATH_MAX];
} IndexTreeEntry;

typedef struct {
    char name[255];
    char hash[41];
    char type[10];
    int mode;
} TreeEntry;

typedef struct {
    time_t time;
    char hash[41];
    char tree[41];
    char parent[41];
    char user[256];
    char email[256];
    char msg[1024];
} CommitEntry;

typedef struct {
    char work_dir[PATH_MAX];
    char project_dir[PATH_MAX];
} ProjectContext;

typedef struct {
    void (*delete)(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* entry);
    void (*modif)(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* src,
                  TreeEntry* target);
    void (*add)(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* entry);
    void (*type_change)(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* src,
                        TreeEntry* target);
} DiffCallbacks;

#endif
