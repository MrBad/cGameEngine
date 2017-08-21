#ifndef SPRITE_H
#define SPRITE_H

#include <GL/glew.h>
#include "vertex.h"


typedef struct {
	float x, y, width, height;
	GLuint textureID;
	Vertex ventrices[6];
	GLuint vboID;
} Sprite;

Sprite* spriteNew(float x, float y, float width, float height, GLuint textureID);
void spriteSetPos(Sprite *sprite, float x, float y);
void spriteDraw(Sprite *sprite);
void spriteDelete(Sprite *sprite);

#endif
