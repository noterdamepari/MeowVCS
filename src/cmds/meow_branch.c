#include "meow.h"
#include "misc.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>

void meow_branch(char* name) {
    char work_dir[PATH_MAX];
    char path_to_head[PATH_MAX];
    char path_to_branch[PATH_MAX];
    char commit_hash[41];
    find_work_dir(work_dir);

    snprintf(path_to_head, PATH_MAX, "%s/HEAD", work_dir);
    snprintf(path_to_branch, PATH_MAX, "%s/refs/heads/%s", work_dir, name);
    FILE* rheadfile = fopen_s(path_to_head, "rb");
    char buffer[1024];
    fgets(buffer, 1024, rheadfile);
    fclose(rheadfile);

    if (!strncmp("ref: ", buffer, 5)) {
        char path_to_current_branch[PATH_MAX];
        snprintf(path_to_current_branch, PATH_MAX, "%s/%s", work_dir, buffer + 5);
        FILE* curr_br = fopen_s(path_to_current_branch, "rb");
        fgets(buffer, 1024, curr_br);
        fclose(curr_br);
    }
    strcpy(commit_hash, buffer);
    LOG("%s", commit_hash);
    FILE* new_branch = fopen_s(path_to_branch, "wb");
    fprintf(new_branch, "%s", commit_hash);
    fclose(new_branch);
    FILE* wheadfile = fopen_s(path_to_head, "wb");
    fprintf(wheadfile, "ref: refs/heads/%s", name);
    fclose(wheadfile);
}
