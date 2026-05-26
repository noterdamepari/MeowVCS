#include "cmds.h"
#include "meow.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* default_dir = "/.meow";
const char* objects_dir = "/objects";

int main(int argc, char** argv) {

    switch (argc) {
        case 2: {
            if (!strcmp(argv[1], "init"))
                meow_init();
            if (!strcmp(argv[1], "log")) {
                meow_log(-1, NULL, NULL);
            }
            if (!strcmp(argv[1], "status"))
                meow_status();
            if (!strcmp(argv[1], "branch"))
                get_branches();
            break;
        }
        case 3: {
            if (!strcmp(argv[1], "commit") && !strcmp(argv[2], "-m")) {
                fprintf(stderr, "Error: Сomment not found\n");
            }
            if (!strcmp(argv[1], "rm")) {
                meow_rm(argv[2], NO_CACHED);
            }
            if (!strcmp(argv[1], "add"))
                meow_add(argv[2], STAGED);
            if (!strcmp(argv[1], "checkout"))
                meow_checkout(argv[2]);
            if (!strcmp(argv[1], "log")) {
                char* sep = strchr(argv[2], '.');
                if (!sep) {
                    meow_log(-1, NULL, argv[2]);
                } else {
                    *sep = '\0';
                    char* hash2 = sep + 2;
                    meow_log(-1, argv[2], hash2);
                }
            }
            if (!strcmp(argv[1], "branch")) {
                meow_branch(argv[2]);
            }
            break;
        }
        case 4: {
            if (!strcmp(argv[1], "commit")) {
                if (!strcmp(argv[2], "-m")) {
                    meow_commit(argv[3]);
                } else {
                    fprintf(stderr, "Error: bad usage\n");
                    exit(EXIT_FAILURE);
                }
            }
            if (!strcmp(argv[1], "rm") && !strcmp(argv[2], "--cached")) {
                meow_rm(argv[3], CACHED);
            }
            if (!strcmp(argv[1], "diff"))
                meow_diff(argv[2], argv[3]);
            if (!strcmp(argv[1], "log")) {
                if (!strcmp(argv[2], "--n")) {
                    meow_log(atoi(argv[3]), NULL, NULL);
                }
            }
            break;
        }
        case 5: {
            if (!strcmp(argv[1], "log")) {
                if (!strcmp(argv[2], "--n")) {
                    meow_log(atoi(argv[3]), NULL, argv[4]);
                } else if (!strcmp(argv[3], "--n")) {
                    meow_log(atoi(argv[4]), NULL, argv[2]);
                }
            }
            if (!strcmp(argv[1], "checkout") && !strcmp(argv[3], "--")) {
                meow_checkout_file(argv[2], argv[4]);
            }
            break;
        }
        default: {
            fprintf(stderr, "Error: No arguments\n");
            fprintf(stderr, "Usage: mw <command> [args]\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
