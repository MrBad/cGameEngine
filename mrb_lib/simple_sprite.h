#ifndef SIMPLE_SPRITE_H
#define SIMPLE_SPRITE_H

#include <GL/glew.h>
#include "vertex.h"


typedef struct {
	float x, y, width, height;
	GLuint textureID;
	Vertex ventrices[6];
	GLuint vboID;
} SimpleSprite;

SimpleSprite* simpleSpriteNew(float x, float y, float width, float height, GLuint textureID);
void simpleSpriteSetPos(SimpleSprite *sprite, float x, float y);
void simpleSpriteDraw(SimpleSprite *sprite);
void simpleSpriteDelete(SimpleSprite *sprite);

#endif
