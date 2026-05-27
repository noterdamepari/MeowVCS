#include "cmds.h"
#include "meow.h"
#include "types.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char* default_dir = "/.meow";
const char* objects_dir = "/objects";

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Error: No arguments\n");
        fprintf(stderr, "Usage: mw <command> [args]\n");
        return 1;
    }

    // init
    if (!strcmp(argv[1], "init")) {
        meow_init();
        return 0;
    }

    // status
    else if (!strcmp(argv[1], "status")) {
        meow_status();
        return 0;
    }

    // branch
    else if (!strcmp(argv[1], "branch")) {
        if (argc == 2) {
            get_branches();
            return 0;
        } else if (argc == 3) {
            meow_branch(argv[2]);
            return 0;
        }
    }

    // add
    else if (!strcmp(argv[1], "add")) {
        if (argc == 3) {
            meow_add(argv[2], STAGED);
            return 0;
        } else {
            fprintf(stderr, "Error: mw add takes only 1 argument now(\n");
            return 1;
        }
    }

    // rm
    else if (!strcmp(argv[1], "rm")) {
        if (argc == 4 && !strcmp(argv[2], "--cached")) {
            meow_rm(argv[3], CACHED);
            return 0;
        } else if (argc == 3) {
            meow_rm(argv[2], NO_CACHED);
            return 0;
        } else {
            fprintf(stderr, "Error: mw rm takes only 1 argument now(\n");
            return 1;
        }
    }

    // log
    else if (!strcmp(argv[1], "log")) {
        if (argc == 2) {
            meow_log(-1, NULL, NULL);
            return 0;

        } else if (argc == 3) {
            char* sep = strchr(argv[2], '.');
            if (!sep) {
                meow_log(-1, NULL, argv[2]);
                return 0;
            } else {
                *sep = '\0';
                char* hash2 = sep + 2;
                meow_log(-1, argv[2], hash2);
                return 0;
            }
        } else if (argc == 4) {
            if (!strcmp(argv[2], "--n")) {
                meow_log(atoi(argv[3]), NULL, NULL);
                return 0;
            }
        } else if (argc == 5) {
            if (!strcmp(argv[2], "--n")) {
                meow_log(atoi(argv[3]), NULL, argv[4]);
                return 0;
            } else if (!strcmp(argv[3], "--n")) {
                meow_log(atoi(argv[4]), NULL, argv[2]);
                return 0;
            }
        } else {
            fprintf(stderr, "Error: bad usage\n");
            return 1;
        }
    }

    // commit
    else if (!strcmp(argv[1], "commit")) {
        if (argc > 3 && !strcmp(argv[2], "-m")) {
            if (argc != 4) {
                fprintf(stderr, "Error: Сomment not found\n");
                return 1;
            } else {
                meow_commit(argv[3]);
                return 0;
            }
        } else {
            fprintf(stderr, "Error: bad usage\n");
            return 1;
        }
    }

    // checkout
    else if (!strcmp(argv[1], "checkout")) {
        if (argc == 3) {
            meow_checkout(argv[2]);
            return 0;
        } else if (argc == 5 && !strcmp(argv[3], "--")) {
            meow_checkout_file(argv[2], argv[4]);
            return 0;
        } else {
            fprintf(stderr, "Error: bad usage\n");
            return 1;
        }
    }

    // diff
    else if (!strcmp(argv[1], "diff")) {
        if (argc != 4) {
            fprintf(stderr, "Error: bad usage\n mw diff <commit1> <commit2>\n");
            return 1;
        }
        meow_diff(argv[2], argv[3]);
        return 0;
    }

    // unknown cmd
    else {
        fprintf(stderr, "Error: unknown command");
        return 1;
    }

    return 0;
}
