#include "meow.h"

static void dir_traverse(const char* current_dir, const char* project_dir){
    DIR* dir;
    struct dirent* ent;
    struct stat st;

    dir = opendir(current_dir);

    if (dir == NULL) assert(5);

    while((ent=readdir(dir)) != 0){
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, ".meow"))
            continue;
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s", current_dir, ent->d_name);
        stat(path, &st);
        if(S_ISDIR(st.st_mode)){
            printf("\n[%s]\n", ent->d_name);
            dir_traverse(path, project_dir);
            printf("\n");
        }
    }

    rewinddir(dir);


    while((ent=readdir(dir)) != 0){
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, ".meow"))
            continue;
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s", current_dir, ent->d_name);
        stat(path, &st);
        if(!S_ISDIR(st.st_mode)){
            char rel_path[PATH_MAX];
            make_path_relative(project_dir, path, rel_path);
            printf("%s\n", rel_path);
        }
    }

    closedir(dir);
}


void write_project_dir(){ 
    char work_dir[PATH_MAX];
    char project_dir[PATH_MAX];

    if(find_work_dir(work_dir) == -1){ 
        fprintf(stderr, "Error: .meow/ not found\n");
        return; 
    }
    find_project_dir(project_dir, work_dir);

    dir_traverse(project_dir, project_dir);
}
