#include "misc.h"
#include "types.h"
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void meow_status() {
    char work_dir[PATH_MAX];
    char path_to_index[PATH_MAX];
    char path_to_head[PATH_MAX];
    find_work_dir(work_dir);
    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    snprintf(path_to_head, PATH_MAX, "%s/HEAD", work_dir);

    FILE* index = fopen(path_to_index, "rb");
    if (!index) {
        perror("Error: Index not found");
        exit(EXIT_FAILURE);
    }
    FILE* head = fopen(path_to_head, "rb");
    if (!head) {
        perror("Error: HEAD file not found");
        exit(EXIT_FAILURE);
    }

    char buffer[PATH_MAX];

    fgets(buffer, PATH_MAX, head);

    if (!strncmp("ref: ", buffer, 5)) {
        char* br_name = strchr(buffer, '/');
        if (!br_name) {
            fprintf(stderr, "HEAD file corruption");
            exit(EXIT_FAILURE);
        }
        br_name = strchr(br_name + 1, '/');
        if (!br_name) {
            fprintf(stderr, "HEAD file corruption");
            exit(EXIT_FAILURE);
        }
        printf("On branch %s\n\n", br_name + 1);
    } else {
        printf("On commit %s\n", buffer);
        fclose(head);
        fclose(index);
        return; // if we in detached head mode all files already commited, nothing to check
    }

    int entries_amt = 0;
    printf("%d", entries_amt);
    fread(&entries_amt, sizeof(int), 1, index);
    printf("%d", entries_amt);
    indexMeta entry;
    char entry_path[PATH_MAX];
    uint8_t staged_exists = 0;

    for (int i = 0; i < entries_amt; i++) {
        fread(&entry, sizeof(indexMeta), 1, index);
        fread(entry_path, sizeof(char), entry.path_len, index);
        if (entry.sstatus == STAGED) {
            if (!staged_exists) {
                printf("Staged files:\n");
                staged_exists = 1;
            }
            printf("\t%s\n", entry_path);
        }
    }
    if (!staged_exists)
        printf("No staged files");

    fclose(head);
    fclose(index);
}
