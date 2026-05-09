#include "meow.h"

void write_project_dir(){ 
    char work_dir[PATH_MAX];
    char project_dir[PATH_MAX];

    find_work_dir(work_dir);
    find_project_dir(project_dir, work_dir);

    dir_traverse(project_dir);
}

void dir_traverse(const char* project_dir){
    DIR* dir;
    struct dirent* ent;
    struct stat st;

    dir = opendir(project_dir);

    if (dir == NULL) assert(5);

    while((ent=readdir(dir)) != 0){
        if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..") && strcmp(ent->d_name, ".meow")){
            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "%s/%s", project_dir, ent->d_name);
            stat(path, &st);
            if(S_ISDIR(st.st_mode)){
                printf("[%s]\n", ent->d_name);
                dir_traverse(path);
            } else { 
                printf("%s\n", ent->d_name);
            }
        }

    }
    closedir(dir);
}
