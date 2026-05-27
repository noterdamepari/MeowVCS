#include "avl.h"
#include "misc.h"
#include "types.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

avlTree* open_index(char* work_dir, unsigned int* entries_amt) {
    char path_to_index[PATH_MAX];
    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    FILE* index = fopen_s(path_to_index, "rb");

    fread(entries_amt, sizeof(int), 1, index);

    avlTree* tree = NULL;

    // index to tree
    for (int i = 0; i < *entries_amt; i++) {
        IndexTreeEntry tree_entry;
        memset(&tree_entry, 0, sizeof(IndexTreeEntry));
        fread(&tree_entry.meta, sizeof(indexMeta), 1, index);
        fread(tree_entry.path, sizeof(char), tree_entry.meta.path_len, index);
        if (!tree) {
            tree = avl_create(&tree_entry);
        } else {
            avl_insert(&tree, &tree_entry);
        }
    }
    fclose(index);
    return tree;
}

void save_index(avlTree* idx, char* work_dir, int* entries_amt) {
    char path_to_index[PATH_MAX];
    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    FILE* index = fopen_s(path_to_index, "wb");

    fwrite(entries_amt, sizeof(int), 1, index);
    if (idx)
        avl_save_to_file(idx, index);
    fclose(index);
}

void close_index(avlTree* idx) {
    if (idx)
        avl_del_tree(idx);
}
