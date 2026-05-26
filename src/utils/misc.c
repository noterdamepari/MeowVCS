#include "misc.h"
#include "avl.h"
#include "meow.h"
#include "object.h"
#include "types.h"
#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

char is_path_absolute(char* path) {
    if (!path || path[0] == '\0')
        return 0;
    if (path[0] == '/' || path[0] == '\\')
        return 1; // Unix
    if (isalpha(path[0]) && path[1] == ':')
        return 1; // Windows
    return 0;
}

void get_commit_from_head(char* commit_hash, char* work_dir) {
    char path_to_head[PATH_MAX];
    char head[PATH_MAX];
    snprintf(path_to_head, PATH_MAX, "%s/HEAD", work_dir);
    FILE* headfile = fopen_s(path_to_head, "rb");
    fgets(head, PATH_MAX, headfile);

    if (!strncmp("ref: ", head, 5)) {
        char path_to_br[PATH_MAX];
        snprintf(path_to_br, PATH_MAX, "%s/%s", work_dir, head + 5);
        FILE* br = fopen_s(path_to_br, "rb");
        fscanf(br, "%40s", commit_hash);
        fclose(br);
    } else {
        strcpy(commit_hash, head);
    }
    fclose(headfile);
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
    if (!root || !input || !output) {
        if (output)
            output[0] = '\0';
        return -2;
    }

    if (output)
        output[0] = '\0';

    size_t root_len = strlen(root);

    while (root_len > 0 && root[root_len - 1] == '/') {
        root_len--;
    }

    if (!strncmp(input, root, root_len)) {
        const char* relative_ptr = input + root_len;

        while (*relative_ptr == '/') {
            relative_ptr++;
        }

        strcpy(output, relative_ptr);
        return 0;
    }

    return -1;
}

int is_detached_head(const char* work_dir) {
    char path_to_head[PATH_MAX];
    snprintf(path_to_head, PATH_MAX, "%s/HEAD", work_dir);

    FILE* head = fopen_s(path_to_head, "rb");

    char buffer[PATH_MAX];
    fgets(buffer, PATH_MAX, head);
    fclose(head);

    return strncmp(buffer, "ref: ", 5) != 0;
}

void rmdir_rec(char* path, char* project_dir) {
    char rel_path[PATH_MAX];
    make_path_relative(project_dir, path, rel_path);
    if (strcmp(rel_path, ".meow") == 0 || strcmp(rel_path, "./.meow") == 0 ||
        strncmp(rel_path, ".meow/", 6) == 0 || strncmp(rel_path, "./.meow/", 8) == 0) {
        return;
    }
    DIR* d = opendir_s(path);
    struct dirent* ent;
    struct stat st;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        char sub_path[PATH_MAX];
        snprintf(sub_path, PATH_MAX, "%s/%s", path, ent->d_name);
        stat(sub_path, &st);
        if (S_ISDIR(st.st_mode)) {
            rmdir_rec(sub_path, project_dir);
        } else {
            remove(sub_path);
        }
    }
    closedir(d);
    rmdir(path);
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
    fprintf(stderr, "Error: .meow/ not found\n");
    exit(EXIT_FAILURE);
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
    fgetc(inflated_commit_src);
    fgets(msg, 1024, inflated_commit_src);
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
