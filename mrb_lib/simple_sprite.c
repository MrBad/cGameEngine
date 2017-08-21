#include <stdio.h>
#include <GL/glew.h>
#include <stdlib.h>
#include <stddef.h>
#include "error.h"
#include "vertex.h"
#include "simple_sprite.h"


void simpleSpriteSetPos(SimpleSprite *sprite, float x, float y) 
{
	sprite->x = x;
	sprite->y = y;	

	vertexSetPos(sprite->ventrices + 0, x + sprite->width, y + sprite->height);
	vertexSetPos(sprite->ventrices + 1, x, y + sprite->height);
	vertexSetPos(sprite->ventrices + 2, x, y);
	vertexSetPos(sprite->ventrices + 3, x, y);
	vertexSetPos(sprite->ventrices + 4, x + sprite->width, y);
	vertexSetPos(sprite->ventrices + 5, x + sprite->width, y + sprite->height);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, sprite->vboID);
	glBufferData(
			GL_ARRAY_BUFFER, sizeof(sprite->ventrices), 
			sprite->ventrices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	}

SimpleSprite *simpleSpriteNew(float x, float y, float width, float height, GLuint textureID) 
{
	SimpleSprite *sprite = calloc(1, sizeof(SimpleSprite));
	if(!sprite) {
		fatalError("Cannot calloc a new sprite\n");
	}

	sprite->x = x;
	sprite->y = y;
	sprite->textureID = textureID;
	sprite->width = width;
	sprite->height = height;
	sprite->vboID = 0;


	vertexSetUV(sprite->ventrices + 0, 1, 1);
	vertexSetUV(sprite->ventrices + 1, 0, 1);
	vertexSetUV(sprite->ventrices + 2, 0, 0);
	vertexSetUV(sprite->ventrices + 3, 0, 0);
	vertexSetUV(sprite->ventrices + 4, 1, 0);
	vertexSetUV(sprite->ventrices + 5, 1, 1);
	
	for(int i = 0; i < 6; i++) {
		vertexSetColor(sprite->ventrices + i, 255, 0, 255, 255);
	}

	glGenBuffers(1, &sprite->vboID);

	simpleSpriteSetPos(sprite, x, y);

	return sprite;
}


void simpleSpriteDraw(SimpleSprite *sprite) 
{

	glBindBuffer(GL_ARRAY_BUFFER, sprite->vboID);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
			0,2,GL_FLOAT,GL_FALSE,
			sizeof(Vertex), (void*)offsetof(Vertex, pos));

	glVertexAttribPointer(
			1, 4, GL_UNSIGNED_BYTE,GL_TRUE,
			sizeof(Vertex), (void*)offsetof(Vertex, color));
	
	glVertexAttribPointer(
			2, 2, GL_FLOAT, GL_FALSE,
			sizeof(Vertex), (void*)offsetof(Vertex, uv));

	///
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void simpleSpriteDelete(SimpleSprite *sprite) 
{
	if(sprite) {
		if(sprite->vboID) {
			glDeleteBuffers(1, &sprite->vboID);
		}
		free(sprite);
		sprite = NULL;
	} else {
		fatalError("Trying to destroy an empty sprite");
	}
}
