#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>

typedef struct Array Array;

struct Array {
    void **data;
    int len;
    int size;
};

/**
 * Creates a new array
 * 
 * @return new array
 */
Array *arrayNew();

/**
 * Destroys an array
 * 
 * @param array The array to destroy
 */
void arrayDelete(Array **array);

/**
 * Pushes an element to the end of an array
 *
 * @param arr The array to push into
 * @param element The element to push
 */
void arrayPush(Array *arr, void *element);

/**
 * Get the last element from the array
 * 
 * @param arr The array to pop from
 * @return The poped element
 */
void *arrayPop(Array *arr);

/**
 * Extracts the first element of the array
 *
 * @param arr The array to shift from
 * @return The shifted element
 */
void *arrayShift(Array *arr);

void arrayUnshift(Array *arr, void *element);

int arrayCompact(Array *arr);

/**
 * Scans the array for an element
 *
 * @param arr The array
 * @param element The searched element
 * @return the index of the element or -1 on failure
 */
int arrayIndexOf(Array *arr, void *element);

/**
 * Sets the array index idx with an element
 *
 * @param arr The array
 * @param idx Index in the array
 * @param element the element to set
 * @return true if set, false on error (invalid index)
 */
bool arraySet(Array *arr, int idx, void *element);

bool arrayUnset(Array *arr, int idx);

/**
 * Gets the element on index idx
 *
 * @param arr The array
 * @param idx The index
 * @return the element at that index, of NULL on error (invalid index)
 */
void *arrayGet(Array *arr, int idx);

/**
 * Gets the number of elements in the array
 *
 * @param arr The array
 * @return the number of elements
 */
static inline int arrayLen(Array *arr) { return arr->len; }

int arrayAdd(Array *arr, void *element);

/**
 * Resets the array, cleaning it's slots and making the length zero.
 * It does not free the buffer, so it can be use later without realloc
 * @param arr The array
 */
void arrayReset(Array *arr);

/**
 * Helper to loop in the array
 */
#define arrayForEach(array, element, idx) \
    for(idx = 0; idx < array->len && ((element = array->data[idx]) || true); idx++)

void arrayTest();

#endif

