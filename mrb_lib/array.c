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


	arrayDel(arr, &employees[1]);
	assert(arraySize(arr) == 8);
	assert(arrayLen(arr) == 4);

	/*
	for(i = 0; i < arrayLen(arr); i++) {
		printf("%d: %s %s %d\n", i, ((Employee*)arr->data[i])->fname, 
				((Employee *)arr->data[i])->lname, 
				((Employee *)arr->data[i])->age);	
	}
	*/

	assert(((Employee *)arr->data[0])->age == 20);
	assert(strcmp(((Employee *)arr->data[0])->fname, "John") == 0);
	assert(strcmp(((Employee *)arr->data[1])->fname, "Popescu") == 0);
	assert(strcmp(((Employee *)arr->data[2])->fname, "Vasile") == 0);
	assert(strcmp(((Employee *)arr->data[3])->fname, "Gheorghe") == 0);

	arrayDelete(arr);
}

#endif // DO_TESTS
