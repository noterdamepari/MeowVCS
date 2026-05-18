#include "meow.h"
#include "object.h"
#include "types.h"
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

static void get_dirname(const char* path, char* dst) {
    char* slash = strchr(path, '/');
    if (slash) {
        size_t len = slash - path;
        strncpy(dst, path, len);
        dst[len] = '\0';
    }
}

char create_blob(char* path, char* work_dir, struct stat* st, char* ohash) {
    char work_obj_dir[PATH_MAX];
    char path_to_tempfile[PATH_MAX];

    snprintf(work_obj_dir, PATH_MAX, "%s%s", work_dir, objects_dir);
    snprintf(path_to_tempfile, PATH_MAX, "%s/tempfile", work_obj_dir);

    FILE* f = fopen(path, "rb");
    if (!f) {
        puts("Cannot open file");
        return 1;
    }

    hash_and_create_obj(BLOB, f, ohash);

    fclose(f);
    remove(path_to_tempfile);
    return 0;
}

void write_tree(const indexMeta* entries, char** entries_paths, const int entries_amt, int path_offset, char* ohash) {
    FILE* tmp_tree_obj = tmpfile();
    if (!tmp_tree_obj) {
        perror("Error: Cannot create tempfile in write_tree func");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (i < entries_amt) {
        if (entries[i].fstatus == DELETED)
            continue;
        const char* path = entries_paths[i] + path_offset;
        char* slash = strchr(path, '/');

        if (!slash) {
            // if in root dir
            fprintf(tmp_tree_obj, "%o blob %s %s\n", entries[i].mode, entries[i].hash, path);
            LOG("%o blob %s %s\n", entries[i].mode, entries[i].hash, path);
            i++;
        } else {
            // subdir
            char dirname[256];
            get_dirname(path, dirname);
            int dirname_len = strlen(dirname);
            int start = i;
            int cnt = 0;
            while (i < entries_amt) {
                path = entries_paths[i] + path_offset;
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
            write_tree(entries + start, entries_paths + start, cnt, path_offset + dirname_len + 1, hash);
            fprintf(tmp_tree_obj, "040000 tree %s %s\n", hash, dirname);
            LOG("040000 tree %s %s\n", hash, dirname);
        }
    }

    fflush(tmp_tree_obj);
    rewind(tmp_tree_obj);

    hash_and_create_obj(TREE, tmp_tree_obj, ohash);

    fclose(tmp_tree_obj);
}
