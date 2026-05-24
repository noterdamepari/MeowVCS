#ifndef TREE_H
#define TREE_H
#include "types.h"
void write_tree(const indexMeta* entries, char** entries_path, const int entries_amt,
                int path_offset, char* ohash);
void walk_tree_diff(char* src_hash, char* target_hash, char* prefix, ProjectContext* p_ctx,
                    DiffCallbacks* cbs);
#endif
