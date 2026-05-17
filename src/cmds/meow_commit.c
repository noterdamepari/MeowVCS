#include "misc.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

void meow_commit(char* msg) {
    if (!msg) {
        printf("Сomment not found");
        return;
    }
    char work_dir[PATH_MAX];
    char path_to_index[PATH_MAX];

    if (find_work_dir(work_dir) == -1) {
        fprintf(stderr, "Error: .meow/ not found\n");
        return;
    }
    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);

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

    char hash[41];
    write_tree(entries, entries_amt, 0, hash);

    free(entries);
    fclose(index);
}
