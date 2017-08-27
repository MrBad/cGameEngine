#include "user.h"
#include "game.h"

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
	
	user->direction = vec2f(rand() % 100, rand() % 100);
	user->direction = vec2fNormalize(user->direction);
	if(user->direction.x == 0 && user->direction.y == 0)
		user->direction = vec2f(10, 120);
	user->velocity = vec2fMulS(user->direction, speed);
	return user;
}

void userSetPos(User *user, Vec2f pos) 
{
	user->sprite->x = pos.x;
	user->sprite->y = pos.y;
	user->pos = pos;
}
inline Vec2f userGetPos(User *user) {
	return (Vec2f) {user->pos.x, user->pos.y};
}

void userDelete(User *user) 
{
	if (user) free(user);
}

	
void initZombies(Game *game) 
{
	int i;
	Vec2f pos;
	float speed;
	Sprite *sprite; 
	Color c;
	game->zombies = listNew(NULL);
	
	for(i = 0; i < game->level->zombiesLen; i++) {	
		pos = game->level->zombiesPos[i];
		speed = 1.5f;
		sprite = spriteNew(
				pos.x, pos.y, USER_WIDTH, USER_HEIGHT,
				game->level->textures[CIRCLE_TEX]->id);
		c = color(255, 0, 0, 255);
		spriteSetColor(sprite, &c);
		listAdd(game->zombies, userNew(pos, speed, sprite, ZOMBIE));
		sbAddSprite(game->usersBatch, sprite);
	}

}

void initHumans(Game *game)
{
	int i;
	Vec2f pos;
	float speed;
	Sprite *sprite;
	Color c;
	game->humans = listNew(NULL);

	for(i = 0; i < game->level->numHumans; i++) {
		// init humans //
		pos = vec2f(
				128 + rand() % (game->level->maxWidth - 256), 
				128 + rand() % (game->level->maxHeight - 256));

		speed = 1.0f;
		sprite = spriteNew(
				pos.x, pos.y, USER_WIDTH, USER_HEIGHT,
				game->level->textures[CIRCLE_TEX]->id);
		c = color(rand() % 128, rand() % 255, rand() % 128, 255);
		spriteSetColor(sprite, &c);
		listAdd(game->humans, userNew(pos, speed, sprite, HUMAN));
		// add the sprite to users batches
		sbAddSprite(game->usersBatch, sprite);
	}
}

void initPlayer(Game *game) 
{
	Vec2f pos;
	float speed;
	Sprite *sprite;
	Color color;

	pos = game->level->playerPos;
	speed = 10.0f;
	sprite = spriteNew(
			pos.x, pos.y, USER_WIDTH, USER_HEIGHT,
			game->level->textures[CIRCLE_TEX]->id);
	color = (Color){128, 128, 255, 255};
	spriteSetColor(sprite, &color);
	game->player = userNew(pos, speed, sprite, PLAYER);
	// add the sprite to users batches
	sbAddSprite(game->usersBatch, game->player->sprite);
}

inline Rect userGetRect(User *user)
{
	return (Rect) {
		user->pos.x, user->pos.y, 
		user->sprite->width, user->sprite->height};
}



void humansUpdate(Game *game) 
{
	// update human position //
	ListNode *node; User *human;
	listForeach(game->humans, node, human) {
		if(game->totalFrames % 30 == 0) {// change his direction once half a second
			human->direction = vec2fRotate(human->direction, rand() %10);
		}
		Vec2f newPos = vec2fMulS(human->direction, human->speed);
		newPos = vec2fAdd(human->pos, newPos);
		userSetPos(human, newPos);
	}
}

User *getNearUser(Game *game, User *user) 
{
	Vec2f distVec;
	float minDistance = 0xFFFFFF, sqLen;
	User *closestUser = user;
	User *other; ListNode *node;
	listForeach(game->users, node, other) {
		if(other->type != HUMAN)
			continue;
		distVec = vec2fSub(other->pos, user->pos);
		sqLen = vec2fSquaredLength(distVec);
		if(sqLen < minDistance) {
			closestUser = other;
			minDistance = sqLen;
		}
	}
	return closestUser;
}

void zombiesUpdate(Game *game)
{
	// update zombies position - TODO find nearest human and hunt him//
	//
	ListNode *node; User *zombie;
	listForeach(game->zombies, node, zombie) {
		if(game->totalFrames % 20 == 0) {		
			User *nearHuman = getNearUser(game, zombie);
			Vec2f newDir = vec2fSub(nearHuman->pos, zombie->pos);
			zombie->direction = vec2fNormalize(newDir);
		} 
		else { 
			zombie->direction = vec2fRotate(zombie->direction, rand() %10);
		}

		Vec2f newPos = vec2fMulS(zombie->direction, zombie->speed);
		newPos = vec2fAdd(zombie->pos, newPos);
		userSetPos(zombie, newPos);
	}
}
