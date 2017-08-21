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
	for(i = 0; i < SIZE(game->sprites); i++) {
		game->sprites[i] = spriteNew(
				i*128, i*96 + (rand() % 100), 
				128, 96, game->texture->id);
	}
	int numVertices = SIZE(game->sprites) * sizeof(Vertex) * 6;
	Vertex *vertices = malloc(numVertices);
	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
			0, 2, GL_FLOAT, GL_FALSE,
			sizeof(Vertex), (void *)offsetof(Vertex, pos));
	glVertexAttribPointer(
			1, 4, GL_UNSIGNED_BYTE, GL_TRUE,
			sizeof(Vertex), (void *)offsetof(Vertex, color));
	glVertexAttribPointer(
			2, 2, GL_FLOAT, GL_FALSE,
			sizeof(Vertex), (void *)offsetof(Vertex, uv));
	glDisableVertexAttribArray(vao);

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
		glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(game->cam->cameraMatrix.m[0][0]));


		/*int x, y;
		  for(i = 0; i < SIZE(game->sprites); i++) {

		// bind sprite texture
		glBindTexture(GL_TEXTURE_2D, game->texture->id);
		GLint textureLocation = glGetUniformLocation(game->prog->programID, "mySampler");
		glUniform1i(textureLocation, 0);

		x = game->sprites[i]->x;
		y = game->sprites[i]->y;
		spriteSetPos(game->sprites[i], x + 1, y);
		spriteDraw(game->sprites[i]);
		}*/


		glBindTexture(GL_TEXTURE_2D, game->texture->id);
		GLint textureLocation = glGetUniformLocation(game->prog->programID, "mySampler");
		glUniform1i(textureLocation, 0);

		glEnableVertexAttribArray(vao);
		int j = 0;

		for(i = 0; i < SIZE(game->sprites); i++) {
			spriteSetPos(game->sprites[i], game->sprites[i]->x+1, game->sprites[i]->y);
			// copy buffers //
			for(j = 0; j < 6; j++) {
				vertices[i*6 + j].pos.x = game->sprites[i]->ventrices[j].pos.x;
				vertices[i*6 + j].pos.y = game->sprites[i]->ventrices[j].pos.y;
				vertices[i*6 + j].uv.u = game->sprites[i]->ventrices[j].uv.u;
				vertices[i*6 + j].uv.v = game->sprites[i]->ventrices[j].uv.v;
				vertices[i*6 + j].color.r = game->sprites[i]->ventrices[j].color.r;
				vertices[i*6 + j].color.g = game->sprites[i]->ventrices[j].color.g;
				vertices[i*6 + j].color.b = game->sprites[i]->ventrices[j].color.b;
				vertices[i*6 + j].color.a = game->sprites[i]->ventrices[j].color.a;
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(game->sprites[0]->ventrices), game->sprites[0]->ventrices, GL_DYNAMIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, numVertices, vertices, GL_DYNAMIC_DRAW);	
		glDrawArrays(GL_TRIANGLES, 0, numVertices/6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);


		glBindTexture(GL_TEXTURE_2D, 0);
		glProgramUnuse(game->prog);

		windowUpdate(game->win);


	}
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	free(vertices);
	for(i = 0; i < SIZE(game->sprites); i++) {
		spriteDelete(game->sprites[i]);
	}
}
