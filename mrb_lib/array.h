#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


typedef struct Array Array;

struct Array {
	void **data;
	int len; 
	int size;
};


Array* arrayNew();

void arrayDelete(Array **array);

void arrayPush(Array *arr, void *element);
void *arrayPop(Array *arr);

void *arrayShift(Array *arr);
void arrayUnshift(Array *arr, void *element);

int arrayCompact(Array *arr);

int arrayIndexOf(Array *arr, void *element);

bool arraySet(Array *arr, int idx, void *element);

void *arrayGet(Array *arr, int idx);

static inline int arrayLen(Array *arr) { return arr->len; }

#define arrayForeach(array, element, idx) \
	for(idx = 0; idx < arr->len && ((element = arr->data[idx]) || true); idx++)

void arrayTest();


#endif
