#ifndef STRDUP_H
#define STRDUP_H

#ifndef strdup

#include <string.h>
#include <stdlib.h>

char *strdup(const char *str)
{
    int n = strlen(str) + 1;
    char *dup = malloc(n);
    if(dup) {
        strcpy(dup, str);
    }
	dup[n] = 0;
    return dup;
}

#endif

#endif
