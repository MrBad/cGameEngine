#include "aabb.h"
#include <assert.h>
#include <stdio.h>


bool aabbIntersect(AABB *a, AABB *b) 
{
	assert(a->minX < a->maxX);
	assert(a->minY < a->maxY);
	assert(b->minX < b->maxX);
	assert(b->minY < b->maxY);


	return	a->minX < b->maxX && a->maxX > b->minX &&
			a->minY < b->maxY && a->maxY > b->minY; 

}

// a fits inside b ?!
bool aabbFitsInAABB(AABB a, AABB b) 
{
	return b.minX < a.minX
		&& b.maxX > a.maxX
		&& b.minY < a.minY
		&& b.maxY > a.maxY;
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
	assert(aabbIntersect(&myAABB[0], &myAABB[1]));
	assert(!aabbIntersect(&myAABB[0], &myAABB[2]));
	assert(!aabbIntersect(&myAABB[1], &myAABB[2]));
	assert(!aabbIntersect(&myAABB[0], &myAABB[3]));
	assert(!aabbIntersect(&myAABB[1], &myAABB[3]));
	assert(aabbIntersect(&myAABB[2], &myAABB[3]));

	assert(aabbFitsInAABB(aabb(10, 10, 20, 20), aabb(0, 0, 40, 40)));
	assert(!aabbFitsInAABB(aabb(0, 0, 40, 40), aabb(10, 10, 20, 20)));
}

#endif

