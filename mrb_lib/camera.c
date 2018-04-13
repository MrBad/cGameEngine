#include <stdio.h>
#include <stdlib.h>
#include "camera.h"


Camera *cameraNew(int screenWidth, int screenHeight) 
{
    Camera *cam = NULL;

    if (!(cam = calloc(1, sizeof(Camera)))) {
        fprintf(stderr, "Cannot allocate memory for camera\n");
        return NULL;
    }

    cam->screenWidth = screenWidth;
    cam->screenHeight = screenHeight;
    cam->scale = 1.0f;
    cam->position.x = 0;
    cam->position.y = 0;
    cam->needsUpdate = true;

    // generate it only on start and in the future on win resize
    cam->orthoMatrix = mat4fOrtho(0, (float) screenWidth,
            (float) screenHeight, 0, -1, 1);

    return cam;
}

void cameraUpdate(Camera *camera)
{
    if (!camera->needsUpdate) {
        return;
    }

    Vec3f translateVec = {
        -camera->position.x + camera->screenWidth / 2,
        -camera->position.y + camera->screenHeight / 2,
        0.0f
    };

    Vec3f scaleVec = { camera->scale, camera->scale, 0.0f };

    camera->cameraMatrix = mat4fMul(camera->orthoMatrix, mat4fTranslate(translateVec));
    camera->cameraMatrix = mat4fMul(mat4fScale(scaleVec), camera->cameraMatrix);
    camera->needsUpdate = false;
#if 0
    printf("camX: %.2f, camY: %.2f, scale: %.2f\n\t",
            camera->position.x, camera->position.y, camera->scale);
    printAABB(cameraGetAABB(camera));
    printf("\n");
#endif
}

void cameraSetPosition(Camera *camera, float x, float y) 
{
    camera->position.x = x;
    camera->position.y = y;
    camera->needsUpdate = true;
}

void cameraSetScale(Camera *camera, float scale)
{
    camera->scale = scale;
    camera->needsUpdate = true;
}

AABB cameraGetAABB(Camera *cam)
{
    return (AABB) {
        (cam->position.x - cam->screenWidth / 2) / cam->scale,
            (cam->position.y - cam->screenHeight / 2) / cam->scale,
            (cam->position.x + cam->screenWidth / 2) / cam->scale,
            (cam->position.y + cam->screenHeight / 2) / cam->scale,
    };
}

void cameraDelete(Camera *camera)
{
    free(camera);
}

