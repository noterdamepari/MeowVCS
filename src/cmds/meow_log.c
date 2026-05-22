#include "meow.h"
#include "misc.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void write_log_rec(FILE* stream, char* commit_hash, char* work_dir) {
    if (!strcmp(commit_hash, "nil")) {
        return;
    }
    char buffer[1024];
    char path_to_commit[PATH_MAX];
    snprintf(path_to_commit, PATH_MAX, "%s/objects/%.2s/%s", work_dir, commit_hash, commit_hash + 2);
    FILE* commit = fopen_inflated(path_to_commit);
    fscanf(commit, "%*s %*ld");
    fgetc(commit);
    fgets(buffer, 1024, commit);
    fscanf(commit, "%*s %s", buffer);
    LOG("parent %s\n", buffer);
    fgetc(commit); // skip \n

    char user[256];
    char email[256];
    time_t time;
    char message[256];

    fscanf(commit, "%*s %s %s %ld", user, email, &time);
    fgetc(commit);
    fscanf(commit, "%s", message);
    struct tm* local_t = localtime(&time);
    fprintf(stream, "* %s\n  %s %s %d:%d:%d\n  %s\n", commit_hash, user, email, local_t->tm_hour, local_t->tm_min, local_t->tm_sec, message);
    fclose(commit);
    write_log_rec(stream, buffer, work_dir);
}

void meow_log() {
    char work_dir[PATH_MAX];
    char path_to_head[PATH_MAX];
    find_work_dir(work_dir);
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

    write_log_rec(stdout, commit_hash, work_dir);

    fclose(headfile);
}
