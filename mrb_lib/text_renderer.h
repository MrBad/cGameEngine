#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "array.h"
#include "texture.h"
#include "gl_program.h"
#include "sprite_batch.h"
#include "camera.h"

typedef struct {
    Texture *texture;   // Texture with all letters
    GLProgram *prog;    // GL program to use
    Camera *camera;     // Camera
    int numX, numY;     // Number of letters in texture on x and y axis
    int fontSize;       // Font size to use
    float spacing;      // Spacing between letters, 0.0 -> 1.0 ->
    SpriteBatch *sb;    // Sprite batch to use in draw
    Array *sprites;     // All letters. Can be removed if using sprite-batch array
    Color currColor;    // Current drawing color
} TextRenderer;

/**
 * Creates a new text renderer
 *
 * @param texturePath Path to texture to use
 * @param numX Number of letters per column
 * @param numY Number of letters per row
 * @param prog GLProgram to use
 * @return A new TextRenderer
 */
TextRenderer *trNew(char *texturePath, int numX, int numY, GLProgram *prog);

/**
 * Destroys a text renderer
 * @param tr The renderer to destroy
 */
void trDelete(TextRenderer *tr);

/**
 * Sets the font size
 *
 * @param tr The text renderer it refers
 * @param fontSize The new font size
 */
void trSetFontSize(TextRenderer *tr, int fontSize);

/**
 * Sets spacing between letters
 * @param tr The text renderer
 * @param spacing space between letter, in letter width - 1.0 default
 */
void trSetSpacing(TextRenderer *tr, float spacing);

/**
 * Sets the font color
 *
 * @param tr The text renderer
 * @param color Color to use color(r, g, b, a)
 */
void trSetColor(TextRenderer *tr, Color color);

/**
 * Sets game camera, to be used by text
 */
void trSetCamera(TextRenderer *tr, Camera *cam);
int trTextAt(TextRenderer *tr, int x, int y, char *text);
//AABB trGetBox(TextRenderer *tr, char *text);
void trRender(TextRenderer *tr);

#endif // TEXT_RENDERER_H

