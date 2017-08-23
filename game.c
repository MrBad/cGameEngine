#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "game.h"
#include "mrb_lib/sprite_batch.h"
#include "level.h"



#define SIZE(a) sizeof(a)/sizeof(*a)

Game *gameNew() 
{
	Game *game;

	if(!(game = malloc(sizeof(*game)))) {
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
	

	cameraSetPosition(game->cam, game->level->playerPos.x, game->level->playerPos.x);
	

	// create new user batch //
	game->usersBatch = sbNew(game->prog);
	sbInit(game->usersBatch);

	// init player //
	Vec2f pos = game->level->playerPos;
	float speed = 1.0f;
	Sprite *sprite = spriteNew(
			pos.x, pos.y, USER_WIDTH, USER_HEIGHT, 
			game->level->textures[CIRCLE_TEX]->id);
	game->player = userNew(pos, speed, sprite, PLAYER);

	// add the sprite to users batches
	sbAddSprite(game->usersBatch, game->player->sprite);
	

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
	for(int i = 0; i < 4; i++) {
		if(game->level->textures[i]) {
			free(game->level->textures[i]);
		}
	}
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
	float playerSpeed = 10;
	Sprite *player = game->player->sprite;
	if(inMgrIsKeyPressed(inmgr, IM_KEY_A)) {
		spriteSetPos(player, player->x - playerSpeed, player->y);
		cameraSetPosition(camera, player->x, player->y);
		//cameraSetPosition(camera, camera->position.x-camSpeed, camera->position.y);
	}
	if(inMgrIsKeyPressed(inmgr, IM_KEY_D)) {
		spriteSetPos(player, player->x + playerSpeed, player->y);
		cameraSetPosition(camera, player->x, player->y);
		//cameraSetPosition(camera, camera->position.x+camSpeed, camera->position.y);
	}
	if(inMgrIsKeyPressed(inmgr, IM_KEY_W)) {
		spriteSetPos(player, player->x, player->y+playerSpeed);
		cameraSetPosition(camera, player->x, player->y);
		//cameraSetPosition(camera, camera->position.x, camera->position.y+camSpeed);
	}
	if(inMgrIsKeyPressed(inmgr, IM_KEY_S)) {

		spriteSetPos(player, player->x, player->y-playerSpeed);
		cameraSetPosition(camera, player->x, player->y);
		//cameraSetPosition(camera, camera->position.x, camera->position.y-camSpeed);
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
		numFrames++;


		inMgrUpdate(game->inmgr);
		if(game->inmgr->quitRequested) {
			game->state = GAME_OVER;
		}

		gameHandleInput(game);
		cameraUpdate(game->cam);
		windowClear();

		glProgramUse(game->prog);
		glActiveTexture(GL_TEXTURE0);

		// send matrix location
		GLint pLocation = glGetUniformLocation(game->prog->programID, "P");
		glUniformMatrix4fv(pLocation, 1, GL_FALSE, 
				&(game->cam->cameraMatrix.m[0][0]));

		// build vertices based on new sprite type //
		sbBuildBatches(game->level->mapBatch);
		sbDrawBatches(game->level->mapBatch);

		sbBuildBatches(game->usersBatch);
		sbDrawBatches(game->usersBatch);	

//		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		//glBindVertexArray(0);

		glProgramUnuse(game->prog);

		windowUpdate(game->win);


	}
	//
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
