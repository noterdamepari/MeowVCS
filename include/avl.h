#ifndef AVL_H
#define AVL_H
#include "types.h"
#include <stdint.h>
#include <stdio.h>

#define AVLTYPE IndexTreeEntry
#define TYPE2 avlTree**

typedef struct avlTree_t {
    struct avlTree_t* child[2]; // child0 - left, child1 - right
    int32_t height;
    AVLTYPE value;
} avlTree;

typedef struct {
    TYPE2* arr;
    int current;
} stack;

avlTree* avl_create(AVLTYPE value);
char avl_insert(avlTree** tree, AVLTYPE* value);
void avl_del_tree(avlTree* tree);
void avl_traverse(avlTree* tree);
char avl_erase(avlTree** tree, AVLTYPE value);
void avl_save_to_file(avlTree* tree, FILE* f);
IndexTreeEntry* avl_find(avlTree* tree, char* path);
#endif
