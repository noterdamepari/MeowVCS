#include "meow.h"
#include "misc.h"
#include "types.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>

void tree_diff(char* src_hash, char* target_hash, char* prefix, char* work_dir, char* project_dir);

void meow_diff(char* source_commit, char* target_commit) {
    char src_tree[41];
    char target_tree[41];
    char work_dir[PATH_MAX];
    char project_dir[PATH_MAX];
    find_work_dir(work_dir);
    find_project_dir(project_dir, work_dir);
    get_tree(source_commit, src_tree, work_dir);
    get_tree(target_commit, target_tree, work_dir);
    tree_diff(src_tree, target_tree, project_dir, work_dir, project_dir);
}

int read_tree_entry(FILE* tree, TreeEntry* entry) {
    if (fscanf(tree, "%o %9s %40s %s", &entry->mode, entry->type, entry->hash, entry->name) == 4)
        return 1;
    return 0;
}

void line_diff(char* src_hash, char* target_hash, char* work_dir) {
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, src_hash, src_hash + 2);
    FILE* src = fopen_inflated(path);
    snprintf(path, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, target_hash, target_hash + 2);
    FILE* target = fopen_inflated(path);

    fscanf(src, "%*s %*ld");
    fgetc(src);
    fscanf(target, "%*s %*ld");
    fgetc(target);

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

void tree_diff(char* src_hash, char* target_hash, char* prefix, char* work_dir, char* project_dir) {
    if (!strcmp(src_hash, target_hash)) {
        return;
    }

    char path_src[PATH_MAX];
    char path_target[PATH_MAX];
    snprintf(path_src, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, src_hash, src_hash + 2);
    snprintf(path_target, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, target_hash,
             target_hash + 2);

    FILE* inflated_src = fopen_inflated(path_src);
    FILE* inflated_target = fopen_inflated(path_target);
    printf("%s <- %s\n", src_hash, target_hash);

    TreeEntry src_entry;
    TreeEntry target_entry;

    fscanf(inflated_src, "%*s %*ld");
    fgetc(inflated_src);
    fscanf(inflated_target, "%*s %*ld");
    fgetc(inflated_target);

    int src_flag = read_tree_entry(inflated_src, &src_entry);
    int target_flag = read_tree_entry(inflated_target, &target_entry);

    while (src_flag || target_flag) {
        int cmp = (src_flag && target_flag) ? strcmp(src_entry.name, target_entry.name) : 0;
        char path[PATH_MAX];
        char rel_path[PATH_MAX];
        if (src_flag && (!target_flag || cmp < 0)) {
            snprintf(path, PATH_MAX, "%s/%s", prefix, src_entry.name);
            make_path_relative(project_dir, path, rel_path);
            printf("DELETED %s\n", rel_path);
            src_flag = read_tree_entry(inflated_src, &src_entry);
        } else if (target_flag && (!src_flag || cmp > 0)) {
            snprintf(path, PATH_MAX, "%s/%s", prefix, target_entry.name);
            make_path_relative(project_dir, path, rel_path);
            printf("ADDED %s\n", rel_path);
            target_flag = read_tree_entry(inflated_target, &target_entry);
        } else {
            char new_prefix[PATH_MAX];
            snprintf(new_prefix, PATH_MAX, "%s/%s", prefix, src_entry.name);
            make_path_relative(project_dir, new_prefix, rel_path);
            if (strcmp(src_entry.type, target_entry.type)) {
                printf("TYPE CHANGE: %s changed from %s to %s\n", rel_path, src_entry.type,
                       target_entry.type);
                printf("DELETED: %s\n", rel_path);
                printf("ADDED:   %s\n", rel_path);
            } else if (strcmp(src_entry.hash, target_entry.hash)) {
                if (!strcmp(src_entry.type, "tree")) {
                    tree_diff(src_entry.hash, target_entry.hash, new_prefix, work_dir, project_dir);
                } else {
                    printf("MODIFIED %s\n", rel_path);
                    line_diff(src_entry.hash, target_entry.hash, work_dir);
                }
            }
            src_flag = read_tree_entry(inflated_src, &src_entry);
            target_flag = read_tree_entry(inflated_target, &target_entry);
        }
    }
    fclose(inflated_src);
    fclose(inflated_target);
}
