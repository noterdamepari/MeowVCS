#include "misc.h"
#include "meow.h"
#include "object.h"
#include "types.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

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
    if (output)
        output[0] = '\0';
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

void get_tree(char* commit, char* tree, char* work_dir) {
    char path_commit_src[PATH_MAX];

    snprintf(path_commit_src, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, commit, commit + 2);
    FILE* inflated_commit_src = fopen_inflated(path_commit_src);
    fscanf(inflated_commit_src, "%*s %*ld");
    fgetc(inflated_commit_src);
    fscanf(inflated_commit_src, "%*s %s", tree);
    LOG("tree %s\n", tree);

    fclose(inflated_commit_src);
}

void get_msg(char* commit, char* msg, char* work_dir) {
    char path_commit_src[PATH_MAX];

    snprintf(path_commit_src, PATH_MAX, "%s%s/%.2s/%s", work_dir, objects_dir, commit, commit + 2);
    FILE* inflated_commit_src = fopen_inflated(path_commit_src);
    fscanf(inflated_commit_src, "%*s %*ld");
    fgetc(inflated_commit_src);
    fscanf(inflated_commit_src, "%*s %*s");
    fgetc(inflated_commit_src);
    fscanf(inflated_commit_src, "%*s %*s");
    fgetc(inflated_commit_src);
    fscanf(inflated_commit_src, "%*s %*s %*s %*ld");
    fgetc(inflated_commit_src);
    fscanf(inflated_commit_src, "%s", msg);
    LOG("message %s\n", msg);

    fclose(inflated_commit_src);
}

FILE* fopen_inflated(char* path) {
    FILE* inflated = tmpfile();
    FILE* f = fopen_s(path, "rb");
    inf(f, inflated);
    rewind(inflated);
    fclose(f);
    return inflated;
}

FILE* fopen_s(char* path, char* modes) {
    FILE* f = fopen(path, modes);
    if (!f) {
        fprintf(stderr, "Error: cannot open file %s", path);
        exit(EXIT_FAILURE);
    }
    return f;
}

DIR* opendir_s(char* path) {
    DIR* d = opendir(path);
    if (!d) {
        fprintf(stderr, "Error: cannot open dir %s", path);
        exit(EXIT_FAILURE);
    }
    return d;
}
