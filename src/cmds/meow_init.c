#include "meow.h"
#include <stdio.h>

char meow_init() {
    char def_path[PATH_MAX];
    char path[PATH_MAX];

    getcwd(def_path, PATH_MAX);

    strcat(def_path, default_dir);
    mkdir(def_path, 0777);

    snprintf(path, PATH_MAX, "%s/index", def_path);
    FILE* index = fopen(path, "wb");
    if (!index)
        perror("Cannot create index");
    fprintf(index, "0");
    fclose(index);

    snprintf(path, PATH_MAX, "%s/HEAD", def_path);
    FILE* head = fopen(path, "wb");
    if (!head)
        perror("Cannot create HEAD file");
    fprintf(head, "nil");
    fclose(head);

    snprintf(path, PATH_MAX, "%s/config", def_path);
    FILE* cfg = fopen(path, "wb");
    if (!cfg)
        perror("Cannot create config file");

    char buffer[255];
    printf("Your username: ");
    scanf("%s", buffer);
    unsigned len = strlen(buffer);
    buffer[len] = ' ';
    printf("Your email: ");
    scanf("%s", buffer + len + 1);
    fprintf(cfg, "%s", buffer);
    puts("Project initialized");
    fclose(cfg);

    snprintf(path, PATH_MAX, "%s%s", def_path, objects_dir);
    mkdir(path, 0777);
    snprintf(path, PATH_MAX, "%s/refs", def_path);
    mkdir(path, 0777);
    snprintf(path, PATH_MAX, "%s/refs/heads", def_path);
    mkdir(path, 0777);
    snprintf(path, PATH_MAX, "%s/refs/master", def_path);
    FILE* master_branch = fopen(path, "wb");
    if (!master_branch)
        perror("Cannot create branch file");
    fprintf(master_branch, "nil");
    fclose(master_branch);
    return 0;
}
