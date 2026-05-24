#include "meow.h"
#include "misc.h"
#include "object.h"
#include "types.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void get_dirname(const char* path, char* dst) {
    const char* slash = strchr(path, '/');
    if (slash) {
        size_t len = slash - path;
        strncpy(dst, path, len);
        dst[len] = '\0';
    }
}

void write_tree(const indexMeta* entries, char** entries_paths, const int entries_amt,
                int path_offset, char* ohash) {
    FILE* tmp_tree_obj = tmpfile();
    if (!tmp_tree_obj) {
        perror("Error: Cannot create tempfile in write_tree func");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (i < entries_amt) {
        if (entries[i].fstatus == DELETED) {
            i++;
            continue;
        }
        const char* path = entries_paths[i] + path_offset;
        const char* slash = strchr(path, '/');

        if (!slash) {
            // if in root dir
            fprintf(tmp_tree_obj, "%o blob %s %s\n", entries[i].mode, entries[i].hash, path);
            LOG("%o blob %s %s\n", entries[i].mode, entries[i].hash, path);
            i++;
        } else {
            // subdir
            char dirname[256];
            get_dirname(path, dirname);
            int dirname_len = strlen(dirname);
            int start = i;
            int cnt = 0;
            while (i < entries_amt) {
                path = entries_paths[i] + path_offset;
                if (strchr(path, '/') == NULL)
                    break;
                char curr_dirname[256];
                get_dirname(path, curr_dirname);
                if (strcmp(dirname, curr_dirname) != 0)
                    break;
                cnt++;
                i++;
            }
            char hash[41];
            write_tree(entries + start, entries_paths + start, cnt, path_offset + dirname_len + 1,
                       hash);
            fprintf(tmp_tree_obj, "040000 tree %s %s\n", hash, dirname);
            LOG("040000 tree %s %s\n", hash, dirname);
        }
    }

    fflush(tmp_tree_obj);
    rewind(tmp_tree_obj);

    hash_and_create_obj(TREE, tmp_tree_obj, ohash);

    fclose(tmp_tree_obj);
}

int read_tree_entry(FILE* tree, TreeEntry* entry) {
    if (fscanf(tree, "%o %9s %40s %s", &entry->mode, entry->type, entry->hash, entry->name) == 4)
        return 1;
    return 0;
}

void walk_tree_diff(char* src_hash, char* target_hash, char* prefix, ProjectContext* p_ctx,
                    DiffCallbacks* cbs) {
    if (!strcmp(src_hash, target_hash)) {
        return;
    }

    char path_src[PATH_MAX];
    char path_target[PATH_MAX];
    TreeEntry src_entry;
    TreeEntry target_entry;
    FILE* inflated_src = NULL;
    FILE* inflated_target = NULL;
    memset(&src_entry, 0, sizeof(TreeEntry));
    memset(&target_entry, 0, sizeof(TreeEntry));
    int src_flag = 0;
    int target_flag = 0;
    if (src_hash && strlen(src_hash) >= 2) {
        snprintf(path_src, PATH_MAX, "%s%s/%.2s/%s", p_ctx->work_dir, objects_dir, src_hash,
                 src_hash + 2);
        inflated_src = fopen_inflated(path_src);
        fscanf(inflated_src, "%*s %*ld");
        fgetc(inflated_src);
        src_flag = read_tree_entry(inflated_src, &src_entry);
    }
    if (target_hash && strlen(target_hash) >= 2) {
        snprintf(path_target, PATH_MAX, "%s%s/%.2s/%s", p_ctx->work_dir, objects_dir, target_hash,
                 target_hash + 2);
        inflated_target = fopen_inflated(path_target);
        fscanf(inflated_target, "%*s %*ld");
        fgetc(inflated_target);
        target_flag = read_tree_entry(inflated_target, &target_entry);
    }

    while (src_flag || target_flag) {
        int cmp = (src_flag && target_flag) ? strcmp(src_entry.name, target_entry.name) : 0;
        char path[PATH_MAX];
        char rel_path[PATH_MAX];

        // DELETED
        if (src_flag && (!target_flag || cmp < 0)) {

            snprintf(path, PATH_MAX, "%s/%s", prefix, src_entry.name);
            make_path_relative(p_ctx->project_dir, path, rel_path);

            if (!strcmp(src_entry.type, "tree")) {
                walk_tree_diff(src_entry.hash, "", path, p_ctx, cbs);
            }

            cbs->delete(path, rel_path, p_ctx, &src_entry);

            src_flag = read_tree_entry(inflated_src, &src_entry);

            // ADDED
        } else if (target_flag && (!src_flag || cmp > 0)) {

            snprintf(path, PATH_MAX, "%s/%s", prefix, target_entry.name);
            make_path_relative(p_ctx->project_dir, path, rel_path);

            cbs->add(path, rel_path, p_ctx, &target_entry);

            if (!strcmp(target_entry.type, "tree")) {
                walk_tree_diff("", target_entry.hash, path, p_ctx, cbs);
            }

            target_flag = read_tree_entry(inflated_target, &target_entry);

            // MODIF OR TYPE
        } else if (target_flag && src_flag) {

            char new_prefix[PATH_MAX];
            snprintf(new_prefix, PATH_MAX, "%s/%s", prefix, src_entry.name);
            make_path_relative(p_ctx->project_dir, new_prefix, rel_path);
            if (strcmp(src_entry.type, target_entry.type)) {
                cbs->type_change(new_prefix, rel_path, p_ctx, &src_entry, &target_entry);

            } else if (strcmp(src_entry.hash, target_entry.hash) ||
                       src_entry.mode != target_entry.mode) {

                if (!strcmp(src_entry.type, "tree")) {
                    walk_tree_diff(src_entry.hash, target_entry.hash, new_prefix, p_ctx, cbs);
                } else {
                    cbs->modif(new_prefix, rel_path, p_ctx, &src_entry, &target_entry);
                }
            }
            src_flag = read_tree_entry(inflated_src, &src_entry);
            target_flag = read_tree_entry(inflated_target, &target_entry);
        }
    }
    if (inflated_src)
        fclose(inflated_src);
    if (inflated_target)
        fclose(inflated_target);
}
