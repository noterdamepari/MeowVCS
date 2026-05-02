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

    char is_path_absolute(char* path);
    char find_work_dir(char* buffer);
    char add_to_indexfile();
    int def(FILE *source, FILE *dest, int level);
    int inf(FILE *source, FILE *dest);
#endif
