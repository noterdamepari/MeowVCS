#include "avl.h"
#include "misc.h"
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

static void write_tree_to_index_rec(char* target_hash, char* prefix, ProjectContext* p_ctx,
                                    avlTree** index_tree, int* entries_amt) {
    char path_to_tree[PATH_MAX];
    snprintf(path_to_tree, PATH_MAX, "%s/objects/%.2s/%s", p_ctx->work_dir, target_hash,
             target_hash + 2);

    FILE* tree_inflated = fopen_inflated(path_to_tree);
    fscanf(tree_inflated, "%*s %*ld");
    fgetc(tree_inflated);

    TreeEntry entry;
    IndexTreeEntry new_index_entry;

    struct stat st;
    while (fscanf(tree_inflated, "%o %s %40s %s", &entry.mode, entry.type, entry.hash,
                  entry.name) == 4) {
        char new_prefix[PATH_MAX];
        snprintf(new_prefix, PATH_MAX, "%s/%s", prefix, entry.name);
        if (!strcmp(entry.type, "tree")) {
            write_tree_to_index_rec(entry.hash, new_prefix, p_ctx, index_tree, entries_amt);
        } else {
            char rel_path[PATH_MAX];
            make_path_relative(p_ctx->project_dir, new_prefix, rel_path);
            stat(prefix, &st);
            strcpy(new_index_entry.meta.hash, entry.hash);
            strcpy(new_index_entry.path, rel_path);
            new_index_entry.meta.path_len = strlen(rel_path) + 1;
            new_index_entry.meta.mtime = st.st_mtime;
            new_index_entry.meta.fstatus = 0;
            new_index_entry.meta.sstatus = 1;
            new_index_entry.meta.mode = entry.mode;
            if (!*index_tree) {
                *index_tree = avl_create(&new_index_entry);
                (*entries_amt)++;
            } else {
                avl_insert(index_tree, &new_index_entry);
                (*entries_amt)++;
            }
        }
    }
}

void update_index_from_tree(char* target_hash, ProjectContext* p_ctx) {
    char path_to_index[PATH_MAX];
    snprintf(path_to_index, PATH_MAX, "%s/index", p_ctx->work_dir);
    FILE* index = fopen_s(path_to_index, "wb");
    avlTree* tree = NULL;

    int entries_amt = 0;
    fwrite(&entries_amt, sizeof(int), 1, index);
    write_tree_to_index_rec(target_hash, p_ctx->project_dir, p_ctx, &tree, &entries_amt);
    avl_save_to_file(tree, index);
    rewind(index);
    fwrite(&entries_amt, sizeof(int), 1, index);
    avl_del_tree(tree);
    fclose(index);
}

void add_to_index(indexMeta* entry, char* entry_path, char* work_dir) {
}
