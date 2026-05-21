#include "unpack.h"
#include "meow.h"
#include "object.h"
#include "types.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void unpack_blob(char* blob_src, char* file_dst, int mode) {
    FILE* tmp = tmpfile();
    FILE* blob = fopen(blob_src, "rb");
    if (!blob) {
        fprintf(stderr, "Error: Cannot open blob in unpack_blob func %s", blob_src);
        exit(EXIT_FAILURE);
    }
    FILE* f = fopen(file_dst, "wb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file in unpack_blob func %s", file_dst);
        exit(EXIT_FAILURE);
    }
    inf(blob, tmp);
    rewind(tmp);
    char type[10];
    size_t size;
    fscanf(tmp, "%s %ld", type, &size);
    if (strcmp(type, "blob")) {
        fprintf(stderr, "Error: Incorrect blob header");
    }
    fgetc(tmp);
    char buffer[CHUNK];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, sizeof(char), CHUNK, tmp)) > 0) {
        fwrite(buffer, sizeof(char), bytes_read, f);
    }
    fclose(tmp);
    fclose(blob);
    fclose(f);
    if (mode > 0)
        chmod(file_dst, mode & 0777);
}

void unpack_tree(char* tree_hash, char* path_to_tree, char* work_dir) {
    char path_to_obj[PATH_MAX];
    snprintf(path_to_obj, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, tree_hash, tree_hash + 2);

    FILE* tmp = tmpfile();
    FILE* tree = fopen(path_to_obj, "rb");
    if (!tree) {
        fprintf(stderr, "Error: required object %s not found", tree_hash);
        exit(EXIT_FAILURE);
    }
    inf(tree, tmp);
    rewind(tmp);

    TreeEntry entry;

    fscanf(tmp, "%*s %*ld");
    fgetc(tmp);

    while (fscanf(tmp, "%o %s %s %s", &entry.mode, entry.type, entry.hash, entry.name) == 4) {
        if (!strcmp(entry.type, "tree")) {
            char new_path_to_tree[PATH_MAX];
            snprintf(new_path_to_tree, PATH_MAX, "%s/%s", path_to_tree, entry.name);
            unpack_tree(entry.hash, new_path_to_tree, work_dir);
        } else if (!strcmp(entry.type, "blob")) {
            // TODO: Распакоука
            char path_to_blob[PATH_MAX];
            char path_to_file[PATH_MAX];
            snprintf(path_to_blob, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, entry.hash, entry.hash + 2);
            snprintf(path_to_file, PATH_MAX, "%s/%s", path_to_tree, entry.name);
            unpack_blob(path_to_blob, path_to_file, entry.mode);
        } else {
            fprintf(stderr, "Error: something went wrong with entry %s %s, tree %s", entry.type, entry.hash, tree_hash);
            exit(EXIT_FAILURE);
        }
    }
    fclose(tree);
    fclose(tmp);
}
