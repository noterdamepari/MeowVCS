#include "meow.h"
#include "misc.h"
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
    struct stat st;
    if (stat(path_to_branch, &st) == 0) {
        fprintf(stderr, "Error: branch with this name already exists\n");
        exit(EXIT_FAILURE);
    }

    get_commit_from_head(commit_hash, work_dir);

    FILE* new_branch = fopen_s(path_to_branch, "wb");
    fprintf(new_branch, "%s", commit_hash);
    fclose(new_branch);
    FILE* wheadfile = fopen_s(path_to_head, "wb");
    fprintf(wheadfile, "ref: refs/heads/%s", name);
    fclose(wheadfile);
}
