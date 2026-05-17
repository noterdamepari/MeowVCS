#include "meow.h"
#include "misc.h"
#include "object.h"
#include "types.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void meow_commit(char* msg) {
    if (!msg) {
        fprintf(stderr, "Сomment not found\n");
        exit(EXIT_FAILURE);
    }
    char work_dir[PATH_MAX];
    char path_to_index[PATH_MAX];
    char path_to_head[PATH_MAX];
    char path_to_cfg[PATH_MAX];

    if (find_work_dir(work_dir) == -1) {
        fprintf(stderr, "Error: .meow/ not found\n");
        exit(EXIT_FAILURE);
    }
    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    snprintf(path_to_cfg, PATH_MAX, "%s/config", work_dir);
    snprintf(path_to_head, PATH_MAX, "%s/HEAD", work_dir);

    FILE* index = fopen(path_to_index, "rb");
    if (!index) {
        fprintf(stderr, "Error: index not found\n");
        exit(EXIT_FAILURE);
    }
    int entries_amt;
    fscanf(index, "%d", &entries_amt);
    indexEntry* entries = (indexEntry*)malloc(sizeof(indexEntry) * entries_amt);
    LOG("Index content:");
    LOG("MSG: %s\n", msg);
    for (int i = 0; i < entries_amt; i++) {
        fscanf(index, "%40s %o %d %d %ld %s", entries[i].hash, &entries[i].mode, &entries[i].fstatus, &entries[i].sstatus, &entries[i].mtime, entries[i].path);
    }

    char tree_hash[41];
    write_tree(entries, entries_amt, 0, tree_hash);

    FILE* tmp = tmpfile();

    FILE* cfg = fopen(path_to_cfg, "rb");
    if (!cfg) {
        fprintf(stderr, "Error: config file not found\n");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    FILE* head = fopen(path_to_head, "rb");
    if (!head) {
        fprintf(stderr, "Error: head not found\n");
        exit(EXIT_FAILURE);
    }
    fgets(buffer, 256, head);

    char parent[41];
    char on_head = 0; // if we now on HEAD
    char path_to_branch[256];

    if (!strncmp("ref:", buffer, 4)) {
        snprintf(path_to_branch, 256, "%s/%s", work_dir, buffer + 5);
        LOG("\n%s\n\n", path_to_branch);
        FILE* br = fopen(path_to_branch, "rb");
        if (!br) {
            fprintf(stderr, "Error: Cannot open branch file\n");
        }
        on_head = 1;
        fscanf(br, "%40s", parent);
        fclose(br);
    } else {
        fprintf(stderr, "Error: You not on head now\n");
    }

    fprintf(tmp, "tree %s\n", tree_hash);
    LOG("tree %s\n", tree_hash);

    fprintf(tmp, "parent %s\n", parent);
    LOG("parent %s\n", parent);

    time_t t = time(NULL);

    fgets(buffer, 256, cfg);
    fprintf(tmp, "author %s %ld\n\n%s", buffer, t, msg);
    LOG("author %s %ld\n\n%s", buffer, t, msg);

    fflush(tmp);
    rewind(tmp);

    LOG("\n\n");
    char hash[41];
    hash_and_create_obj(COMMIT, tmp, hash);

    if (on_head) {
        FILE* br = fopen(path_to_branch, "wb");
        if (!br) {
            fprintf(stderr, "Error: Cannot open branch file\n");
            exit(EXIT_FAILURE);
        }
        fprintf(br, "%s", hash);
        fclose(br);
    }
    char* branch_name = strrchr(path_to_branch, '/') + 1;
    if (!strcmp(parent, "nil"))
        printf("[%s (root-commit) %s] %s\n", hash, branch_name, msg);
    else
        printf("[%s %s] %s\n", hash, branch_name, msg);

    fclose(index);

    FILE* new_index = fopen(path_to_index, "wb");
    fprintf(new_index, "%u\n", entries_amt);
    for (int i = 0; i < entries_amt; i++) {
        entries[i].sstatus = COMMITED;
        fprintf(index, "%40s %o %d %d %ld %s\n", entries[i].hash, entries[i].mode, entries[i].fstatus, entries[i].sstatus, entries[i].mtime, entries[i].path);
    }

    free(entries);
    fclose(cfg);
    fclose(head);
}
