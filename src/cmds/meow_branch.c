#include "meow.h"
#include "misc.h"
#include "types.h"
#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void meow_branch(char* name) {
    char work_dir[PATH_MAX];
    char path_to_head[PATH_MAX];
    char path_to_branch[PATH_MAX];
    char commit_hash[41];
    find_work_dir(work_dir);

    snprintf(path_to_head, PATH_MAX, "%s/HEAD", work_dir);
    snprintf(path_to_branch, PATH_MAX, "%s/refs/heads/%s", work_dir, name);

    LOG("1\n");
    struct stat st;
    if (stat(path_to_branch, &st) == 0) {
        fprintf(stderr, "Error: branch with this name already exists\n");
        exit(EXIT_FAILURE);
    }

    get_commit_from_head(commit_hash, work_dir);

    LOG("1\n");
    char* slash = strchr(name, '/');
    while (slash) {
        *slash = '\0';
        char path_to_new_dir[PATH_MAX];
        snprintf(path_to_new_dir, PATH_MAX, "%s/refs/heads/%s", work_dir, name);
        LOG("%s\n", path_to_new_dir);
        mkdir(path_to_new_dir, 0777);
        *slash = '/';
        slash = strchr(slash + 1, '/');
    }

    FILE* new_branch = fopen_s(path_to_branch, "wb");
    fprintf(new_branch, "%s", commit_hash);
    fclose(new_branch);
    FILE* wheadfile = fopen_s(path_to_head, "wb");
    fprintf(wheadfile, "ref: refs/heads/%s", name);
    fclose(wheadfile);
}

static void list_branches(char* path, char* active_br, ProjectContext* p_ctx) {
    DIR* d = opendir_s(path);
    struct dirent* ent;
    struct stat st;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        char sub_path[PATH_MAX];
        snprintf(sub_path, PATH_MAX, "%s/%s", path, ent->d_name);
        stat(sub_path, &st);
        if (S_ISDIR(st.st_mode)) {
            list_branches(sub_path, active_br, p_ctx);
        } else {
            char rel_path[PATH_MAX];
            make_path_relative(p_ctx->project_dir, sub_path, rel_path);
            if (active_br && !strcmp(active_br, rel_path)) {
                printf("* %s\n", rel_path);
            } else {
                printf("  %s\n", rel_path);
            }
        }
    }
    closedir(d);
}

void get_branches() {
    ProjectContext p_ctx;
    find_work_dir(p_ctx.work_dir);
    char path_to_head[PATH_MAX];
    char active_br[PATH_MAX];
    snprintf(p_ctx.project_dir, PATH_MAX, "%s/refs/heads", p_ctx.work_dir);
    snprintf(path_to_head, PATH_MAX, "%s/HEAD", p_ctx.work_dir);

    FILE* headfile = fopen_s(path_to_head, "rb");
    char buffer[1024];
    fgets(buffer, 1024, headfile);
    if (!strncmp("ref: ", buffer, 5)) {
        strcpy(active_br, buffer + 16);
        LOG("%s\n", active_br);
        list_branches(p_ctx.project_dir, active_br, &p_ctx);
    } else {
        list_branches(p_ctx.project_dir, NULL, &p_ctx);
    }
    fclose(headfile);
}
