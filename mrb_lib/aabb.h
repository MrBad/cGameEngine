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
	return (AABB) {minX, minY, maxX, maxY};
}
bool aabbIntersect(AABB *a, AABB *b);
//bool aabbContainsAABB(AABB *a, AABB *b);
bool aabbFitsInAABB(AABB a, AABB b);
void aabbTest();

#endif
