#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "game.h"


#define SIZE(a) sizeof(a)/sizeof(*a)

Game *gameNew() 
{
	Game *game;
	unsigned int i;

	if(!(game = malloc(sizeof(*game)))) {
		fprintf(stderr, "Cannot alloc memory for Game\n");
		return NULL;
	}

	game->win = NULL;
	game->prog = NULL;
	game->cam = NULL;
	game->inmgr = NULL;
	game->sprite = NULL;
	for(i = 0; i < SIZE(game->sprites); i++) {
		game->sprites[i] = NULL;
	}
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
	unsigned int i;

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
	for(i = 0; i < SIZE(game->sprites); i++) {
		game->sprites[i] = spriteNew(i*128, i* 96 + (rand() % 100), 128, 96);	
	}
	
	if(!(game->texture = loadTexture("resources/earth.png"))) {
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
	if(game->sprite) {
		spriteDelete(game->sprite);
		game->sprite = NULL;
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
	//float time = 0;
	while(game->state == GAME_PLAYING) {
		inMgrUpdate(game->inmgr);
		if(game->inmgr->quitRequested) {
			game->state = GAME_OVER;
		}

		gameHandleInput(game);
		cameraUpdate(game->cam);
		//time += 0.05f;
		windowClear();

		glProgramUse(game->prog);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, game->texture->id);
		GLint textureLocation = glGetUniformLocation(game->prog->programID, "mySampler");
		glUniform1i(textureLocation, 0);
		
		// send time to shader
        //GLint timeLocation = glGetUniformLocation(game->prog->programID, "time");
        //glUniform1f(timeLocation, time);

        // send matrix location
        GLint pLocation = glGetUniformLocation(game->prog->programID, "P");
        glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(game->cam->cameraMatrix.m[0][0]));

		//
		//spriteBatchBegin()
		//spriteBatchDraw()
		//spriteBatchEnd()
		//spriteBatchRender()
		//
		//spriteDraw(game->sprite);
		unsigned int i;
		for(i = 0; i < SIZE(game->sprites); i++) {
			spriteDraw(game->sprites[i]);
		}

        glBindTexture(GL_TEXTURE_2D, 0);
        glProgramUnuse(game->prog);

        windowUpdate(game->win);
	}
}
