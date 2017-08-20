#ifndef SPRITE_H
#define SPRITE_H

#include <GL/glew.h>


typedef struct {
	float x, y, width, height;
	GLuint vboID;
} Sprite;

Sprite* spriteNew(float x, float y, float width, float height);
void spriteDraw(Sprite *sprite);
void spriteDelete(Sprite *sprite);

#endif
