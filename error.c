#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
void fatalError(const char* format, ...) {
    va_list argList;

	va_start(argList, format);
    vfprintf(stderr, format, argList);
    va_end(argList);

	exit(1);
}

