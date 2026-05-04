#ifndef MEOW_H
#define MEOW_H

    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include <assert.h>
    #include <time.h>
    #include "sha1.h"
    #include "zlib.h"
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <limits.h>

    #define CHUNK 16384 // 16kb
    #define PATH_MAX 4096


    extern const char* default_dir;
    extern const char* objects_dir;
    extern const char* meow_index;

    char is_path_absolute(char* path);
    char find_work_dir(char* buffer);
    void find_project_dir(char* buffer, char* work_dir);
    int make_path_relative(const char *root, const char *input, char *output);
    char create_object(char* path, char* work_dir, struct stat* st, char* hash);
    void get_object_path(char *dest, const char *work_dir, uint8_t *hash);
    int def(FILE *source, FILE *dest, int level);
    int inf(FILE *source, FILE *dest);

    typedef struct{
        char hash[41]; // hash of file
        uint8_t status; // 0 - modif, 1 - new, 2 - deleted
        uint64_t mtime;
        char path[PATH_MAX];
    } indexEntry;


#endif
