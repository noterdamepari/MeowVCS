#include "meow.h"

const char* default_dir = "/.meow";
const char* objects_dir = "/objects";
const char* meow_index = "/index";

char is_path_absolute(char* path){
    if (!path || path[0] == '\0') return 0;
    if (path[0] == '/' || path[0] == '\\') return 1; // Unix
    if (isalpha(path[0]) && path[1] == ':') return 1; // Windows
    return 0;
}

char meow_init(){
    char def_path[255];
    char path[255];

    getcwd(def_path, 100);

    strcat(def_path, default_dir);
    mkdir(def_path, 0777);

    strcpy(path, def_path);
    strcat(path, meow_index);
    FILE* index = fopen(path, "w");
    fclose(index);

    strcpy(path, def_path);
    strcat(path, objects_dir);
    mkdir(path, 0777); 

    return 0;
}

void meow_add(char* file){
    char path[255];
    if (!is_path_absolute(file)) {
        getcwd(path, 100);
        strcat(path, "/");
        strcat(path, file);
    } else {
        strcpy(path, file);
    }

    struct stat st;
    if (stat(path, &st) != 0) return;

    SHA1_CTX sha;
    SHA1Init(&sha);

    char header[64];
    int h_len = snprintf(header, sizeof(header), "blob %ld", st.st_size);
    h_len++;

    printf("%s\n", header);

    uint8_t results[CHUNK];

    FILE* f = fopen(path, "rb");

    SHA1Update(&sha, (uint8_t *)header, h_len); // хешируем хедер блоба

    SHA1Final(results, &sha);

    printf("0x"); 
    for (int i = 0; i < 20; i++) printf("%02x", results[i]);

    putchar('\n');
    fclose(f);
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
