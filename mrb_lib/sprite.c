#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include "sprite.h"

Sprite *spriteNew(float x, float y, float width, float height, GLuint textureID) 
{
    Sprite *sp = NULL;

    if (!(sp = calloc(1, sizeof(*sp)))) {
        fprintf(stderr, "Cannot malloc sprite\n");
        return NULL;
    }
    spriteSetTextureID(sp, textureID);
    spriteSetPos(sp, x, y);
    spriteSetDimensions(sp, width, height);

    Color c = color(255, 255, 255, 255);
    spriteSetColor(sp, &c);
    sp->dirty = true;

    spriteSetNumFrames(sp, 1, 1);
    AABB uv = aabb(0, 0, 1, 1);
    spriteSetUV(sp, uv);

    return sp;
}

void spriteSetPos(Sprite *sp, float x, float y) 
{
    sp->x = x;
    sp->y = y;
    sp->dirty = true;
}

void spriteSetDimensions(Sprite *sp, float width, float height) 
{
    sp->width = width;
    sp->height = height;
    sp->dirty = true;
}

void spriteSetColor(Sprite *sp, Color *color) 
{
    sp->color.r = color->r;
    sp->color.g = color->g;
    sp->color.b = color->b;
    sp->color.a = color->a;
}

void spriteSetNumFrames(Sprite *sp, int numX, int numY)
{
    sp->numX = numX;
    sp->numY = numY;
}

bool spriteSetFrame(Sprite *sp, int x, int y)
{
    if (x > sp->numX)
        return false;
    if (y > sp->numY)
        return false;
    y = sp->numY - y - 1; // start from top to bottom
    float fx = 1.0f / sp->numX,
          fy = 1.0f / sp->numY;

    AABB uv = aabb(x * fx, y * fy, (x + 1) * fx, (y + 1) * fy);
    spriteSetUV(sp, uv);

    return true;
}

void spriteSetTextureID(Sprite *sp, GLuint textureID) 
{
    if (textureID <= 0) {
        fprintf(stderr, "Invalid Texture iD: %d\n", textureID);
        exit(1);
    }
    sp->textureID = textureID;
    sp->dirty = true;
}

bool spriteSetUV(Sprite *sp, AABB uv)
{
    if (uv.minX < 0 || uv.minX > 1 || uv.minY < 0 || uv.minY > 1 ||
        uv.maxX < 0 || uv.maxX > 1 || uv.maxY < 0 || uv.maxY > 1 ||
        uv.minX > uv.maxX || uv.minY > uv.maxY)
    {
        fprintf(stderr, "spriteSetUV AABB uv must have vals from 0.0 to 1.0\n");
        return false;
    }
    sp->uv = uv;

    return true;
}

void spriteDelete(Sprite *sp) 
{
    free(sp);
}

