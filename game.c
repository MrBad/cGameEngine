#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "game.h"
#include "level.h"
#include "collision.h"
#include <math.h>
#define SIZE(a) sizeof(a)/sizeof(*a)

Game *gameNew() 
{
	Game *game;

	if(!(game = calloc(1, sizeof(*game)))) {
		fprintf(stderr, "Cannot alloc memory for Game\n");
		return NULL;
	}

	game->win = NULL;
	game->prog = NULL;
	game->cam = NULL;
	game->inmgr = NULL;
	game->level = NULL;
	game->usersBatch = NULL;
	return game;
}

static bool gameInitShaders(Game *game) 
{
	glProgramCompileShaders(game->prog, "shaders/color_shader");
	glProgramAddAttribute(game->prog, "vertexPosition");
	glProgramAddAttribute(game->prog, "vertexColor");
	glProgramLinkShaders(game->prog);
	return true;
}

bool gameInit(Game *game, int winWidth, int winHeight, const char *title) 
{

	game->state = GAME_PLAYING;
	if(!(game->win = windowNew(title, winWidth, winHeight, 0))) {
		fprintf(stderr, "Cannot init window\n");
		return false;
	}
	if(!(game->inmgr = inMgrNew())) {
		fprintf(stderr, "Cannot init Input Manager\n");
		return false;
	}
	if(!(game->cam = cameraNew(winWidth, winHeight))) {
		fprintf(stderr, "Cannot init Camera\n");
		return false;
	}
	srand(time(NULL));

	
	if(!(game->prog = glProgramNew())) {
		fprintf(stderr, "Cannot init glProgram\n");
		return false;	
	}
	gameInitShaders(game);


	// create a sprite batch
	// init the sprite batch

	if(!(game->level = levelNew("resources/level1.txt"))) {
		fprintf(stderr, "Cannot create level\n");
		return false;
	}
	if(!(loadLevel(game->level, game->prog))) {
		fprintf(stderr, "Cannot create level\n");
		return false;
	}	
	// init map sprite batch //
	sbInit(game->level->mapBatch);

	game->camSpeed = 5.0f;
	game->scaleSpeed = 1.02f;
	

	// create new user batch //
	game->usersBatch = sbNew(game->prog);
	sbInit(game->usersBatch);

	initHumans(game);
	initZombies(game);
	initPlayer(game);

	// populate game->users; //
	int i = 0, j = 0;
	game->users = malloc((game->humansLen+game->zombiesLen+1) * sizeof(* game->users));
	game->users[j++] = game->player;
	for(i = 0; i < game->humansLen; i++, j++)
		game->users[j] = game->humans[i];
	for(i = 0; i < game->zombiesLen; i++, j++)
		game->users[j] = game->zombies[i];

	cameraSetPosition(game->cam, game->player->pos.x, game->player->pos.y);
	return true;
}

void gameDelete(Game *game) 
{
	int i;
	if(game->prog) {
		glProgramDelete(game->prog);
		game->prog = NULL;
	}
	if(game->inmgr) {
		inMgrDelete(game->inmgr);
		game->inmgr = NULL;
	}
	if(game->cam) {
		cameraDelete(game->cam);
		game->cam = NULL;
	}

	for(i = 0; i < game->zombiesLen; i++) {
		if(game->zombies[i]) {
			spriteDelete(game->zombies[i]->sprite);
			userDelete(game->zombies[i]);
		}
	}
	for(i = 0; i < game->humansLen; i++) {
		if(game->humans[i])
			spriteDelete(game->humans[i]->sprite);
			userDelete(game->humans[i]);
	}

	free(game->zombies);
	free(game->humans);

	userDelete(game->player);
	
	if(game->users)
		free(game->users);
	sbDelete(game->usersBatch);
	levelDelete(game->level);

	windowDelete(game->win);
	free(game);

}

void gameHandleInput(Game *game) 
{

	InMgr *inmgr = game->inmgr;
	Camera *camera = game->cam;	
	//float camSpeed = game->camSpeed;
	float scaleSpeed = game->scaleSpeed;
	User *player = game->player;

	if(inMgrIsKeyPressed(inmgr, IM_KEY_A)) {
		Vec2f pos = { player->pos.x-player->speed, player->pos.y };
		userSetPos(player, pos);
		cameraSetPosition(camera, pos.x, pos.y);
	}
	
	if(inMgrIsKeyPressed(inmgr, IM_KEY_D)) {
		Vec2f pos = { player->pos.x + player->speed, player->pos.y };
		userSetPos(player, pos);
		cameraSetPosition(camera, pos.x, pos.y);
	}

	if(inMgrIsKeyPressed(inmgr, IM_KEY_W)) {
		Vec2f pos = { player->pos.x, player->pos.y + player->speed};
		userSetPos(player, pos);
		cameraSetPosition(camera, pos.x, pos.y);
	}
	if(inMgrIsKeyPressed(inmgr, IM_KEY_S)) {
		Vec2f pos = { player->pos.x, player->pos.y - player->speed};
		userSetPos(player, pos);
		cameraSetPosition(camera, pos.x, pos.y);
	}
	if(inMgrIsKeyPressed(inmgr, IM_KEY_Q)) {
		cameraSetScale(camera, camera->scale * scaleSpeed);
	}

	if(inMgrIsKeyPressed(inmgr, IM_KEY_E)) {
		cameraSetScale(camera, camera->scale / scaleSpeed);
	}	
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
	}
}

//TODO - use quad tree to check collision
void checkAllCollisions(Game *game)
{
	int i, j;
	//Vec2f newPos, distance;
	Sprite *s;
	
	// check bricks / walls collisions //
	for(i = 0; i < game->level->mapBatch->spritesLen; i++) {

		s = game->level->mapBatch->sprites[i];;	
		
		Rect a = userGetRect(game->player);
		Rect b = {s->x, s->y, s->width, s->height};	
		if(isColliding(&a, &b)) {
			userBrickCollision(game->player, s);
		}
		
		for(int j = 0; j < game->humansLen; j++) {
			Rect a = userGetRect(game->humans[j]);
			if(isColliding(&a, &b)) {
				userBrickCollision(game->humans[j], s);
				game->humans[j]->direction = vec2fRotate(
						game->humans[j]->direction, 
						rand()%45);
			}
		}
		for(int j = 0; j < game->zombiesLen; j++) {
			Rect a = userGetRect(game->zombies[j]);
			if(isColliding(&a, &b)) {
				userBrickCollision(game->zombies[j], s);
				game->zombies[j]->direction = vec2fRotate(
						game->zombies[j]->direction, 
						rand()%30);
			}
		}
	}

	int len = game->zombiesLen+game->humansLen+1;
	for(i = 0; i < len; i++) {
		User *a = game->users[i];
		for(j = i; j < len; j++) {
			User *b = game->users[j];
			userUserCollision(a, b, game);
		}
	}
}


void gameLoop(Game *game) 
{

	Uint32 currTicks, prevTicks;
	int numFrames = 0;



	prevTicks = currTicks = SDL_GetTicks();

	while(game->state == GAME_PLAYING) {

		currTicks = SDL_GetTicks();
		if(currTicks - prevTicks > 1000) {
			prevTicks = currTicks;
			printf("FPS: %d\n", numFrames);
			numFrames = 0;
		}
		numFrames++;


		inMgrUpdate(game->inmgr);
		if(game->inmgr->quitRequested) {
			game->state = GAME_OVER;
		}

		gameHandleInput(game);
		// update human position //
		for(int i = 0; i < game->humansLen; i++) {
			if(numFrames % 30 == 0) {// change his direction once half a second
				game->humans[i]->direction = vec2fRotate(game->humans[i]->direction, rand() %10);
			}
			Vec2f newPos = vec2fMulS(game->humans[i]->direction, game->humans[i]->speed);
			newPos = vec2fAdd(game->humans[i]->pos, newPos);
			userSetPos(game->humans[i], newPos);
		}
		// update zombies position - TODO find nearest human and hunt him//
		for(int i = 0; i < game->zombiesLen; i++) {
			if(numFrames % 20 == 0) {// change his direction once half a second
				game->zombies[i]->direction = vec2fRotate(game->zombies[i]->direction, rand() %10);
			}
			Vec2f newPos = vec2fMulS(game->zombies[i]->direction, game->zombies[i]->speed);
			newPos = vec2fAdd(game->zombies[i]->pos, newPos);
			userSetPos(game->zombies[i], newPos);
		}
		checkAllCollisions(game);
		cameraUpdate(game->cam);

		windowClear();

		glProgramUse(game->prog);
		glActiveTexture(GL_TEXTURE0);

		// send matrix location
		GLint pLocation = glGetUniformLocation(game->prog->programID, "P");
		glUniformMatrix4fv(pLocation, 1, GL_FALSE, 
				&(game->cam->cameraMatrix.m[0][0]));

		// build vertices for map //
		sbBuildBatches(game->level->mapBatch);
		sbDrawBatches(game->level->mapBatch);

		// build vertices for users //
		sbBuildBatches(game->usersBatch);
		sbDrawBatches(game->usersBatch);	


		glProgramUnuse(game->prog);
		
		windowUpdate(game->win);


	}
	//
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
