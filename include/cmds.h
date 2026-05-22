#ifndef CMDS_H
#define CMDS_H

void meow_init();
void meow_commit(char* msg);
void meow_add(char* file);
void meow_status();
void meow_diff(char* source_commit, char* target_commit);
#endif
