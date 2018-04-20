#ifndef TEXTURE_H
#define TEXTURE_H
#include <GL/glew.h>

typedef struct {
    GLuint id;
    int width;
    int height;
} Texture;

/**
 * Read a texture from png file and uploads it to GPU
 *
 * @param filePath Path to png file
 * @return a new Texture or NULL on failure
 */
Texture *loadTexture(const char *filePath);

/**
 * Destroys a texture
 * @param texture The texture
 */
void textureDelete(Texture *texture);

#endif // TEXTURE_H

