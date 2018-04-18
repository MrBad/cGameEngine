#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include "vertex.h"
#include "mat4f.h"
#include "aabb.h"

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

/**
 * Creates a new camera object.
 */
Camera *cameraNew(int screenWidth, int screenHeight);

/**
 * Sets camera world position (center point)
 */
void cameraSetPosition(Camera *camera, float x, float y);

/**
 * Sets the camera scale
 */
void cameraSetScale(Camera *camera, float newScale);

/**
 * Gets the world coordinates the camera sees
 */
AABB cameraGetAABB(Camera *camera);

/**
 * Update the camera matrix if needed
 *
 * @param camera The camera
 */
void cameraUpdate(Camera *camera);

/**
 * Destroys the camera
 */
void cameraDelete(Camera *camera);

#endif

