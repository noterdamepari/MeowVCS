#ifndef UNPACK_H
#define UNPACK_H
void unpack_blob(char* blob_src, char* file_dst, int mode);
void unpack_tree(char* tree_hash, char* path_to_tree, char* work_dir);
#endif
