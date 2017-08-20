#ifndef TEXTURE_H
#define TEXTURE_H
#include <GL/glew.h>

typedef struct {
	GLuint id;
	int width;
	int height;
} Texture;

Texture *loadTexture(const char *filePath);

#endif
