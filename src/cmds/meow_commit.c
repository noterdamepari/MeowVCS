#include "misc.h"
#include "object.h"
#include "types.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void meow_commit(char* msg) {
    if (!msg) {
        printf("Сomment not found");
        return;
    }
    char work_dir[PATH_MAX];
    char path_to_index[PATH_MAX];
    char path_to_cfg[PATH_MAX];

    if (find_work_dir(work_dir) == -1) {
        fprintf(stderr, "Error: .meow/ not found\n");
        return;
    }
    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    snprintf(path_to_cfg, PATH_MAX, "%s/config", work_dir);

    FILE* index = fopen(path_to_index, "rb");
    if (!index) {
        fprintf(stderr, "Error: index not found\n");
        return;
    }
    int entries_amt;
    fscanf(index, "%d", &entries_amt);
    indexEntry* entries = (indexEntry*)malloc(sizeof(indexEntry) * entries_amt);
    puts("Index content:");
    printf("MSG: %s\n", msg);
    for (int i = 0; i < entries_amt; i++) {
        fscanf(index, "%40s %d %lld %s", entries[i].hash, &entries[i].status, &entries[i].mtime, entries[i].path);
    }

    char tree_hash[41];
    write_tree(entries, entries_amt, 0, tree_hash);

    FILE* tmp = tmpfile();

    FILE* cfg = fopen(path_to_cfg, "rb");
    if (!cfg) {
        fprintf(stderr, "Error: config file not found\n");
        return;
    }
    printf("\n\nCOMMIT:\n");
    fprintf(tmp, "tree %s\n", tree_hash);
    printf("tree %s\n", tree_hash);
    fprintf(tmp, "parent nil\n"); // TODO: get from .meow/refs/HEAD
    printf("parent nil\n");
    char buffer[256];
    time_t t = time(NULL);

    fgets(buffer, 256, cfg);
    fprintf(tmp, "%s %ld\n\n%s", buffer, t, msg);
    printf("%s %ld\n\n%s", buffer, t, msg);

    fflush(tmp);
    rewind(tmp);
    struct stat st;
    int fd = fileno(tmp);
    fstat(fd, &st);

    char hash[41];
    hash_and_create_obj(COMMIT, &st, tmp, hash);
    printf("%s", hash);

    printf("\n\n");
    free(entries);
    fclose(index);
    fclose(cfg);
}
