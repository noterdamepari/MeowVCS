#include "meow.h"

#define AVLTYPE TreeEntry
#define TYPE2   avlTree**

// typedef struct avlTree_t {
//     struct avlTree_t* child[2]; // child0 - left, child1 - right
//     AVLTYPE value;
//     int32_t height;
// }avlTree;

typedef struct {
    TYPE2* arr;
    int current;
} stack;

static stack* init_stack(int amt){
    stack* res = (stack*)malloc(sizeof(stack));
    res->arr = (TYPE2*)malloc(sizeof(TYPE2) * amt);
    res->current = 0;
    return res;
}

static void push(stack* stk, TYPE2 value){
    stk->arr[stk->current++] = value;
}

static TYPE2 pop(stack* stk){
    return stk->arr[--stk->current];
}

static void del_stack(stack* stk){
    free(stk->arr);
    free(stk);
}

avlTree* avl_create(AVLTYPE value){
    avlTree* tmp = (avlTree*)malloc(sizeof(avlTree));
    tmp->value = value;
    tmp->height = 0;
    tmp->child[0] = NULL;
    tmp->child[1] = NULL;
    return tmp;
}

void avl_del_tree(avlTree* tree){
    if(tree == NULL) 
        return;
    avl_del_tree(tree->child[0]);
    avl_del_tree(tree->child[1]);
    free(tree);
}

static int32_t get_height(avlTree* tree){
    return (tree == NULL) ? -1 : tree->height; // if node exist return height, else return -1
}

static void height_update(avlTree* tree){
    int32_t h0 = get_height(tree->child[0]);
    int32_t h1 = get_height(tree->child[1]);
    tree->height = (h0 > h1 ? h0 : h1) + 1;
}

static int32_t get_balance(avlTree* tree){
    return (tree == NULL) ? 0 : (get_height(tree->child[0]) - get_height(tree->child[1])); // if negative - right rotation, positive - left 
}

static char avl_rotate(avlTree** tree, int8_t side){
    avlTree* root = *tree;
    if (!root)
        return 2; // tree doesn`t exists

    avlTree* new_root = root->child[!side];
    root->child[!side] = new_root->child[side];
    new_root->child[side] = root;
    *tree = new_root;
    height_update(root);
    height_update(new_root);
    return 0;
}

static void avl_rebalance(avlTree** tree){
    int8_t balance = get_balance(*tree);
    if (balance >= -1 && balance <= 1)
        return; // ok
    if (balance <= -2){
        int8_t child_balance = get_balance((*tree)->child[1]);
        if (child_balance > 0){
            avl_rotate(&((*tree)->child[1]), 1);
        }
        avl_rotate(tree, 0);
    } else if (balance >= 2){
        int8_t child_balance = get_balance((*tree)->child[0]);
        if (child_balance < 0){
            avl_rotate(&((*tree)->child[0]), 0);
        }
        avl_rotate(tree, 1);
    }
}

char avl_insert(avlTree** tree, AVLTYPE value){
    avlTree** node = tree;
    stack* stk = init_stack(64);
    while (*node){
        if (strcmp(value.name, (*node)->value.name) == 0){
            del_stack(stk);
            return 1; // dupe
        }
        if (strcmp(value.name, (*node)->value.name) < 0){
            push(stk, node);
            node = &((*node)->child[0]);
        } else {
            push(stk,node);
            node = &((*node)->child[1]);
        }
    }
    *node = avl_create(value);
    while(stk->current > 0){
        node = pop(stk);
        height_update(*node);
        avl_rebalance(node);
    }
    del_stack(stk);
    return 0;
}

char avl_erase(avlTree** tree, AVLTYPE value){
    avlTree** node = tree;
    stack* stk = init_stack(64);
    while (*node){
        if (strcmp(value.name, (*node)->value.name) == 0){
            if ((*node)->child[0] == NULL){ // only one child on right side
                avlTree* tmp = *node;
                *node = tmp->child[1];
                free(tmp);
            } else if ((*node)->child[1] == NULL) { // only one child on left side
                avlTree* tmp = *node;
                *node = tmp->child[0];
                free(tmp);
            } else { // two childs
                avlTree** ptr = &((*node)->child[1]);
                push(stk, node);
                while ((*ptr)->child[0]){
                    push(stk, ptr);
                    ptr = &((*ptr)->child[0]);
                }
                (*node)->value = (*ptr)->value;
                avlTree* tmp = *ptr;
                *ptr = tmp->child[1];
                free(tmp);
            }

            while(stk->current > 0){ // avl_rebalance cycle
                node = pop(stk);
                height_update(*node);
                avl_rebalance(node);
            }
            del_stack(stk);
            return 0; // deleted
        } else if (strcmp(value.name, (*node)->value.name) < 0){
            push(stk, node);
            node = &((*node)->child[0]);
        } else {
            push(stk,node);
            node = &((*node)->child[1]);
        }
    }
    del_stack(stk);
    return 1; // miss
}

void avl_traverse(avlTree* tree){
    if (!tree) return;
    avl_traverse(tree->child[0]);
    printf("%s\n", tree->value.name);
    avl_traverse(tree->child[1]);
}

