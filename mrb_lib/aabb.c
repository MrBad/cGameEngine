#include "aabb.h"
#include <assert.h>
#include <stdio.h>

void printAABB(AABB limits) 
{
    printf("{ %8.2f, %8.2f, %8.2f, %8.2f }\n",
            limits.minX, limits.minY, limits.maxX, limits.maxY);
}

bool aabbIntersects(AABB *a, AABB *b)
{
    assert(a);
    assert(b);
    assert(a->minX < a->maxX);
    assert(a->minY < a->maxY);
    assert(b->minX < b->maxX);
    assert(b->minY < b->maxY);

    return (a->minX < b->maxX &&
            a->maxX > b->minX &&
            a->minY < b->maxY &&
            a->maxY > b->minY);
}

// a fits inside b ?!
bool aabbFitsIn(AABB a, AABB b)
{
    return (b.minX < a.minX &&
            b.maxX >= a.maxX &&
            b.minY < a.minY &&
            b.maxY >= a.maxY);
}

#ifdef COMPILE_TESTS
void aabbTest() 
{
    AABB myAABB[] = {
        aabb(0, 0, 200, 200),
        aabb(-10, -10, 20, 20),
        aabb(-20, -20, -10, -10),
        aabb(-100, -100, -15, 15)
    };

    printf("Testing AABB\n");
    assert(aabbIntersects(&myAABB[0], &myAABB[1]));
    assert(!aabbIntersects(&myAABB[0], &myAABB[2]));
    assert(!aabbIntersects(&myAABB[1], &myAABB[2]));
    assert(!aabbIntersects(&myAABB[0], &myAABB[3]));
    assert(!aabbIntersects(&myAABB[1], &myAABB[3]));
    assert(aabbIntersects(&myAABB[2], &myAABB[3]));

    assert(aabbFitsIn(aabb(10, 10, 20, 20), aabb(0, 0, 40, 40)));
    assert(!aabbFitsIn(aabb(0, 0, 40, 40), aabb(10, 10, 20, 20)));
}
#endif // COMPILE_TESTS

