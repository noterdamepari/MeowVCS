#include "meow.h"
#include <stdio.h>
#include <stdlib.h>

char meow_init() {
    char def_path[PATH_MAX];
    char path[PATH_MAX];

    getcwd(def_path, PATH_MAX);

    strcat(def_path, default_dir);
    if (mkdir(def_path, 0777) == -1) {
        fprintf(stderr, "Error: Repository already exists");
        exit(EXIT_FAILURE);
    }

    snprintf(path, PATH_MAX, "%s/index", def_path);
    FILE* index = fopen_s(path, "wb");

    int entries_amt = 0;
    fwrite(&entries_amt, sizeof(int), 1, index);
    fclose(index);

    snprintf(path, PATH_MAX, "%s/config", def_path);
    FILE* cfg = fopen_s(path, "wb");

    char username[255];
    char email[255];
    printf("Your username: ");
    scanf("%s", username);
    printf("Your email: ");
    scanf("%s", email);
    fprintf(cfg, "%s <%s>", username, email);
    fclose(cfg);

    snprintf(path, PATH_MAX, "%s%s", def_path, objects_dir);
    mkdir(path, 0777);
    snprintf(path, PATH_MAX, "%s/refs", def_path);
    mkdir(path, 0777);
    snprintf(path, PATH_MAX, "%s/refs/heads", def_path);
    mkdir(path, 0777);

    snprintf(path, PATH_MAX, "%s/refs/heads/master", def_path);
    FILE* master_branch = fopen_s(path, "wb");
    fprintf(master_branch, "nil");
    fclose(master_branch);

    snprintf(path, PATH_MAX, "%s/HEAD", def_path);
    FILE* head = fopen_s(path, "wb");
    fprintf(head, "ref: refs/heads/master");
    fclose(head);
    puts("Project initialized");
    return 0;
}
