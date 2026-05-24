#ifndef OBJECT_H
#define OBJECT_H
#include "stdio.h"
#include <sys/stat.h>

typedef enum {
    BLOB,
    TREE,
    COMMIT
} object_type;

int def(FILE* source, FILE* dest, int level);
int inf(FILE* source, FILE* dest);
void hash_and_create_obj(object_type type, FILE* f, char* ohash);
int object_exists(const char* hash);
char create_blob(char* path, char* work_dir, struct stat* st, char* ohash);
#endif
