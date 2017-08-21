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



int sortByTexture(const void *a, const void *b) {
	if ((*(Sprite **)a)->textureID < (*(Sprite **)b)->textureID)
		return -1;
	else if((*(Sprite **)a)->textureID > (*(Sprite **)b)->textureID)
		return 1;
	return 0;
}

void gameLoop(Game *game) 
{

	unsigned int i;
	Uint32 currTicks, prevTicks;
	int numFrames = 0;


	// building sprites //
	for(i = 0; i < SIZE(game->sprites); i++) {
		GLuint textureID = i % 2 == 0 ? game->earthTex->id : game->circleTex->id;
		game->sprites[i] = spriteNew(
				i*128, i*96 + (rand() % 100), 
				128, 96, textureID);
		Color color = {rand() % 255, rand() % 255, rand() % 255, 255};
		spriteSetColor(game->sprites[i], &color);
	}

	qsort(game->sprites, SIZE(game->sprites), sizeof(Sprite *), sortByTexture);	
	
	for(i = 0; i < 8; i++) 
		printf("%d %d\n", i, game->sprites[i]->textureID);

	int numVertices = SIZE(game->sprites) * 6;

	printf("sprites: %lu, numVertices: %d\n", SIZE(game->sprites), numVertices);

	Vertex *vertices = calloc(1, numVertices * sizeof(Vertex));
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

	glBindVertexArray(0);

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



		glBindVertexArray(vao);
		
		
		int j = 0;
		int numBatch = 0;
		//
		// build vertices based on new sprite type //
		//
		GLuint lastTextureID = game->sprites[0]->textureID;
		game->spriteBatches[numBatch].textureID = lastTextureID;
		game->spriteBatches[numBatch].offset = 0;
		game->spriteBatches[numBatch].numVertices = 0;	
		for(i = 0; i < SIZE(game->sprites); i++) {
			
			if(game->sprites[i]->textureID != lastTextureID) {
				lastTextureID = game->sprites[i]->textureID;
				numBatch++;
				game->spriteBatches[numBatch].offset = i * 6;
				game->spriteBatches[numBatch].textureID = lastTextureID;
				game->spriteBatches[numBatch].numVertices = 0;
			}	
			spriteSetPos(game->sprites[i], game->sprites[i]->x+1, game->sprites[i]->y);
			
			Sprite *sp = game->sprites[i];
			vertexSetPos(vertices + i*6 + 0, sp->x + sp->width, sp->y + sp->height);
			vertexSetPos(vertices + i*6 + 1, sp->x,				sp->y + sp->height); 	
			vertexSetPos(vertices + i*6 + 2, sp->x,				sp->y			  ); 	
			vertexSetPos(vertices + i*6 + 3, sp->x,				sp->y			  ); 	
			vertexSetPos(vertices + i*6 + 4, sp->x + sp->width, sp->y			  ); 	
			vertexSetPos(vertices + i*6 + 5, sp->x + sp->width, sp->y + sp->height);

			vertexSetUV(vertices + i*6 + 0, 1, 1);	
			vertexSetUV(vertices + i*6 + 1, 0, 1);	
			vertexSetUV(vertices + i*6 + 2, 0, 0);	
			vertexSetUV(vertices + i*6 + 3, 0, 0);	
			vertexSetUV(vertices + i*6 + 4, 1, 0);	
			vertexSetUV(vertices + i*6 + 5, 1, 1);	
			
			for(j = 0; j < 6; j++) {
				vertexSetColor(
					vertices + i * 6 + j, 
					game->sprites[i]->color.r, 
						game->sprites[i]->color.g, 
						game->sprites[i]->color.b, 
						game->sprites[i]->color.a);
			}

			game->spriteBatches[numBatch].numVertices += 6;
		}
		
		glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);
	
		// process batches //	
		for(int i = 0; i <= numBatch; i++) {
			glBindTexture(GL_TEXTURE_2D, game->spriteBatches[i].textureID);
			GLint textureLocation = glGetUniformLocation(game->prog->programID, "mySampler");
			glUniform1i(textureLocation, 0);
			
			//glDrawArrays(GL_TRIANGLES, 0, numVertices);
			glDrawArrays(
					GL_TRIANGLES, game->spriteBatches[i].offset, 
					game->spriteBatches[i].numVertices);
		
		
			glBindTexture(GL_TEXTURE_2D, 0);
		}

//		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glBindVertexArray(0);

		glProgramUnuse(game->prog);

		windowUpdate(game->win);


	}
	//
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(vao);
	free(vertices);
	for(i = 0; i < SIZE(game->sprites); i++) {
		spriteDelete(game->sprites[i]);
	}
}
