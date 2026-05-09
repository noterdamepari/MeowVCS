#include "meow.h"

const char* default_dir = "/.meow";
const char* objects_dir = "/objects";
const char* meow_index = "/index";

char meow_init(){
    char def_path[PATH_MAX];
    char path[PATH_MAX];

    getcwd(def_path, PATH_MAX);

    strcat(def_path, default_dir);
    mkdir(def_path, 0777);

    strcpy(path, def_path);
    strcat(path, meow_index);
    FILE* index = fopen(path, "wb");
    if (!index) perror("Cannot create index");
    fprintf(index, "0");
    fclose(index);

    strcpy(path, def_path);
    strcat(path, objects_dir);
    mkdir(path, 0777); 
    return 0;
}

void meow_add(char* file){
    char work_dir[PATH_MAX];
    char path_to_index[PATH_MAX];
    char path_to_indextmp[PATH_MAX];
    char path[PATH_MAX];

    if(find_work_dir(work_dir) == -1){ 
        fprintf(stderr, "Error: .meow/ not found\n");
        return; 
    }

    if (!is_path_absolute(file)) {
        getcwd(path, PATH_MAX);
        strcat(path, "/");
        strcat(path, file);
    } else {
        strcpy(path, file);
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        puts("Err: file doesn`t exists");
        return;
    }

    int64_t file_mtime = st.st_mtime;

    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    snprintf(path_to_indextmp, PATH_MAX, "%s/index.tmp", work_dir);

    FILE* index = fopen(path_to_index, "rb");
    if (!index) {
        fprintf(stderr, "Error: index not found\n");
        return; 
    }

    FILE* index_tmp = fopen(path_to_indextmp, "wb");
    if (!index) perror("index.tmp not created");

    fprintf(index_tmp, "0\n");
    unsigned int entries_amt;
    fscanf(index, "%u\n", &entries_amt);

    indexEntry entry;

    char project_dir[PATH_MAX];
    char rel_path[PATH_MAX];
    find_project_dir(project_dir, work_dir);
    make_path_relative(project_dir, path, rel_path);

    char tmpbuffer[PATH_MAX+64];

    char inserted = 0;
    int new_entries_amt = entries_amt;
    
    for (int i = 0; i < entries_amt; i++){
        fscanf(index, "%40s %hhu %lld %s", entry.hash, &entry.status, &entry.mtime, entry.path);
        char strcmp_res = strcmp(rel_path, entry.path);
        if (!strcmp_res){ // already in index -> modified
            inserted = 1;
            if (entry.mtime != file_mtime){ // nothing to do
                uint8_t hex_hash[41];
                create_blob(path, work_dir, &st, hex_hash);
                entry.status = 0;
                strcpy(entry.hash, hex_hash);
                entry.mtime = file_mtime;
            } else {
                puts("Nothing to do, already in index");
                fclose(index_tmp);
                fclose(index);
                remove(path_to_indextmp);
                return;
            }
            fprintf(index_tmp, "%s %hhu %lld %s\n", entry.hash, entry.status, entry.mtime, entry.path);
        } else if (strcmp_res < 0 && !inserted){
            inserted = 1;
            new_entries_amt++;

            uint8_t hex_hash[41];
            create_blob(path, work_dir, &st, hex_hash);
            indexEntry new_entry;
            strcpy(new_entry.hash, hex_hash);
            strcpy(new_entry.path, rel_path);
            new_entry.status = 1;
            new_entry.mtime = file_mtime;
            fprintf(index_tmp, "%s %hhu %lld %s\n", new_entry.hash, new_entry.status, new_entry.mtime, new_entry.path);
         
            fprintf(index_tmp, "%s %hhu %lld %s\n", entry.hash, entry.status, entry.mtime, entry.path);
        } else {
            fprintf(index_tmp, "%s %hhu %lld %s\n", entry.hash, entry.status, entry.mtime, entry.path);
        }
    }

    printf("%s\n", rel_path);
    if(!inserted){
        uint8_t hex_hash[41];
        create_blob(path, work_dir, &st, hex_hash);
        strcpy(entry.hash, hex_hash);
        strcpy(entry.path, rel_path);
        entry.status = 1;
        entry.mtime = file_mtime;
        fprintf(index_tmp, "%s %hhu %lld %s\n", entry.hash, entry.status, entry.mtime, entry.path);
        new_entries_amt++;
    }

    rewind(index_tmp);
    fprintf(index_tmp, "%u\n", new_entries_amt);
 
    fclose(index_tmp);
    fclose(index);

    if (rename(path_to_indextmp, path_to_index) != 0) {
        perror("Err: rename failed");
        return;
    }

}

int main(int argc, char** argv){
    switch (argc){
        case 2:{
            if (!strcmp(argv[1], "init")) meow_init();
            if (!strcmp(argv[1], "dir_traverse")) write_project_dir();
            break;
        } 
        case 3:{
            if (!strcmp(argv[1], "add")) meow_add(argv[2]);
            break;
        }
        default:{
            puts("Err: No arguments");
            break;
        }
    }
    return 0;
}
