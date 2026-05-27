#include "misc.h"
#include "tree.h"
#include "types.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>

static void line_diff(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* src_ent,
                      TreeEntry* target_ent) {
    char obj_path[PATH_MAX];
    snprintf(obj_path, PATH_MAX, "%s/objects/%.2s/%s", p_ctx->work_dir, src_ent->hash,
             src_ent->hash + 2);
    FILE* src = fopen_inflated(obj_path);
    snprintf(obj_path, PATH_MAX, "%s/objects/%.2s/%s", p_ctx->work_dir, target_ent->hash,
             target_ent->hash + 2);
    FILE* target = fopen_inflated(obj_path);

    if (src_ent->mode != target_ent->mode) {
        printf("MODE CHANGED: %o -> %o", src_ent->mode, target_ent->mode);
    }

    fscanf(src, "%*s %*ld");
    fgetc(src);
    fscanf(target, "%*s %*ld");
    fgetc(target);

    printf("MODIFIED: %s\n", rel_path);

    char line1[1024];
    char line2[1024];
    char* ptr1;
    char* ptr2;
    int num1 = 0;
    int num2 = 0;
    while (1) {
        ptr1 = fgets(line1, sizeof(line1), src);
        ptr2 = fgets(line2, sizeof(line2), target);
        if (ptr1 && ptr2) {
            num1++;
            num2++;
            if (strcmp(line1, line2)) {
                printf("\tline: %d\t - %s", num1, line1);
                printf("\tline: %d\t + %s", num2, line2);
            }
        } else if (ptr1 && !ptr2) {
            num1++;
            printf("\tline: %d\t - %s", num1, line1);
        } else if (!ptr1 && ptr2) {
            num2++;
            printf("\tline: %d\t + %s", num2, line2);
        } else {
            break;
        }
    }

    fclose(src);
    fclose(target);
}

static void add(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* src) {
    printf("ADDED: %s\n", rel_path);
}

static void deleted(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* src) {
    printf("DELETED: %s\n", rel_path);
}

static void typechange(char* path, char* rel_path, ProjectContext* p_ctx, TreeEntry* src,
                       TreeEntry* target) {
    printf("TYPE CHANGE: %s changed from %s to %s\n", rel_path, src->type, target->type);
    printf("DELETED: %s\n", rel_path);
    printf("ADDED:   %s\n", rel_path);
}

void meow_diff(char* source, char* target) {
    char src_hash[41];
    char target_hash[41];
    char src_tree[41];
    char target_tree[41];
    char src_msg[1024];
    char target_msg[1024];
    ProjectContext p_ctx;

    find_work_dir(p_ctx.work_dir);
    find_project_dir(p_ctx.project_dir, p_ctx.work_dir);

    get_commit_from_link(src_hash, NULL, source, p_ctx.work_dir);
    get_commit_from_link(target_hash, NULL, target, p_ctx.work_dir);

    DiffCallbacks cbs = {
        .add = add,
        .delete = deleted,
        .modif = line_diff,
        .type_change = typechange,
    };

    get_tree(src_hash, src_tree, p_ctx.work_dir);
    get_tree(target_hash, target_tree, p_ctx.work_dir);
    get_msg(src_hash, src_msg, p_ctx.work_dir);
    get_msg(target_hash, target_msg, p_ctx.work_dir);
    printf("%s -> %s\n\n", src_msg, target_msg);

    walk_tree_diff(src_tree, target_tree, p_ctx.project_dir, &p_ctx, &cbs);
}
