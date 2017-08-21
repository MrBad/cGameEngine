#ifndef GLYPH_H
#define GLYPH_H

#include "vertex.h"

typedef struct {
	GLuint textureID;
	float depth;
	Vertex topLeft;
	Vertex bottomLeft;
	Vertex topRight;
	Vertex bottomRight;
} Glyph;

#endif
