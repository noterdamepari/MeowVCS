#ifndef MEOW_H
#define MEOW_H

#include "object.h"
#include "sha1.h"
#include "types.h"
#include "zlib.h"
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define CHUNK 16384 // 16kb

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

extern const char* default_dir;
extern const char* objects_dir;
extern const char* meow_index;

typedef struct avlTree_t {
    TreeEntry value;
    struct avlTree_t* child[2]; // child0 - left, child1 - right
    int32_t height;
} avlTree;

avlTree* avl_create(TreeEntry value);
char avl_insert(avlTree** tree, TreeEntry value);
char avl_erase(avlTree** tree, TreeEntry value);
void avl_traverse(avlTree* tree);
void avl_del_tree(avlTree* tree);

char is_path_absolute(char* path);
char find_work_dir(char* buffer);
void find_project_dir(char* buffer, char* work_dir);
int make_path_relative(const char* root, const char* input, char* output);
char create_blob(char* path, char* work_dir, struct stat* st, char* hash);
void get_object_path(char* dest, const char* work_dir, uint8_t* hash);
int def(FILE* source, FILE* dest, int level);
int inf(FILE* source, FILE* dest);
void write_project_dir();
void write_tree(const indexEntry* entries, const int entries_amt, int path_offset, char* ohash);
#endif
