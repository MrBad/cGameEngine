#include "user.h"


User *userNew(Vec2f pos, float speed, Sprite *sprite, UserType type) 
{
	User *user = NULL;
	if(!(user = malloc(sizeof(*user)))) {
		fprintf(stderr, "Cannot alloc user\n");
		return NULL;
	}

	user->pos = pos;
	user->speed = speed;
	user->sprite = sprite;
	user->type = type;

	return user;
}

void userDelete(User *user) 
{
	free(user);
}

