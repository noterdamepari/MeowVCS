#include "cmds.h"
#include "meow.h"
#include <stdio.h>

const char* default_dir = "/.meow";
const char* objects_dir = "/objects";

int main(int argc, char** argv) {
    switch (argc) {
        case 2: {
            if (!strcmp(argv[1], "init"))
                meow_init();
            if (!strcmp(argv[1], "dir_traverse"))
                // write_project_dir();
                if (!strcmp(argv[1], "commit"))
                    fprintf(stderr, "Error: Сomment not found\n");
            break;
        }
        case 3: {
            if (!strcmp(argv[1], "add"))
                meow_add(argv[2]);
            if (!strcmp(argv[1], "commit"))
                meow_commit(argv[2]);
            break;
        }
        default: {
            fprintf(stderr, "Error: No arguments\n");
            break;
        }
    }
    return 0;
}
