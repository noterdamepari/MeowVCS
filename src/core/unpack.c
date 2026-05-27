#include "unpack.h"
#include "meow.h"
#include "misc.h"
#include "object.h"
#include "types.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void unpack_blob(char* blob_src, char* file_dst, int mode) {
    FILE* inflated_blob = fopen_inflated(blob_src);
    FILE* f = fopen_s(file_dst, "wb");

    char type[10];
    size_t size;
    fscanf(inflated_blob, "%s %ld", type, &size);
    if (strcmp(type, "blob")) {
        fprintf(stderr, "Error: Incorrect blob header");
        exit(EXIT_FAILURE);
    }
    fgetc(inflated_blob);
    char buffer[CHUNK];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, sizeof(char), CHUNK, inflated_blob)) > 0) {
        fwrite(buffer, sizeof(char), bytes_read, f);
    }
    fclose(inflated_blob);
    fclose(f);
    if (mode > 0)
        chmod(file_dst, mode & 0777);
}

void unpack_tree(char* tree_hash, char* path_to_tree, char* work_dir) {
    char path_to_obj[PATH_MAX];
    snprintf(path_to_obj, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, tree_hash,
             tree_hash + 2);

    FILE* inflated_tree = tmpfile();
    FILE* tree = fopen(path_to_obj, "rb");
    if (!tree) {
        fprintf(stderr, "Error: required object %s not found", tree_hash);
        exit(EXIT_FAILURE);
    }
    inf(tree, inflated_tree);
    rewind(inflated_tree);

    TreeEntry entry;

    fscanf(inflated_tree, "%*s %*ld");
    fgetc(inflated_tree);

    while (fscanf(inflated_tree, "%o %s %s %s", &entry.mode, entry.type, entry.hash, entry.name) ==
           4) {
        if (!strcmp(entry.type, "tree")) {
            char new_path_to_tree[PATH_MAX];
            snprintf(new_path_to_tree, PATH_MAX, "%s/%s", path_to_tree, entry.name);
            mkdir(new_path_to_tree, 0777);
            unpack_tree(entry.hash, new_path_to_tree, work_dir);
        } else if (!strcmp(entry.type, "blob")) {
            char path_to_blob[PATH_MAX];
            char path_to_file[PATH_MAX];
            snprintf(path_to_blob, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, entry.hash,
                     entry.hash + 2);
            snprintf(path_to_file, PATH_MAX, "%s/%s", path_to_tree, entry.name);
            unpack_blob(path_to_blob, path_to_file, entry.mode);
        } else {
            fprintf(stderr, "Error: something went wrong with entry %s %s, tree %s", entry.type,
                    entry.hash, tree_hash);
            exit(EXIT_FAILURE);
        }
    }
    fclose(tree);
    fclose(inflated_tree);
}
