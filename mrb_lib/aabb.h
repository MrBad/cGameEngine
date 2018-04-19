#ifndef AABB_H
#define AABB_H

#include <stdbool.h>
#include <assert.h>

typedef struct {
    float minX;
    float minY;
    float maxX;
    float maxY;
} AABB;

static inline AABB aabb(float minX, float minY, float maxX, float maxY) 
{
    assert(minX < maxX);
    assert(minY < maxY);

    return (AABB) { minX, minY, maxX, maxY };
}

/**
 * Prints an AABB
 *
 * @param limits The AABB
 */
void printAABB(AABB limits);

/**
 * Checks if a intersects b
 *
 * @param a Reference to first AABB
 * @param b Reference to second AABB
 * @return true if they intersect
 */
bool aabbIntersects(AABB *a, AABB *b);

/**
 * Checks if a fits in b
 *
 * @param a first AABB
 * @param b second AABB
 * @return true if a fits in b, false otherwise
 */
bool aabbFitsIn(AABB a, AABB b);

/**
 * Internal self test
 */
void aabbTest();

#endif // AABB_H

