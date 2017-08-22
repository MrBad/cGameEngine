#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "game.h"
#include "mrb_lib/sprite_batch.h"

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
	/*
	if(!(game->sprite = spriteNew(0,0, winWidth / 2, winHeight / 2))) {
		fprintf(stderr, "Cannot init Sprite\n");
		return false;
	}*/
	
	if(!(game->earthTex = loadTexture("resources/earth.png"))) {
		fprintf(stderr, "Cannot load texture\n");
		return false;	
	}
	if(!(game->circleTex = loadTexture("resources/circle.png"))) {
		fprintf(stderr, "Cannot load texture\n");
		return false;	
	}
	if(!(game->prog = glProgramNew())) {
		fprintf(stderr, "Cannot init glProgram\n");
		return false;	
	}
	gameInitShaders(game);

	game->camSpeed = 5.0f;
	game->scaleSpeed = 1.02f;

	printf("Size of vertex: %lu\n", sizeof(Vertex) * 6 * 1024);

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
	if(game->circleTex) {
		free(game->circleTex);
		game->circleTex = NULL;
	}
	if(game->earthTex) {
		free(game->earthTex);
		game->earthTex = NULL;
	}
	windowDelete(game->win);
	free(game);

}

void gameHandleInput(Game *game) 
{

	InMgr *inmgr = game->inmgr;
	Camera *camera = game->cam;	
	float camSpeed = game->camSpeed;
	float scaleSpeed = game->scaleSpeed;
	if(inMgrIsKeyPressed(inmgr, IM_KEY_A)) {
		cameraSetPosition(camera, camera->position.x-camSpeed, camera->position.y);
	}
	if(inMgrIsKeyPressed(inmgr, IM_KEY_D)) {
		cameraSetPosition(camera, camera->position.x+camSpeed, camera->position.y);
	}
	if(inMgrIsKeyPressed(inmgr, IM_KEY_W)) {
		cameraSetPosition(camera, camera->position.x, camera->position.y+camSpeed);
	}
	if(inMgrIsKeyPressed(inmgr, IM_KEY_S)) {
		cameraSetPosition(camera, camera->position.x, camera->position.y-camSpeed);
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

	unsigned int i;
	Uint32 currTicks, prevTicks;
	int numFrames = 0;


	// create a sprite batch
	game->spriteBatch = sbNew(game->prog);

	// init the sprite batch
	sbInit(game->spriteBatch);

	// building sprites and add them to batch //
	for(i = 0; i < 1024; i++) {
		GLuint textureID = 
			i % 2 == 0 ? game->earthTex->id : game->circleTex->id;
		Sprite *sprite = spriteNew(
				i*128, i*96 + (rand() % 100), 
				128, 96, textureID);
		Color color = {rand() % 255, rand() % 255, rand() % 255, 255};
		spriteSetColor(sprite, &color);
		sbAddSprite(game->spriteBatch, sprite);
	}


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
		
		for(int i = 0; i < game->spriteBatch->spritesLen; i++) {
			game->spriteBatch->sprites[i]->x += 1;
		}		
		// build vertices based on new sprite type //
		sbBuildBatches(game->spriteBatch);
		sbDrawBatches(game->spriteBatch);
	

//		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		//glBindVertexArray(0);

		glProgramUnuse(game->prog);

		windowUpdate(game->win);


	}
	//
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	sbDelete(game->spriteBatch);
	
}
