#ifndef OBJECT_H
#define OBJECT_H
#include "stdio.h"
#include <sys/stat.h>
typedef enum {
    BLOB = 0,
    TREE
} object_type;

void hash_and_create_obj(object_type type, struct stat* st, FILE* f, char* ohash);
#endif
