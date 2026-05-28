#include "avl.h"
#include "index.h"
#include "misc.h"
#include "types.h"
#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void add_to_tree_rec(char* path, avlTree** tree, unsigned int* entries_amt, char* work_dir,
                            char* project_dir, stage_status status) {

    char rel_path[PATH_MAX];
    make_path_relative(project_dir, path, rel_path);
    if (!strncmp(rel_path, ".meow", 5)) {
        return;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        fprintf(stderr, "Warning: File '%s' was deleted or is inaccessible during execution\n",
                rel_path);
        return;
    }

    // dir
    if (S_ISDIR(st.st_mode)) {
        DIR* d = opendir_s(path);
        struct dirent* ent;
        while ((ent = readdir(d)) != NULL) {
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") ||
                !strcmp(ent->d_name, ".meow"))
                continue;
            char sub_path[PATH_MAX];
            snprintf(sub_path, PATH_MAX, "%s/%s", path, ent->d_name);

            add_to_tree_rec(sub_path, tree, entries_amt, work_dir, project_dir, status);
        }
        closedir(d);
        // file
    } else if (S_ISREG(st.st_mode)) {
        int exists = 0;

        // new entry
        char hash[41];
        IndexTreeEntry* entry = avl_find(*tree, rel_path);
        int64_t file_mtime = st.st_mtime;
        int file_mode = st.st_mode;
        if (entry) {
            exists = 1;
            if (entry->meta.mtime != file_mtime) {
                create_blob(path, work_dir, &st, hash);
                entry->meta.fstatus = MODIFIED;
                entry->meta.sstatus = status;
                entry->meta.mtime = file_mtime;
                entry->meta.mode = file_mode;
                strcpy(entry->meta.hash, hash);
                if (status == STAGED)
                    printf("%s added to index with %o mode\n", rel_path, file_mode);
            } else {
                if (status == STAGED)
                    printf("%s already in index, nothing to do\n", rel_path);
            }
        }

        if (!exists) {
            exists = 1;
            IndexTreeEntry new_entry;
            memset(&new_entry, 0, sizeof(IndexTreeEntry));
            create_blob(path, work_dir, &st, hash);
            strcpy(new_entry.path, rel_path);
            strcpy(new_entry.meta.hash, hash);
            new_entry.meta.path_len = strlen(rel_path) + 1;
            new_entry.meta.fstatus = NEW;
            new_entry.meta.sstatus = status;
            new_entry.meta.mode = file_mode;
            new_entry.meta.mtime = file_mtime;
            avl_insert(tree, &new_entry);
            (*entries_amt)++;
            if (status == STAGED)
                printf("%s added to index with %o mode\n", rel_path, file_mode);
        }
    }
}

void meow_add(char* file, stage_status status) {
    char work_dir[PATH_MAX];
    char path_to_index[PATH_MAX];
    char path_to_indextmp[PATH_MAX];
    char path[PATH_MAX];
    int path_len = strlen(file);
    if (file[path_len - 1] == '/')
        file[path_len - 1] = '\0';

    if (find_work_dir(work_dir) == -1) {
        fprintf(stderr, "Error: .meow/ not found\n");
        exit(EXIT_FAILURE);
    }

    if (is_detached_head(work_dir) && status != COMMITED) {
        fprintf(stderr, "Error: You are not on head now\n");
        exit(EXIT_FAILURE);
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
        return;
    }

    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    snprintf(path_to_indextmp, PATH_MAX, "%s/index.tmp", work_dir);

    char project_dir[PATH_MAX];
    find_project_dir(project_dir, work_dir);

    unsigned int entries_amt = 0;
    avlTree* index = open_index(work_dir, &entries_amt);

    add_to_tree_rec(path, &index, &entries_amt, work_dir, project_dir, status);
    save_index(index, work_dir, &entries_amt);
    close_index(index);
}
