#include <stdio.h>
#include <stdlib.h>
#include "camera.h"


Camera *cameraNew(int screenWidth, int screenHeight) 
{
	Camera *cam = NULL;

	if(!(cam = calloc(1, sizeof(Camera)))) {
		fprintf(stderr, "Cannot allocate memory for camera\n");
		goto ret1;
	}

	cam->screenWidth = screenWidth;
	cam->screenHeight = screenHeight;
	cam->scale = 1.0f;
	cam->position.x = 0;
	cam->position.y = 0;
	cam->needsUpdate = true;

	// generate it only on start and in the future on win resize
	cam->orthoMatrix = mat4fOrtho(0, (float) screenWidth, (float) screenHeight, 0, -1, 1);	

ret1:
	return cam;

}


void cameraUpdate(Camera *camera)
{
	if(!camera->needsUpdate) {
		return;
	}

	Vec3f translateVec = {
		-camera->position.x + camera->screenWidth / 2,
		-camera->position.y + camera->screenHeight / 2,
		0.0f
	};

	Vec3f scaleVec = {
		camera->scale, camera->scale, 0.0f
	};

	//printf("--- OrthoMatrix ---\n");
	//mat4fPrint(camera->orthoMatrix);
	//printf("--- Translate Matrix ---\n");
	//mat4fPrint(mat4Translate(translateVec));

	camera->cameraMatrix = mat4fMul(camera->orthoMatrix, mat4fTranslate(translateVec));
	//printf("--- Camera Matrix ---\n");	
	//mat4fPrint(camera->cameraMatrix);


//	printf("--- Scale Matrix ---\n");
//	mat4fPrint(mat4fScale(scaleVec));
	
	camera->cameraMatrix = mat4fMul(mat4fScale(scaleVec), camera->cameraMatrix);
	//camera->cameraMatrix = mat4fIdentity();

	printf("--- Camera Matrix ---\n");	
	mat4fPrint(camera->cameraMatrix);

	camera->needsUpdate = false;
}


void cameraSetPosition(Camera *camera, float x, float y) 
{
	camera->position.x = x;
	camera->position.y = y;
	camera->needsUpdate = true;
}

void cameraSetScale(Camera *camera, float scale) {
	camera->scale = scale;
	camera->needsUpdate = true;
}

void cameraDelete(Camera *camera)
{
	free(camera);
}
