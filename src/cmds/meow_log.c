#include "meow.h"
#include "misc.h"
#include "types.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_commit(FILE* stream, CommitEntry* commit) {
    struct tm* local_t = localtime(&commit->time);
    fprintf(stream, "* %s\n  %s %s %02d:%02d:%02d %s UTC\n  %s\n\n", commit->hash, commit->user,
            commit->email, local_t->tm_hour, local_t->tm_min, local_t->tm_sec, local_t->tm_zone,
            commit->msg);
}

int get_commit(char* hash, CommitEntry* out, char* work_dir) {
    if (!hash || !strcmp(hash, "nil"))
        return 0;
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s/objects/%.2s/%s", work_dir, hash, hash + 2);
    FILE* commit = fopen_inflated(path);
    snprintf(out->hash, 41, "%s", hash);
    fscanf(commit, "%*s %*ld");
    fgetc(commit);
    fscanf(commit, "%*s %s", out->tree);
    fgetc(commit);
    fscanf(commit, "%*s %s", out->parent);
    fgetc(commit);
    fscanf(commit, "%*s %s %s %ld", out->user, out->email, &out->time);
    fgetc(commit);
    fscanf(commit, "%s", out->msg);
    fclose(commit);
    return 1;
}

static int is_ancestor(char* curr, char* parent, char* work_dir) {
    char current[41];
    strcpy(current, parent);
    CommitEntry temp;

    while (strcmp(current, "nil")) {
        if (!strcmp(current, curr)) {
            return 1;
        }
        if (!get_commit(current, &temp, work_dir)) {
            break;
        }
        strcpy(current, temp.parent);
    }
    return 0;
}

void meow_log(int n, char* to, char* from) {
    char work_dir[PATH_MAX];
    find_work_dir(work_dir);

    CommitEntry entry;

    // hash1..hash2 situation
    if (from && to) {
        if (get_commit(from, &entry, work_dir)) {
            print_commit(stdout, &entry);
        } else { // make msg
            exit(EXIT_FAILURE);
        }
        LOG("%s %s\n", to, from);
        if (is_ancestor(to, from, work_dir)) {
            while (1) {
                if (!get_commit(entry.parent, &entry, work_dir)) {
                    break;
                }
                if (strcmp(to, entry.parent)) {
                    print_commit(stdout, &entry);
                    break;
                }
            }
        } else {
            fprintf(stderr, "Error: commits %s and %s unrelated\n", from, to);
        }
        return;
    }

    // mw log without from..to, from current to root while n>0
    char path_to_head[PATH_MAX];
    snprintf(path_to_head, PATH_MAX, "%s/HEAD", work_dir);
    FILE* headfile = fopen(path_to_head, "rb");
    char buffer[1024];
    fgets(buffer, 1024, headfile);

    char commit_hash[41];

    if (!strncmp("ref: ", buffer, 5)) {
        char path_to_br[PATH_MAX];
        snprintf(path_to_br, PATH_MAX, "%s/%s", work_dir, buffer + 5);
        FILE* br = fopen(path_to_br, "rb");
        if (!br) {
            fprintf(stderr, "Error: branch file not found");
            exit(EXIT_FAILURE);
        }
        fscanf(br, "%40s", commit_hash);
        fclose(br);
    } else {
        fscanf(headfile, "%40s", commit_hash);
    }
    fclose(headfile);

    if (!strcmp(commit_hash, "nil")) {
        printf("No commit history");
        return;
    }

    get_commit(commit_hash, &entry, work_dir);
    print_commit(stdout, &entry);
    if (n > 0)
        n--;

    if (n != -1) {
        while (n > 0 && get_commit(entry.parent, &entry, work_dir)) {
            print_commit(stdout, &entry);
            n--;
        }
        return;
    } else {
        while (get_commit(entry.parent, &entry, work_dir)) {
            print_commit(stdout, &entry);
        }
    }
}
