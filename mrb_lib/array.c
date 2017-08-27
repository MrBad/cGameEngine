#include "array.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>


Array *arrayNew() 
{
	Array *a = NULL;
	if(!(a = malloc(sizeof(*a)))) {
		fprintf(stderr, "Cannot alloc Array\n");
		return NULL;
	}
	a->size = 0;
	a->len = 0;
	a->data = NULL;
	return a;
}

void *arrayAdd(Array *a, void *element) 
{
	if(a->len == a->size) {
		size_t numElements = a->size == 0 ? 2 : a->size * 2;
		a->data = realloc(a->data, numElements * sizeof(*a->data));
		if(!a->data) {
			fprintf(stderr, "Cannot realloc Array!\n");
			return NULL;
		}
		while(a->size < numElements)
			a->data[a->size++] = 0;
	}
	a->data[a->len++] = element;
	return element;
}

// BUG - it is not safe to loop through all indexes and delete them as 
// you go... the length will be automagically updated and i will skip...
// Maybe i will just need to mark as null the index and never consolidate data...
// or make a separate function like slice in js
// or have a special iterator... or... blah
bool arrayDelIdx(Array *a, size_t idx)  
{
	size_t i;
	if(idx > a->len) {
		fprintf(stderr, "index: %lu outside of array size\n", idx);
		return false;
	}
	if(idx > a->len && idx < a->size) {
		fprintf(stderr, "index: %lu already deleted\n", idx);
		return false;
	}
	if(idx > a->size) {
		fprintf(stderr, "index: %lu outside of array size\n", idx);
		return false;	
	}

	for(i = idx; i < a->len; i++) {
		a->data[i] = i+1 < a->len && a->data[i+1] ? a->data[i+1] : 0;
	}
	a->len--;
	return true;
}

bool arrayDel(Array *arr, void *element)
{
	size_t i;
	bool found = false;
	for(i = 0; i < arr->len; i++) {
		if(arr->data[i] == element) {
			found = true;
			break;
		}
	}
	if(found)
		found &= arrayDelIdx(arr, i);
	return found;
}

void arrayDelete(Array *arr) {
	if(arr->size > 0)
		free(arr->data);
	arr->data = NULL;
	arr->len = 0;
	arr->size = 0;
	free(arr);
}

#define arrayForEach(array, element, idx) \
	for(idx = 0; idx < array->len; i++)


// find all NULLs int array and shift them
// Recompute the length of array
// Ideea is to null elements into array and then call compact
// Because it is not safe to delete an element inside a loop
int arrayCompact(Array *arr) 
{
	size_t i, j;

	for(i = 0, j = 0; i < arr->len; i++, j++) {
		if(!arr->data[i]) {
			j++;
		}
		if(i < j)
			arr->data[i] = arr->data[j];
	}
	int shifted = j - i;
	arr->len -= shifted;
	return shifted;
}

int arrayIndexOf(Array *arr, void *element) 
{
	size_t i;
	for(i = 0; i < arr->len; i++) {
		if(arr->data[i] == element) {
			return i;
			break;
		}
	}
	return -1;
}


#ifdef COMPILE_TESTS

typedef struct {
	char *fname;
	char *lname;
	int age;
} Employee;

#define LEN(Arr) sizeof(Arr) / sizeof(*Arr)

void arrayTest() 
{	

	fprintf(stdout, "Testing arrays\n");
	Employee employees[] = {
		{"John", "Doe", 20},
		{"Jane", "Doe", 25},
		{"Popescu", "Ion", 40},
		{"Vasile", "Didi", 20},
		{"Gheorghe", "Vasile", 29},
	};

	unsigned int i;
	Array *arr = arrayNew();
	for(i = 0; i < LEN(employees); i++) {
		arrayAdd(arr, &employees[i]);
	}

	assert(arraySize(arr) == 8);
	assert(arrayLen(arr) == 5);


	//arrayDel(arr, &employees[1]);
	arr->data[1] = NULL;
	arrayCompact(arr);

	assert(arraySize(arr) == 8);
	assert(arrayLen(arr) == 4);

	assert(((Employee *)arr->data[0])->age == 20);
	assert(strcmp(((Employee *)arr->data[0])->fname, "John") == 0);
	assert(strcmp(((Employee *)arr->data[1])->fname, "Popescu") == 0);
	assert(strcmp(((Employee *)arr->data[2])->fname, "Vasile") == 0);
	assert(strcmp(((Employee *)arr->data[3])->fname, "Gheorghe") == 0);

	Employee *emp = arr->data[2];
	assert(2 == arrayIndexOf(arr, emp));

	arr->data[1] = NULL;
	arr->data[3] = NULL;
	
	int shifted = arrayCompact(arr);
	
	assert(shifted == 2);
	assert(arr->len == 2);
	
	assert(strcmp(((Employee*)arr->data[0])->fname, "John") == 0);	
	assert(strcmp(((Employee*)arr->data[1])->fname, "Vasile") == 0);

	for(i = 0; i < arr->len; i++) {
		arr->data[i] = NULL;
	}
	arrayCompact(arr);
	assert(arr->len == 0);
	assert(arr->data[0] == NULL);

	arrayDelete(arr);
}

#endif // DO_TESTS
