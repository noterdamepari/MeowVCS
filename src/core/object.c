#include "object.h"
#include "meow.h"
#include "sha1.h"
#include "zlib.h"
#include <stdio.h>

static int object_exists(const char* hash) {
    char work_obj_dir[PATH_MAX];

    find_work_dir(work_obj_dir);
    strcat(work_obj_dir, objects_dir);
    char path[PATH_MAX];

    snprintf(path, PATH_MAX, "%s/%.2s/%s", work_obj_dir, hash, hash + 2);

    if (access(path, F_OK) == 0) {
        return 1; // Файл существует
    } else {
        return 0; // Файла нет
    }
}

static void create_object(FILE* f, const char* ihash) {
    char work_obj_dir[PATH_MAX];
    char path_to_obj_dir[PATH_MAX];
    char path_to_obj[PATH_MAX];

    find_work_dir(work_obj_dir);
    strcat(work_obj_dir, objects_dir);

    char obj_dir_name[3];
    for (int i = 0; i < 2; i++) {
        obj_dir_name[i] = ihash[i];
    }
    obj_dir_name[2] = '\0';
    snprintf(path_to_obj_dir, PATH_MAX, "%s/%s", work_obj_dir, obj_dir_name);

    mkdir(path_to_obj_dir, 0777);

    const char* obj_file_name = ihash + 2;
    snprintf(path_to_obj, PATH_MAX, "%s/%s", path_to_obj_dir, obj_file_name);
    FILE* obj = fopen(path_to_obj, "wb");
    if (!obj)
        perror("Error: Cannot create blob");
    rewind(f); // return cursor
    def(f, obj, Z_DEFAULT_COMPRESSION);
    LOG("object created - %s\n", ihash);
    fclose(obj);
}

void hash_and_create_obj(object_type type, FILE* f, char* ohash) {
    int fd = fileno(f); // file descriptor
    struct stat st;
    fstat(fd, &st);

    FILE* tmp = tmpfile();
    if (!tmp) {
        perror("Error: Cannot create tempfile in hash_and_create_obj func");
        exit(EXIT_FAILURE);
    }

    SHA1_CTX sha;
    SHA1Init(&sha);
    uint8_t binary_hash[22];

    char header[64];
    uint32_t h_len;
    switch (type) {
        case TREE: {
            h_len = snprintf(header, sizeof(header), "tree %ld", st.st_size);
            break;
        }
        case BLOB: {
            h_len = snprintf(header, sizeof(header), "blob %ld", st.st_size);
            break;
        }
        case COMMIT: {
            h_len = snprintf(header, sizeof(header), "commit %ld", st.st_size);
            break;
        }
        default: {
            break;
        }
    }
    h_len++;

    SHA1Update(&sha, (uint8_t*)header, h_len); // хешируем хедер дерева
    fwrite(header, sizeof(char), h_len, tmp);
    char read_buffer[CHUNK];
    size_t bytes_read;
    while ((bytes_read = fread(read_buffer, sizeof(uint8_t), CHUNK, f)) > 0) {
        SHA1Update(&sha, (uint8_t*)read_buffer, bytes_read);
        fwrite(read_buffer, sizeof(uint8_t), bytes_read, tmp);
    }
    SHA1Final(binary_hash, &sha);

    for (int i = 0; i < 20; i++)
        sprintf(ohash + (i * 2), "%02x", binary_hash[i]);
    ohash[40] = '\0';

    rewind(tmp);
    if (!object_exists(ohash)) {
        create_object(tmp, ohash);
    } else {
        LOG("obj already exists");
    }
    fclose(tmp);
}
