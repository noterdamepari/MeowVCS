#include "index.h"
#include "meow.h"
#include "misc.h"
#include "tree.h"
#include "types.h"
#include "unpack.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void add(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* target) {
    LOG("%s %s\n", path, rel_path);
    printf("ADDED: %s\n", rel_path);
    if (!strcmp(target->type, "tree")) {
        mkdir(path, 0777);
    } else {
        char path_to_blob[PATH_MAX];
        snprintf(path_to_blob, PATH_MAX, "%s/objects/%.2s/%s", p_ctx->work_dir, target->hash,
                 target->hash + 2);
        unpack_blob(path_to_blob, path, target->mode);
    }
}

static void deleted(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* src) {
    printf("DELETED: %s\n", rel_path);
    if (!strcmp(src->type, "tree")) {
        rmdir(path);
    } else {
        remove(path);
    }
}

static void modified(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* src,
                     TreeEntry* target) {
    char path_to_blob[PATH_MAX];
    snprintf(path_to_blob, PATH_MAX, "%s/objects/%.2s/%s", p_ctx->work_dir, target->hash,
             target->hash + 2);
    unpack_blob(path_to_blob, path, target->mode);
    printf("MODIFIED: %s\n", rel_path);
}

static void typechange(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* src,
                       TreeEntry* target) {
    printf("TYPE CHANGE: %s changed from %s to %s\n", rel_path, src->type, target->type);
    printf("DELETED: %s\n", rel_path);
    if (!strcmp(src->type, "tree")) {
        rmdir_rec(path);
    } else {
        remove(path);
    }

    if (!strcmp(target->type, "tree")) {
        unpack_tree(target->hash, path, p_ctx->work_dir);
    } else {
        char path_to_blob[PATH_MAX];
        snprintf(path_to_blob, PATH_MAX, "%s/objects/%.2s/%s", p_ctx->work_dir, target->hash,
                 target->hash + 2);
        unpack_blob(path_to_blob, path, target->mode);
    }
    printf("ADDED: %s\n", rel_path);
}

void meow_checkout(char* target) {
    ProjectContext p_ctx;
    find_work_dir(p_ctx.work_dir);
    find_project_dir(p_ctx.project_dir, p_ctx.work_dir);
    char commit_hash[41];
    char curr_hash[41];
    char target_hash[41];
    get_commit_from_head(commit_hash, p_ctx.work_dir);
    get_tree(commit_hash, curr_hash, p_ctx.work_dir);

    char target_path_to_br[PATH_MAX];
    snprintf(target_path_to_br, PATH_MAX, "%s/refs/heads/%s", p_ctx.work_dir, target);
    int checkout_to_br = 0;
    FILE* br_file = fopen(target_path_to_br, "rb");
    if (!br_file) {
        if (strlen(target) != 40) {
            fprintf(stderr, "Error: pathspec \"%s\" did not match any file(s) known to meow\n",
                    target);
        }
        strcpy(commit_hash, target);
    } else {
        fscanf(br_file, "%40s", commit_hash);
        checkout_to_br = 1;
        fclose(br_file);
    }
    get_tree(commit_hash, target_hash, p_ctx.work_dir);

    DiffCallbacks cbs = {
        .add = add,
        .delete = deleted,
        .modif = modified,
        .type_change = typechange,
    };

    walk_tree_diff(curr_hash, target_hash, p_ctx.project_dir, &p_ctx, &cbs);
    update_index_from_tree(target_hash, &p_ctx);

    char path_to_headfile[PATH_MAX];
    snprintf(path_to_headfile, PATH_MAX, "%s/HEAD", p_ctx.work_dir);
    FILE* head = fopen_s(path_to_headfile, "wb");
    if (checkout_to_br) {
        fprintf(head, "ref: refs/heads/%s", target);
    } else {
        fprintf(head, "%s", target);
    }
    fclose(head);
}
