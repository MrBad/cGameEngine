#ifndef ARRAY_H
#define ARRAY_H


#include <stdlib.h>
#include <stdbool.h>
typedef struct {
	void **data;
	size_t len;
	size_t size;
} Array;


Array *arrayNew();
void *arrayAdd(Array *a, void *element);
static inline int arrayLen(Array *arr) { return arr->len; }
static inline size_t arraySize(Array *arr) { return arr->size; }
#define arrayGet(a, i) a->data[i];
//static inline void *arrayGet(Array *arr, int i) { return arr->data[i]; }

bool arrayDelIdx(Array *a, size_t idx);
bool arrayDel(Array *arr, void *element);
void arrayDelete(Array *arr);

void arrayTest();

#endif // ARRAY_H

