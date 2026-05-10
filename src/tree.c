#include "meow.h"

static void dir_traverse(const char* current_dir, const char* project_dir);

void meow_avl_traverse(avlTree* tree, const char* path, const char* project_dir){
    if (!tree) return;
    meow_avl_traverse(tree->child[0], path, project_dir);

    char full_path[PATH_MAX];
    snprintf(full_path, PATH_MAX, "%s/%s", path, tree->value.name);

    if (tree->value.dir){
        printf("dir: %s/\n", tree->value.name);
        dir_traverse(full_path, project_dir);
        putchar('\n');
    } else {
        char full_path[PATH_MAX];
        snprintf(full_path, PATH_MAX, "%s/%s", path, tree->value.name);
        char rel_path[PATH_MAX];
        make_path_relative(project_dir, full_path, rel_path);
        printf("%s\n", rel_path);
    }

    meow_avl_traverse(tree->child[1], path, project_dir);
}

static void dir_traverse(const char* current_dir, const char* project_dir){
    DIR* dir;
    struct dirent* ent;
    struct stat st;

    dir = opendir(current_dir);

    if (dir == NULL) assert(5);

    avlTree* Tree = NULL;

    while((ent=readdir(dir)) != 0){
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, ".meow"))
            continue;

        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s", current_dir, ent->d_name);
        stat(path, &st);
        TreeEntry node;
        strncpy(node.name, ent->d_name, sizeof(node.name) - 1);
        node.name[sizeof(node.name) - 1] = '\0';
        node.dir = (S_ISDIR(st.st_mode)) ? 1 : 0;
        if (!Tree) {
            Tree = avl_create(node);
        } else {
            avl_insert(&Tree, node);
        } 
    }

    meow_avl_traverse(Tree, current_dir, project_dir);
    avl_del_tree(Tree);
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
