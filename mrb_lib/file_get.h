#ifndef FILE_GET_H
#define FILE_GET_H


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// returns size or -1 on error
// content will be placed in alloced buffer
unsigned char *file_get(const char *path, int *size);

#endif
