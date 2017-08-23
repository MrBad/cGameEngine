#include <math.h>
#include "collision.h"
#include "mrb_lib/vec2f.h"

// bounding check fast collision check
bool isColliding(Rect *a, Rect *b) 
{
	return (!(
				(a->x > b->x + b->width) || 
				(a->x + a->width < b->x) ||
				(a->y > b->y + b->height) || 
				(a->y+a->height < b->y)
	));
}


Vec2f collisionCheck(Rect *a, Rect *b)
{
	Vec2f centerA = {a->x + a->width / 2, a->y + a->height / 2};
	Vec2f centerB = {b->x + b->width / 2, b->y + b->height / 2};
	Vec2f distVec = vec2fSub(centerB, centerA);
	return distVec;
}
