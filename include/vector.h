#ifndef VECTOR_H
#define VECTOR_H

#include "stdlib.h"
#include "types.h"
#define TYPE indexEntry
#define DEFSIZE 16

typedef struct {
    TYPE* buffer;
    int size;
    int cap;
} vector;

vector* init_vector();
void add_to_vector(vector* arr, TYPE x);
void destroy_vector(vector* vec);

#endif
