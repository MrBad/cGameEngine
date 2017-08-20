#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "window.h"
#include "error.h"
#include "sprite.h"
#include "gl_program.h"
#include "texture.h"
#include "camera.h"
#include "inmgr.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

enum {
	GAME_PLAYING,
	GAME_OVER,
};

int main()
{
	Window *window;
	Sprite* sprite = NULL;
	GLProgram* program = NULL;
	Camera *camera = NULL;
	InMgr *inmgr = NULL;

	int status = GAME_PLAYING;
	
	
	window = windowNew("Colors in C", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	inmgr = inMgrNew();
	camera = cameraNew(WINDOW_WIDTH, WINDOW_HEIGHT);
	cameraSetScale(camera, 0.5);
	cameraUpdate(camera);

	//sprite = spriteNew(-1.0f, -1.0f, 2.0f, 2.0f);
	sprite = spriteNew(0, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

	Texture* texture = loadTexture("resources/earth.png");
	if(!texture) {
		fatalError("Cannot load texture\n");
	}
	//fprintf(stdout, "texture: %d, %dx%d\n", texture->id, texture->width, texture->height);

	program = glProgramNew();
	glProgramCompileShaders(program, "shaders/color_shader");
	glProgramAddAttribute(program, "vertexPosition");
	glProgramAddAttribute(program, "vertexColor");
	glProgramLinkShaders(program);

	fprintf(stdout, "Use WASD keys to move, QE to zoom\n");

	float time = 0;
	while(status == GAME_PLAYING) {
		inMgrUpdate(inmgr);
		if(inmgr->quitRequested) {
			status = GAME_OVER;
		}

		float camSpeed = 5.0f;
		float scaleSpeed = 1.02f;
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
		cameraUpdate(camera);
		time += 0.05f;
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		glProgramUse(program);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->id);
		// send texture
		GLint textureLocation = glGetUniformLocation(program->programID, "mySampler");
		glUniform1i(textureLocation, 0);

		// send time to shader
		GLint timeLocation = glGetUniformLocation(program->programID, "time");
		glUniform1f(timeLocation, time);

		// send matrix location
		GLint pLocation = glGetUniformLocation(program->programID, "P");
		glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(camera->cameraMatrix.m[0][0]));

		spriteDraw(sprite);

		glBindTexture(GL_TEXTURE_2D, 0);
		glProgramUnuse(program);
		
		windowUpdate(window);
	}

	
	glProgramDelete(program);
	inMgrDelete(inmgr);
	cameraDelete(camera);
	spriteDelete(sprite);
	
	windowDestroy(window);
}

