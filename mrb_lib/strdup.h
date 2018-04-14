#ifndef STRDUP_H
#define STRDUP_H

#ifndef strdup

#include <string.h>
#include <stdlib.h>

char *strdup(const char *str)
{
    int n = strlen(str);
    char *dup = malloc(n + 1);
    if (dup) {
        strcpy(dup, str);
    }

    return dup;
}

#endif

#endif
