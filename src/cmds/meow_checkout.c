#include "meow.h"
#include "misc.h"
#include "object.h"
#include "unpack.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tree_checkout(char* hash) {
}

void meow_checkout(char* target) {
    char work_dir[PATH_MAX];
    char project_dir[PATH_MAX];
    char path_to_br[PATH_MAX];
    char path_to_obj[PATH_MAX];
    char path_to_tempfile[PATH_MAX];
    char req_commit_hash[41];
    find_work_dir(work_dir);
    find_project_dir(project_dir, work_dir);
    snprintf(path_to_br, PATH_MAX, "%s/refs/heads/%s", work_dir, target);
    snprintf(path_to_tempfile, PATH_MAX, "%s/tempfile", work_dir);

    FILE* br_file = fopen(path_to_br, "rb");
    if (!br_file) {
        if (strlen(target) != 41) {
            fprintf(stderr, "Error: pathspec \"%s\" did not match any file(s) known to meow1\n",
                    target);
        }
        strcpy(req_commit_hash, target);
    } else {
        fscanf(br_file, "%40s", req_commit_hash);
        fclose(br_file);
    }

    snprintf(path_to_obj, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, req_commit_hash,
             req_commit_hash + 2);

    if (!object_exists(req_commit_hash)) {
        fprintf(stderr, "Error: pathspec \"%s\" did not match any file(s) known to meow2\n",
                target);
        exit(EXIT_FAILURE);
    }

    FILE* commit = fopen(path_to_obj, "rb");

    FILE* inflated_commit = tmpfile();
    inf(commit, inflated_commit);
    rewind(inflated_commit);
    char tree_hash[41];
    fscanf(inflated_commit, "%*s %*ld");
    fgetc(inflated_commit);
    fscanf(inflated_commit, "%*s %s", tree_hash);
    LOG("tree %s\n", tree_hash);

    // unpack_tree(tree_hash, project_dir, work_dir);

    fclose(inflated_commit);
    fclose(commit);
}
