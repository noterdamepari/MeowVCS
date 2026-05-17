#ifndef MEOW_H
#define MEOW_H

#include "misc.h"
#include "types.h"
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

extern const char* default_dir;
extern const char* objects_dir;
extern const char* meow_index;
int def(FILE* source, FILE* dest, int level);
int inf(FILE* source, FILE* dest);
#endif
