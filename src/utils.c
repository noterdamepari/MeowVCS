#include "meow.h"
#include <stdio.h>

char is_path_absolute(char* path) {
    if (!path || path[0] == '\0')
        return 0;
    if (path[0] == '/' || path[0] == '\\')
        return 1; // Unix
    if (isalpha(path[0]) && path[1] == ':')
        return 1; // Windows
    return 0;
}

void get_object_path(char* dest, const char* work_dir, uint8_t* hash) {
    sprintf(dest, "%s/objects/%02x/", work_dir, hash[0]);
    mkdir(dest, 0777);

    char hex_hash[41];
    for (int i = 0; i < 20; i++) {
        sprintf(hex_hash + i * 2, "%02x", hash[i]);
    }
    strcat(dest, hex_hash + 2);
}

int make_path_relative(const char* root, const char* input, char* output) {
    char res[PATH_MAX];

    if (realpath(input, res) == NULL) {
        return -1;
    }

    if (strncmp(res, root, strlen(root)) != 0) {
        return -2;
    }

    const char* relative_ptr = res + strlen(root);

    if (*relative_ptr == '/') {
        relative_ptr++;
    }

    strcpy(output, relative_ptr);

    return 0;
}

char find_work_dir(char* buffer) {
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);

    while (1) {
        char tmp_path[PATH_MAX];
        snprintf(tmp_path, sizeof(tmp_path), "%s/.meow", cwd);

        struct stat st;

        if (!stat(tmp_path, &st) && S_ISDIR(st.st_mode)) {
            strcpy(buffer, tmp_path);
            return 0;
        }

        if (!strcmp(cwd, "/")) {
            break; // root
        }

        char* last_slash = strrchr(cwd, '/');
        if (last_slash != NULL) {
            if (last_slash == cwd) {
                strcpy(cwd, "/"); // root
            } else {
                *last_slash = '\0';
            }
        } else {
            break;
        }
    }
    return -1;
}

void find_project_dir(char* buffer, char* work_dir) {
    int len = strlen(work_dir);
    if (len > 6) {
        strncpy(buffer, work_dir, len - 6);
        buffer[len - 6] = '\0';
    } else {
        strcpy(buffer, "/");
    }
}

char create_blob(char* path, char* work_dir, struct stat* st, char* ohash) {
    char work_obj_dir[PATH_MAX];
    char path_to_tempfile[PATH_MAX];
    char path_to_blob_dir[PATH_MAX];
    char path_to_blob[PATH_MAX];

    snprintf(work_obj_dir, PATH_MAX, "%s%s", work_dir, objects_dir);
    snprintf(path_to_tempfile, PATH_MAX, "%s/tempfile", work_obj_dir);

    uint8_t binary_hash[CHUNK];

    SHA1_CTX sha;
    SHA1Init(&sha);

    FILE* f = fopen(path, "rb");
    if (!f)
        assert("Cannot open file");
    FILE* tempfile = tmpfile();
    if (!tempfile)
        assert("Cannot create tempfile");

    char read_buffer[CHUNK];

    // формируем хедер
    char header[64];
    uint32_t h_len = snprintf(header, sizeof(header), "blob %ld", st->st_size);
    h_len++;

    SHA1Update(&sha, (uint8_t*)header, h_len); // хешируем хедер блоба
    fwrite(header, sizeof(char), h_len, tempfile);

    size_t bytes_read;
    while ((bytes_read = fread(read_buffer, sizeof(uint8_t), CHUNK, f)) > 0) {
        SHA1Update(&sha, (uint8_t*)read_buffer, bytes_read);
        fwrite(read_buffer, sizeof(uint8_t), bytes_read, tempfile);
    }
    SHA1Final(binary_hash, &sha);

    for (int i = 0; i < 20; i++)
        sprintf(ohash + (i * 2), "%02x", binary_hash[i]);
    ohash[40] = '\0';

    char blob_dir_name[3];
    for (int i = 0; i < 2; i++)
        blob_dir_name[i] = ohash[i];
    blob_dir_name[2] = '\0';
    snprintf(path_to_blob_dir, PATH_MAX, "%s/%s", work_obj_dir, blob_dir_name);
    mkdir(path_to_blob_dir, 0777);

    const char* blob_file_name = ohash + 2;
    snprintf(path_to_blob, PATH_MAX, "%s/%s", path_to_blob_dir, blob_file_name);

    FILE* blob = fopen(path_to_blob, "wb");
    if (!blob)
        assert("Cannot create blob");
    rewind(tempfile); // return cursor
    def(tempfile, blob, Z_DEFAULT_COMPRESSION);

    printf("%s\n", ohash);

    fclose(f);
    fclose(tempfile);
    remove(path_to_tempfile);
    fclose(blob);
    return 0;
}

static void get_dirname(const char* path, char* dst) {
    char* slash = strchr(path, '/');
    if (slash) {
        size_t len = slash - path;
        strncpy(dst, path, len);
        dst[len] = '\0';
    }
}

void write_tree(const indexEntry* entries, const int entries_amt, int path_offset, char* ohash) {
    FILE* tree_obj = tmpfile();
    int i = 0;
    while (i < entries_amt) {
        const char* path = entries[i].path + path_offset;
        char* slash = strchr(path, '/');
        // if in root dir
        if (!slash) {
            fprintf(tree_obj, "100644 blob %s %s\n", entries[i].hash, path);
            printf("100644 blob %s %s\n", entries[i].hash, path);
            i++;
        } else { // tree
            char dirname[256];
            get_dirname(path, dirname);
            int dirname_len = strlen(dirname);
            int start = i;
            int cnt = 0;
            printf("040000 tree %s\n", dirname);
            while (i < entries_amt) {
                path = entries[i].path + path_offset;
                printf("%s\n", path);
                if (strchr(path, '/') == NULL)
                    break;
                char curr_dirname[256];
                get_dirname(path, curr_dirname);
                if (strcmp(dirname, curr_dirname) != 0)
                    break;
                cnt++;
                i++;
            }
            char hash[41];
            write_tree(entries + start, cnt, path_offset + dirname_len + 1, hash);
        }
    }
    fclose(tree_obj);
}
