#ifndef SPRITE_H
#define SPRITE_H

#include <stdbool.h>
#include "vertex.h"

typedef struct {
    float x, y, width, height;
} Rect;

typedef struct {
    float x, y, width, height;
    Color color;
    GLuint textureID;
    // TODO include box with UV to have multi sprite
    bool dirty;     // do we need to update?
} Sprite;

Sprite *spriteNew(float x, float y, float width, float height, GLuint textureID);
void spriteDelete(Sprite *sp);

void spriteSetPos(Sprite *sp, float x, float y);
void spriteSetDimensions(Sprite *sp, float width, float height);
void spriteSetColor(Sprite *sp, Color *color);
void spriteSetTextureID(Sprite *sp, GLuint textureID);

#endif // SPRITE_H

