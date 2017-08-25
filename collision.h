#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "mrb_lib/sprite.h"
#include "mrb_lib/vec2f.h"

typedef enum {
	COLLIDE_NONE,
	COLLIDE_TOP,
	COLLIDE_BOTTOM,
	COLLIDE_LEFT,
	COLLIDE_RIGHT,
} CollisionType;


bool isColliding(Rect *a, Rect *b);
Vec2f collisionCheck(Rect *a, Rect *b);
Vec2f getDistance(Rect *a, Rect *b);
#endif
