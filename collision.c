#include <math.h>
#include "collision.h"
#include "user.h"
#include "game.h"
#include "mrb_lib/vec2f.h"
#include "mrb_lib/quad_tree.h"


// bounding check fast collision check
inline bool isColliding(Rect *a, Rect *b) 
{
	return (!(
				(a->x >= b->x + b->width) || 
				(a->x + a->width <= b->x) ||
				(a->y >= b->y + b->height) || 
				(a->y+a->height <= b->y)
			));
}


Vec2f collisionCheck(Rect *a, Rect *b)
{
	Vec2f centerA = {a->x + a->width / 2, a->y + a->height / 2};
	Vec2f centerB = {b->x + b->width / 2, b->y + b->height / 2};
	Vec2f distVec = vec2fSub(centerB, centerA);
	return distVec;
}

Vec2f getDistance(Rect *a, Rect *b)
{
	Vec2f centerA = {a->x + a->width / 2, a->y + a->height / 2};
	Vec2f centerB = {b->x + b->width / 2, b->y + b->height / 2};
	Vec2f distVec = vec2fSub(centerB, centerA);
	return distVec;
}


void userBrickCollision(User *user, Sprite *brick) 
{
	Vec2f distance;
	Vec2f newPos;
	Rect a = userGetRect(user);
	Rect b = {brick->x, brick->y, brick->width, brick->height};
	distance = getDistance(&a, &b);
	newPos.x = user->pos.x;
	newPos.y = user->pos.y;
	if(fabs(distance.x) > fabs(distance.y)) {
		if(distance.x < 0) { // left collision
			newPos.x = b.x + b.width;
		} else if (distance.x > 0) { // right collision
			newPos.x = b.x - USER_WIDTH;
		}
	}
	else {
		if(distance.y < 0) { // bottom collision
			newPos.y = b.y + b.height;
		} else if(distance.y > 0) {
			newPos.y = b.y - USER_HEIGHT;
		}
	}
	userSetPos(user, newPos);
}

void userUserCollision(User *a, User *b, Game *game)
{
	Vec2f distVec = vec2fSub(a->pos, b->pos);
	if(distVec.x * distVec.x + distVec.y * distVec.y < USER_WIDTH * USER_WIDTH) {
		float minDistance = USER_WIDTH;
		float distance = vec2fLength(distVec);
		float depth = minDistance - distance;
	
		Vec2f colDeptVec = vec2fMulS(vec2fNormalize(distVec), depth);
		// if a or b is the player, don't push - instead move twice	the other player
		if(a->type == PLAYER) {
			userSetPos(b, vec2fSub(userGetPos(b), colDeptVec));
		} else if(b->type == PLAYER) {
			userSetPos(a, vec2fSub(userGetPos(a), colDeptVec));
		} else {
			colDeptVec = vec2fDivS(colDeptVec, 2.0f);
			userSetPos(a, vec2fAdd(userGetPos(a), colDeptVec));
			userSetPos(b, vec2fSub(userGetPos(b), colDeptVec));
		}

		if(a->type == ZOMBIE || b->type == ZOMBIE) {
			User *zombie = a->type == ZOMBIE ? a : b;
			User *other = a->type == ZOMBIE ? b : a;
			
			if(other->type == HUMAN) { // zombie collided with human, transform human into zombie
				listDel(game->humans, other);
				// transform human into zombie
				other->type = ZOMBIE;
				other->speed = 1.5f;
				spriteSetColor(other->sprite, &zombie->sprite->color);
				// and add it to zombie list
				listAdd(game->zombies, other);

				if(game->humans->items == 0) {
					fprintf(stdout, "YOU LOOSE\n");
					SDL_Delay(2000);
					game->state = GAME_OVER;
				}
			}

			if(other->type == PLAYER) {
				// resurect zombie - transform it to human
				listDel(game->zombies, zombie);
				// transform zombie to human
				zombie->type = HUMAN;
				zombie->speed = 1.0f;
				Color color = {rand()%128, rand()%255, rand()%128, 255};
				spriteSetColor(zombie->sprite, &color);
				listAdd(game->humans, zombie);

				if(game->zombies->items == 0) {
					fprintf(stdout, "YOU WIN!\n");
					SDL_Delay(2000);
					game->state = GAME_OVER;
				}
			}
		}
	}
}

//TODO - use quad tree to check collision
void checkAllCollisions(Game *game)
{
	int i;
	//Vec2f newPos, distance;
	Sprite *s;
	
	QuadTree *qtree;
    qtree = quadTreeNew(aabb(0, 0, game->level->maxWidth, game->level->maxHeight));

	ListNode *node; User *user;
	listForeach(game->users, node, user) {
		AABB uBox = aabb(user->pos.x, user->pos.y, user->pos.x+USER_WIDTH, user->pos.y+USER_HEIGHT);
		quadTreeAdd(qtree, uBox, user);
	}

	QTSurfaces *res = surfacesNew();
	
	listForeach(game->users, node, user) {
		AABB queryBox = aabb(user->pos.x, user->pos.y, 
				user->pos.x + USER_WIDTH, user->pos.y + USER_HEIGHT);
		quadTreeGetIntersections(qtree, queryBox, res);
		for(i = 0; i < res->items; i++) {
			userUserCollision(user, res->data[i]->data, game);
		}
		quadTreeResetResults(res);
	}
	

#if 0
	ListNode *nodeA, *nodeB;
	for(nodeA = game->users->head; nodeA; nodeA = nodeA->next) {
		for(nodeB = nodeA->next; nodeB; nodeB = nodeB->next) {
			userUserCollision(nodeA->data, nodeB->data, game);
		}	
	}
#endif

#if 0	
	// check bricks / walls collisions //
	for(i = 0; i < game->level->mapBatch->spritesLen; i++) {

		s = game->level->mapBatch->sprites[i];;	
		
		Rect a = userGetRect(game->player);
		Rect b = {s->x, s->y, s->width, s->height};	
		if(isColliding(&a, &b)) {
			userBrickCollision(game->player, s);
		}
	
		User *user; ListNode *node;
		listForeach(game->users, node, user) {
			Rect a = userGetRect(user);
			if(isColliding(&a, &b)) {
				userBrickCollision(user, s);
				user->direction = vec2fRotate(user->direction, rand() % 45);
			}
		}	
	}
#endif 
	quadTreeDelete(qtree);

    qtree = quadTreeNew(aabb(-100, -100, game->level->maxWidth+100, game->level->maxHeight+100));
	for(i = 0; i < game->level->mapBatch->spritesLen; i++) {
		s = game->level->mapBatch->sprites[i];
		AABB bBox = aabb(s->x, s->y, s->x+s->width, s->y+s->height);
		quadTreeAdd(qtree, bBox, s);
	}

	listForeach(game->users, node, user) {
		
		AABB queryBox = aabb(user->pos.x, user->pos.y, 
				user->pos.x + USER_WIDTH, user->pos.y + USER_HEIGHT);
		quadTreeGetIntersections(qtree, queryBox, res);
		for(i = 0; i < res->items; i++) {
			userBrickCollision(user, res->data[i]->data);
			user->direction = vec2fRotate(user->direction, rand() % 45);
		}
		quadTreeResetResults(res);
	}

	quadTreeFreeResults(res);
	quadTreeDelete(qtree);
}

