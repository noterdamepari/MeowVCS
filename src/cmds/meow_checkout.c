#include "misc.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void meow_checkout(char* target) {
    char work_dir[PATH_MAX];
    char path_to_br[PATH_MAX];
    char req_commit_hash[41];
    find_work_dir(work_dir);
    snprintf(path_to_br, PATH_MAX, "%s/refs/heads/%s", work_dir, target);

    FILE* br_file = fopen(path_to_br, "rb");
    if (!br_file) {
        if (strlen(target) != 40) {
            fprintf(stderr, "Requested commit not found");
            exit(EXIT_FAILURE);
        }
        strcpy(req_commit_hash, target);
    }
    fscanf(br_file, "%40s", req_commit_hash);
    fclose(br_file);
}
