#include "avl.h"
#include "meow.h"
#include "misc.h"
#include "types.h"
#include "zlib.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void meow_add(char* file) {
    char work_dir[PATH_MAX];
    char path_to_index[PATH_MAX];
    char path_to_indextmp[PATH_MAX];
    char path[PATH_MAX];

    if (find_work_dir(work_dir) == -1) {
        fprintf(stderr, "Error: .meow/ not found\n");
        return;
    }

    if (!is_path_absolute(file)) {
        getcwd(path, PATH_MAX);
        strcat(path, "/");
        strcat(path, file);
    } else {
        strcpy(path, file);
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        fprintf(stderr, "Error: File doesn`t exists\n");
        exit(EXIT_FAILURE);
    }

    int64_t file_mtime = st.st_mtime;
    int file_mode = st.st_mode;

    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    snprintf(path_to_indextmp, PATH_MAX, "%s/index.tmp", work_dir);

    FILE* index = fopen(path_to_index, "rb");
    if (!index) {
        perror("Error: index not found");
        exit(EXIT_FAILURE);
    }

    FILE* index_tmp = fopen(path_to_indextmp, "wb");
    if (!index_tmp) {
        perror("Error: index.tmp not created");
        exit(EXIT_FAILURE);
    }

    unsigned int entries_amt = 0;
    fwrite(&entries_amt, sizeof(int), 1, index_tmp);
    fread(&entries_amt, sizeof(int), 1, index);

    char project_dir[PATH_MAX];
    char rel_path[PATH_MAX];
    find_project_dir(project_dir, work_dir);
    make_path_relative(project_dir, path, rel_path);

    avlTree* tree = NULL;
    char inserted = 0;

    // index to tree
    for (int i = 0; i < entries_amt; i++) {
        IndexTreeEntry tree_entry;
        memset(&tree_entry, 0, sizeof(IndexTreeEntry));
        fread(&tree_entry.meta, sizeof(indexMeta), 1, index);
        fread(tree_entry.path, sizeof(char), tree_entry.meta.path_len, index);
        if (!tree) {
            tree = avl_create(tree_entry);
        } else {
            avl_insert(&tree, &tree_entry);
        }
    }

    // new entry
    char hash[41];
    IndexTreeEntry* entry = avl_find(tree, rel_path);
    if (entry) {
        inserted = 1;
        if (entry->meta.mtime != file_mtime) {
            create_blob(path, work_dir, &st, hash);
            entry->meta.fstatus = MODIFIED;
            entry->meta.sstatus = STAGED;
            entry->meta.mtime = file_mtime;
            entry->meta.mode = file_mode;
            strcpy(entry->meta.hash, hash);
        }
    }

    if (!inserted) {
        IndexTreeEntry new_entry;
        memset(&new_entry, 0, sizeof(IndexTreeEntry));
        create_blob(path, work_dir, &st, hash);
        strcpy(new_entry.path, rel_path);
        strcpy(new_entry.meta.hash, hash);
        new_entry.meta.path_len = strlen(rel_path) + 1;
        new_entry.meta.fstatus = NEW;
        new_entry.meta.sstatus = STAGED;
        new_entry.meta.mode = file_mode;
        new_entry.meta.mtime = file_mtime;
        avl_insert(&tree, &new_entry);
        entries_amt++;
    }

    avl_save_to_file(tree, index_tmp);

    avl_del_tree(tree);

    printf("%s added to index with %o mode", rel_path, file_mode);

    rewind(index_tmp);
    fwrite(&entries_amt, sizeof(int), 1, index_tmp);

    fclose(index_tmp);
    fclose(index);

    if (rename(path_to_indextmp, path_to_index) != 0) {
        perror("Error: rename failed");
        exit(EXIT_FAILURE);
        return;
    }
}
