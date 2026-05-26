#include "avl.h"
#include "index.h"
#include "misc.h"
#include "types.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void rm_rec(avlTree** index, char* path, rm_option opt, int* entries_amt,
                   char* project_dir) {

    char rel_path[PATH_MAX];
    make_path_relative(project_dir, path, rel_path);
    struct stat st;
    if (stat(path, &st) == -1) {
        fprintf(stderr, "Error: file doesn`t exists");
        exit(EXIT_FAILURE);
    }
    if (strcmp(rel_path, ".meow") == 0 || strcmp(rel_path, "./.meow") == 0 ||
        strncmp(rel_path, ".meow/", 6) == 0 || strncmp(rel_path, "./.meow/", 8) == 0) {
        return;
    }
    if (S_ISDIR(st.st_mode)) {
        DIR* d = opendir_s(path);
        struct dirent* ent;
        while ((ent = readdir(d)) != NULL) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
            char sub_path[PATH_MAX];
            snprintf(sub_path, PATH_MAX, "%s/%s", path, ent->d_name);

            rm_rec(index, sub_path, opt, entries_amt, project_dir);
        }
        closedir(d);
        if (opt == NO_CACHED)
            rmdir(path);
    } else if (S_ISREG(st.st_mode)) {
        if (index && *index && avl_find(*index, rel_path)) {
            avl_erase(index, rel_path);
            (*entries_amt)--;
        } else {
            fprintf(stderr, "Error: file %s not found in index\n", rel_path);
        }
        if (opt == NO_CACHED)
            remove(path);
    }
}

void meow_rm(char* file, rm_option opt) {
    ProjectContext p_ctx;
    char path_to_index[PATH_MAX];
    char path[PATH_MAX];
    int path_len = strlen(file);
    if (path_len > 0 && file[path_len - 1] == '/')
        file[path_len - 1] = '\0';

    find_work_dir(p_ctx.work_dir);
    find_project_dir(p_ctx.project_dir, p_ctx.work_dir);

    if (is_detached_head(p_ctx.work_dir)) {
        fprintf(stderr, "Error: You are not on head now");
        exit(EXIT_FAILURE);
    }

    if (!is_path_absolute(file)) {
        getcwd(path, PATH_MAX);
        strcat(path, "/");
        strcat(path, file);
    } else {
        strcpy(path, file);
    }

    snprintf(path_to_index, PATH_MAX, "%s/index", p_ctx.work_dir);

    int entries_amt = 0;
    avlTree* index = open_index(p_ctx.work_dir, &entries_amt);
    rm_rec(&index, path, opt, &entries_amt, p_ctx.project_dir);
    save_index(index, p_ctx.work_dir, &entries_amt);
    close_index(index);
}
