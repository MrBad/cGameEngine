#ifndef SPRITE_H
#define SPRITE_H

#include <stdbool.h>
#include "vertex.h"
#include "aabb.h"

typedef struct {
    float x, y, width, height;
} Rect;

typedef struct {
    float x, y;             // position.
    float width, height;    // Dimensions.
    Color color;            // color to use
    GLuint textureID;       // Texture id to use (see Texture->id)
    AABB uv;                // using an AABB for UV, with values from 0 to 1.
    bool dirty;             // do we need to update?
} Sprite;

/**
 * Creates a new sprite
 */
Sprite *spriteNew(float x, float y, float width, float height, GLuint textureID);

/**
 * Destroys the sprite
 */
void spriteDelete(Sprite *sp);

/**
 * Sets sprite new position
 */
void spriteSetPos(Sprite *sp, float x, float y);

/**
 * @brief Sets the sampling UV of the texture.
 *
 * Sets the sampling UV for the texture, using an AABB for the two triangles
 * of the sprite. For eg., if we want to display only first quarter of the
 * texture, we will use aabb(0, 0, 0.5, 0.5).
 * Default is aabb(0, 0, 1, 1)
 *
 * @param sp The sprite
 * @param uv Sampling UV in AABB format.
 */
bool spriteSetUV(Sprite *sp, AABB uv);

/**
 * Sets the sprite dimensions
 */
void spriteSetDimensions(Sprite *sp, float width, float height);

/**
 * Sets the sprite color
 */
void spriteSetColor(Sprite *sp, Color *color);

/**
 * Sets sprite texture id
 */
void spriteSetTextureID(Sprite *sp, GLuint textureID);

#endif // SPRITE_H

