#ifndef MEOW_H
#define MEOW_H

#ifdef DEBUG
#define LOG(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#else
#define LOG(fmt, ...) // Ничего не делаем
#endif

#include "misc.h"
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define CHUNK 16384 // 16kb

extern const char* default_dir;
extern const char* objects_dir;
#endif
