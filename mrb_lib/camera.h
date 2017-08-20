#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include "vertex.h"
#include "mat4f.h"

// 2D camera
typedef struct {
	Position position;
	float scale;
	int screenWidth;
	int screenHeight;
	Mat4f orthoMatrix;
	Mat4f cameraMatrix;
	bool needsUpdate;
} Camera;


Camera *cameraNew(int screenWidth, int screenHeight);
void cameraSetPosition(Camera *camera, float x, float y);
void cameraSetScale(Camera *camera, float newScale);
void cameraUpdate(Camera *camera);
void cameraDelete(Camera *camera);

#endif
