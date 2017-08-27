#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "game.h"
#include "level.h"
#include "collision.h"
#include <assert.h>

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
	game->totalFrames = 0;
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
	game->users = listNew(NULL);
	listAdd(game->users, game->player);

	ListNode *node; User *human, *zombie;
	
	listForeach(game->humans, node, human)
		listAdd(game->users, human);

	listForeach(game->zombies, node, zombie)
		listAdd(game->users, zombie);

	cameraSetPosition(game->cam, game->player->pos.x, game->player->pos.y);
	return true;
}

void gameDelete(Game *game) 
{
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

	ListNode *node; User *user;
	listForeach(game->users, node, user) {
		spriteDelete(user->sprite);
		userDelete(user);
	}
	listDelete(game->users);
	listDelete(game->humans);
	listDelete(game->zombies);
	
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
	Vec2f pos;
	if(inMgrIsKeyPressed(inmgr, IM_KEY_A)) {
		pos = vec2f(player->pos.x-player->speed, player->pos.y);
		userSetPos(player, pos);
		cameraSetPosition(camera, pos.x, pos.y);
	}
	
	if(inMgrIsKeyPressed(inmgr, IM_KEY_D)) {
		pos = vec2f(player->pos.x + player->speed, player->pos.y);
		userSetPos(player, pos);
		cameraSetPosition(camera, pos.x, pos.y);
	}

	if(inMgrIsKeyPressed(inmgr, IM_KEY_W)) {
		pos = vec2f(player->pos.x, player->pos.y + player->speed);
		userSetPos(player, pos);
		cameraSetPosition(camera, pos.x, pos.y);
	}
	if(inMgrIsKeyPressed(inmgr, IM_KEY_S)) {
		pos = vec2f(player->pos.x, player->pos.y - player->speed);
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
		numFrames++; game->totalFrames++;


		inMgrUpdate(game->inmgr);
		if(game->inmgr->quitRequested) {
			game->state = GAME_OVER;
		}

		gameHandleInput(game);

		humansUpdate(game);
		zombiesUpdate(game);

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
