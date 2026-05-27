#ifndef INDEX_H
#define INDEX_H
#include "avl.h"
#include "types.h"
void update_index_from_tree(char* target_hash, ProjectContext* p_ctx);
avlTree* open_index(char* work_dir, unsigned int* entries_amt);
void save_index(avlTree* idx, char* work_dir, unsigned int* entries_amt);
void close_index(avlTree* idx);
#endif
