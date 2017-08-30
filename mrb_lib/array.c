#include "array.h"

static bool arrayGrow(Array *arr) 
{
	int num = arr->size == 0 ? 4 : arr->size * 2;
	arr->data = realloc(arr->data, num * sizeof(*arr->data));
	if(!arr->data) {
		perror("realloc");
		return false;
	}

	memset(
			arr->data + arr->len, 0, 
			(num - arr->len) * sizeof(*arr->data));
	
	arr->size = num;
	return true;
}

void arrayPush(Array *arr, void *element)
{
	if(arr->len == arr->size) {
		arrayGrow(arr);
	}
	arr->data[arr->len++] = element;
}

void arrayDelete(Array **array)
{
	Array *arr = *array;
	if(arr) {
		free(arr->data);
		free(arr);
		arr = NULL;
	}
}

void *arrayPop(Array *arr) 
{
	if(arr->len == 0) {
		return NULL;
	}
	arr->len--;
	void *element = arr->data[arr->len];
	arr->data[arr->len] = NULL;
	return element;
}

void *arrayShift(Array *arr) 
{
	if(arr->len == 0) {
		return NULL;
	}
	arr->len--;
	void *element = arr->data[0];
	memmove(arr->data, arr->data+1, (arr->size-1) * sizeof(*arr->data));
	arr->data[arr->len] = NULL;
	return element;	
}

void arrayUnshift(Array *arr, void *element) 
{
	if(arr->len == arr->size) {
		arrayGrow(arr);
	}
	memmove(arr->data+1, arr->data, (arr->size-1) * sizeof(*arr->data));
	arr->data[0] = element;
	arr->len++;
}


int arrayCompact(Array *arr) 
{
	int i, j;
	for(i = 0, j = 0; i < arr->len; i++, j++) {
		if(!arr->data[i])
			j++;
		if(i < j)
			arr->data[i] = arr->data[j];
	}
	int shifted = j - i;
	arr->len -= shifted;
	return shifted;
	
}

int arrayIndexOf(Array *arr, void *element)
{
	int i;
	for(i = 0; i < arr->len; i++) {
		if(arr->data[i] == element) {
			return i;
			break;
		}
	}
	return -1;
}

static bool arrayValidIndex(Array *arr, int idx) 
{
	if(idx > arr->size) {
		fprintf(stderr, "arraySet: index %d out of size\n", idx);
		return false;
	}
	if(idx > arr->len) {
		fprintf(stderr, "arraySet: index %d out of lenght\n", idx);
		return false;
	}
	return true;
}

bool arraySet(Array *arr, int idx, void *element) 
{
	if(!arrayValidIndex(arr, idx))
		return false;
	arr->data[idx] = element;
	return true;	
}

void *arrayGet(Array *arr, int idx)
{
	if(!arrayValidIndex(arr, idx))
		return false;
	return arr->data[idx];
}

Array* arrayNew() 
{
	Array *arr;
	if(!(arr = malloc(sizeof(*arr)))) {
		perror("malloc");
		return NULL;
	}
	arr->len = 0;
	arr->size = 0;
	arr->data = NULL;
	return arr;
}


#ifdef COMPILE_TESTS

void arrayTest()
{
	int i;
	char *str;
	printf("Array test\n");
	Array *arr = NULL;

   	arr = arrayNew();
	arrayPush(arr, "Testing");
	arrayPush(arr, "This");
	arrayPush(arr, "Array");
	arrayPush(arr, "Thing");
	
	for(i = 0; i < arrayLen(arr); i++) {
		arrayPush(arr, arrayShift(arr));
	}
	
	assert(strcmp(arrayGet(arr, 0), "Testing") == 0);
	assert(strcmp(arrayGet(arr, 1), "This") == 0);
	assert(strcmp(arrayGet(arr, 2), "Array") == 0);
	assert(strcmp(arrayGet(arr, 3), "Thing") == 0);

	assert(strcmp(arrayShift(arr), "Testing") == 0);
	assert(arrayLen(arr) == 3);
	assert(arr->size == 4);

	assert(strcmp(arrayShift(arr), "This") == 0);
	arraySet(arr, 1, "THAT");
	assert(strcmp(arr->data[1], "THAT") == 0);
	assert(arr->len == 2);
	

	str = arrayPop(arr);
	assert(strcmp(str, "THAT") == 0);
	arrayUnshift(arr, "CUCU");	
	assert(strcmp(arrayGet(arr, 0), "CUCU") == 0);
	arrayPush(arr, "BAU");
	
	assert(arrayIndexOf(arr, "Array") == 1);
	arr->data[1] = NULL;
	arrayCompact(arr);

	assert(arr->len == 2);
	assert(strcmp(arrayGet(arr, 0), "CUCU") == 0);
	assert(strcmp(arrayGet(arr, 1), "BAU") == 0);

	arrayForeach(arr, str, i) {
		printf("{%s}\n", str);
	}

	arrayDelete(&arr);
	exit(0);
}

#endif // COMPILE_TESTS
