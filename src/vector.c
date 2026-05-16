#include "vector.h"

vector* init_vector() {
    vector* res = (vector*)malloc(sizeof(vector));
    res->buffer = (TYPE*)malloc(DEFSIZE * sizeof(TYPE));
    res->size = 0;
    res->cap = DEFSIZE;
    return res;
}

void add_to_vector(vector* arr, TYPE x) {
    if (arr->size == arr->cap) {
        arr->cap *= 2;
        arr->buffer = (TYPE*)realloc(arr->buffer, arr->cap * sizeof(TYPE));
    }
    arr->buffer[arr->size++] = x;
}

void destroy_vector(vector* vec) {
    free(vec->buffer);
    free(vec);
}
