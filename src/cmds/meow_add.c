#include "meow.h"
#include "misc.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void meow_add(char* file) {
    char work_dir[PATH_MAX];
    char path_to_index[PATH_MAX];
    char path_to_indextmp[PATH_MAX];
    char path[PATH_MAX];

    if (find_work_dir(work_dir) == -1) {
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
        fprintf(stderr, "Error: File doesn`t exists\n");
        exit(EXIT_FAILURE);
    }

    int64_t file_mtime = st.st_mtime;
    int file_mode = st.st_mode;

    snprintf(path_to_index, PATH_MAX, "%s/index", work_dir);
    snprintf(path_to_indextmp, PATH_MAX, "%s/index.tmp", work_dir);

    FILE* index = fopen(path_to_index, "rb");
    if (!index) {
        perror("Error: index not found");
        exit(EXIT_FAILURE);
    }

    FILE* index_tmp = fopen(path_to_indextmp, "wb");
    if (!index_tmp) {
        perror("Error: index.tmp not created");
        exit(EXIT_FAILURE);
    }

    fprintf(index_tmp, "0\n");
    unsigned int entries_amt;
    fscanf(index, "%u\n", &entries_amt);

    indexEntry entry;

    char project_dir[PATH_MAX];
    char rel_path[PATH_MAX];
    find_project_dir(project_dir, work_dir);
    make_path_relative(project_dir, path, rel_path);

    char inserted = 0;
    int new_entries_amt = entries_amt;

    for (int i = 0; i < entries_amt; i++) {
        fscanf(index, "%40s %o %hhu %ld %s", entry.hash, &entry.mode, &entry.status, &entry.mtime, entry.path);
        char strcmp_res = strcmp(rel_path, entry.path);
        if (!strcmp_res) { // already in index -> modified
            inserted = 1;
            if (entry.mtime != file_mtime) { // file has been changed
                char hash[41];
                create_blob(path, work_dir, &st, hash);
                entry.status = MODIFIED;
                strcpy(entry.hash, hash);
                entry.mode = file_mode;
                entry.mtime = file_mtime;
            } else {
                puts("Nothing to do, already in index");
                fclose(index_tmp);
                fclose(index);
                remove(path_to_indextmp);
                return;
            }
            fprintf(index_tmp, "%s %o %hhu %ld %s\n", entry.hash, entry.mode, entry.status, entry.mtime, entry.path);
        } else if (strcmp_res < 0 && !inserted) {
            inserted = 1;
            new_entries_amt++;

            char hash[41];
            create_blob(path, work_dir, &st, hash);
            indexEntry new_entry;
            strcpy(new_entry.hash, hash);
            strcpy(new_entry.path, rel_path);
            new_entry.status = NEW;
            new_entry.mtime = file_mtime;
            new_entry.mode = file_mode;
            fprintf(index_tmp, "%s %o %hhu %ld %s\n", new_entry.hash, new_entry.mode, new_entry.status, new_entry.mtime, new_entry.path);

            fprintf(index_tmp, "%s %o %hhu %ld %s\n", entry.hash, entry.mode, entry.status, entry.mtime, entry.path);
        } else {
            fprintf(index_tmp, "%s %o %hhu %ld %s\n", entry.hash, entry.mode, entry.status, entry.mtime, entry.path);
        }
    }

    if (!inserted) {
        char hash[41];
        create_blob(path, work_dir, &st, hash);
        strcpy(entry.hash, hash);
        strcpy(entry.path, rel_path);
        entry.status = 1;
        entry.mtime = file_mtime;
        entry.mode = file_mode;
        fprintf(index_tmp, "%s %o %hhu %ld %s\n", entry.hash, entry.mode, entry.status, entry.mtime, entry.path);
        new_entries_amt++;
    }
    LOG("%s added to index\n with %o mode", rel_path, entry.mode);

    rewind(index_tmp);
    fprintf(index_tmp, "%u\n", new_entries_amt);

    fclose(index_tmp);
    fclose(index);

    if (rename(path_to_indextmp, path_to_index) != 0) {
        perror("Error: rename failed");
        exit(EXIT_FAILURE);
        return;
    }
}
