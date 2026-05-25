#ifndef TREE_H
#define TREE_H
#include "types.h"
#include <stdio.h>
void write_tree(const indexMeta* entries, char** entries_path, const int entries_amt,
                int path_offset, char* ohash);
void walk_tree_diff(char* src_hash, char* target_hash, char* prefix, ProjectContext* p_ctx,
                    DiffCallbacks* cbs);
int read_tree_entry(FILE* tree, TreeEntry* entry);
int find_obj_in_tree(char* hash, char* rel_path, TreeEntry* out, char* work_dir);
#endif
