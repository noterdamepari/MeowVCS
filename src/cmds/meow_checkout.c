#include "misc.h"
#include "tree.h"
#include "types.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>

void meow_checkout(char* target) {
    ProjectContext p_ctx;
    find_work_dir(p_ctx.work_dir);
    find_project_dir(p_ctx.project_dir, p_ctx.work_dir);
    char path_to_head[PATH_MAX];
    char commit_hash[41];
    char curr_hash[41];
    char target_hash[41];
    char head[PATH_MAX];
    snprintf(path_to_head, PATH_MAX, "%s/HEAD", p_ctx.work_dir);
    FILE* headfile = fopen_s(path_to_head, "rb");
    fscanf(headfile, "%s", head);

    if (!strncmp("ref: ", head, 5)) {
        char path_to_br[PATH_MAX];
        snprintf(path_to_br, PATH_MAX, "%s/%s", p_ctx.work_dir, head + 5);
        FILE* br = fopen_s(path_to_br, "rb");
        fscanf(br, "%40s", commit_hash);
        fclose(br);
    } else {
        strcpy(commit_hash, head);
    }
    get_tree(commit_hash, curr_hash, p_ctx.work_dir);

    char target_path_to_br[PATH_MAX];
    snprintf(target_path_to_br, PATH_MAX, "%s/refs/heads/%s", p_ctx.work_dir, target);
    FILE* br_file = fopen(target_path_to_br, "rb");
    if (!br_file) {
        if (strlen(target) != 41) {
            fprintf(stderr, "Error: pathspec \"%s\" did not match any file(s) known to meow\n",
                    target);
        }
        strcpy(commit_hash, target);
    } else {
        fscanf(br_file, "%40s", commit_hash);
        fclose(br_file);
    }
    get_tree(commit_hash, target_hash, p_ctx.work_dir);

    DiffCallbacks cbs;

    walk_tree_diff(curr_hash, target_hash, p_ctx.work_dir, &p_ctx, &cbs);
}
