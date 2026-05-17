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
    return 0;
}
