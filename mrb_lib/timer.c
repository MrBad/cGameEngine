#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

struct Timer {
    unsigned int prevTicks;
    unsigned int numUpdates;
};

Timer *timerNew(unsigned int ticks)
{
    Timer *t = NULL;

    if ((t = malloc(sizeof *t))) {
        t->prevTicks = ticks;
        t->numUpdates = 0;
    }

    return t;
}

void timerDelete(Timer *t)
{
    free(t);
}

int timerUpdate(Timer *t, unsigned int ticks)
{
    unsigned int diff;
    if (!t)
        return -1;
    t->numUpdates++;
    diff = ticks - t->prevTicks;
    t->prevTicks = ticks;

    return diff;
}

