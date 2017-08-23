#ifndef USER_H
#define USER_H
#include "mrb_lib/sprite.h"
#include "mrb_lib/vec2f.h"

#define USER_WIDTH 60
#define USER_HEIGHT 60

typedef enum {
	NONE,
	HUMAN,
	PLAYER,
	ZOMBIE
} UserType;

typedef struct {
	Vec2f pos;
	float speed;
	Sprite *sprite;
	UserType type;
} User;

User *userNew(Vec2f pos, float speed, Sprite *sprite, UserType type);
void userDelete(User *user);


#endif
