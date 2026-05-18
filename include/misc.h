#ifndef MISC_H
#define MISC_H
#include "types.h"
#include <stdint.h>
#include <sys/stat.h>
char is_path_absolute(char* path);
char find_work_dir(char* buffer);
void find_project_dir(char* buffer, char* work_dir);
int make_path_relative(const char* root, const char* input, char* output);
char create_blob(char* path, char* work_dir, struct stat* st, char* hash);
void get_object_path(char* dest, const char* work_dir, uint8_t* hash);
// void write_project_dir();
void write_tree(const indexMeta* entries, char** entries_path, const int entries_amt, int path_offset, char* ohash);
#endif
