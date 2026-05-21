#include "types.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void clear_project_dir(char* work_dir, char* project_dir) {
    char path_to_index[PATH_MAX];
    char path_to_file[PATH_MAX];
    char* path_to_dir;
    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    FILE* index = fopen(path_to_index, "rb");
    if (!index) {
        fprintf(stderr, "Error: index not found in clear_project_dir func");
        exit(EXIT_FAILURE);
    }
    indexMeta entry;
    char entry_path[PATH_MAX];
    int entries_amt = 0;
    fread(&entries_amt, sizeof(int), 1, index);
    for (int i = 0; i < entries_amt; i++) {
        fread(&entry, sizeof(indexMeta), 1, index);
        fread(entry_path, sizeof(char), entry.path_len + 1, index);
        snprintf(path_to_file, PATH_MAX, "%s/%s", project_dir, entry_path);
        remove(path_to_file);
        path_to_dir = path_to_file;
        char* lash_slash = strrchr(path_to_dir, '/');
        while (lash_slash != NULL) {
            *lash_slash = '\0';
            if (!strcmp(path_to_dir, project_dir) || strlen(path_to_dir) <= strlen(work_dir))
                break;
            if (rmdir(path_to_dir) != 0)
                break;
            lash_slash = strrchr(path_to_dir, '/');
        }
    }
    fclose(index);
    FILE* new_index = fopen(path_to_index, "wb");
    if (!new_index) {
        fprintf(stderr, "Error: index not found in clear_project_dir func");
        exit(EXIT_FAILURE);
    }
    entries_amt = 0;
    fwrite(&entries_amt, sizeof(int), 1, new_index);
    fclose(new_index);
}

void add_to_index(indexMeta* entry, char* entry_path, char* work_dir) {
}
