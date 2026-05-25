#ifndef CMDS_H
#define CMDS_H

#include "types.h"
void meow_init();
void meow_commit(char* msg);
void meow_add(char* file, stage_status status);
void meow_status();
void meow_diff(char* source_commit, char* target_commit);
void meow_log(int n, char* to, char* from);
void meow_checkout(char* target);
void meow_checkout_file(char* target, char* path);
#endif
