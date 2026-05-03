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
    unsigned int something = 0;
    fprintf(index, "%u\n", something);
    fclose(index);

    strcpy(path, def_path);
    strcat(path, objects_dir);
    mkdir(path, 0777); 
    return 0;
}

void meow_add(char* file){
    char work_dir[PATH_MAX];
    char work_obj_dir[PATH_MAX];
    char path_to_tempfile[PATH_MAX];
    char path_to_blob_dir[PATH_MAX];
    char path_to_blob[PATH_MAX];
    char path[PATH_MAX];

    find_work_dir(work_dir);

    snprintf(work_obj_dir, PATH_MAX, "%s%s", work_dir, objects_dir);
    // strcpy(work_obj_dir, work_dir);
    // strcat(work_obj_dir, objects_dir);

    snprintf(path_to_tempfile, PATH_MAX, "%s/tempfile", work_obj_dir);
    // strcpy(path_to_tempfile, work_obj_dir);
    // strcat(path_to_tempfile, "/tempfile");

    // strcpy(path_to_index, work_dir);
    // strcat(path_to_index, "/index");

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

    SHA1_CTX sha;
    SHA1Init(&sha);

    FILE* f = fopen(path, "rb");
    FILE* tempfile = fopen(path_to_tempfile, "w+b");

    uint8_t hash[CHUNK];
    char read_buffer[CHUNK];

    // формируем хедер
    char header[64];
    uint32_t h_len = snprintf(header, sizeof(header), "blob %ld", st.st_size);
    h_len++;

    SHA1Update(&sha, (uint8_t*)header, h_len); // хешируем хедер блоба
    fwrite(header, sizeof(char), h_len, tempfile);

    size_t bytes_read;
    while((bytes_read = fread(read_buffer, sizeof(uint8_t), CHUNK, f)) > 0){
        SHA1Update(&sha, (uint8_t*)read_buffer, bytes_read);
        fwrite(read_buffer, sizeof(uint8_t), bytes_read, tempfile);
    } 
    SHA1Final(hash, &sha);


    if (add_to_indexfile(path, hash, work_dir) != 0){
        return;
    } 

    char blob_dir_name[3];
    snprintf(blob_dir_name, 3, "%02x", hash[0]);
    snprintf(path_to_blob_dir, PATH_MAX, "%s/%s", work_obj_dir, blob_dir_name);
    mkdir(path_to_blob_dir, 0777);

    char blob_file_name[512];
    uint32_t offset = 0;
    for (int i = 1; i < 20; i++){
        offset += snprintf(blob_file_name + offset, sizeof(blob_file_name) - offset, "%02x", hash[i]);
    } 
    snprintf(path_to_blob, PATH_MAX, "%s/%s", path_to_blob_dir, blob_file_name);
    // strcpy(path_to_blob, path_to_blob_dir);
    // strcat(path_to_blob, blob_file_name);

    printf("hash - 0x"); 
    for (int i = 0; i < 20; i++) printf("%02x", hash[i]);    
    putchar('\n');

    FILE* blob = fopen(path_to_blob, "wb");
    rewind(tempfile); // return cursor
    def(tempfile, blob, Z_DEFAULT_COMPRESSION);

    add_to_indexfile(path, hash, work_dir);

    printf("%s\n", work_dir);
    printf("%s\n", work_obj_dir);
    printf("%s\n", path_to_tempfile);
    printf("%s\n", path_to_blob);
    fclose(f);
    fclose(tempfile);
    remove(path_to_tempfile);
    fclose(blob);
}

int main(int argc, char** argv){
    switch (argc){
        case 2:{
            if (!strcmp(argv[1], "init")) meow_init();
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
