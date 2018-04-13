#ifndef LEVEL_H
#define LEVEL_H

#include "mrb_lib/vec2f.h"
#include "mrb_lib/texture.h"
#include "mrb_lib/sprite_batch.h"

enum {
    CIRCLE_TEX,
    GLASS_TEX,
    LIGHT_BRICKS_TEX,
    RED_BRICKS_TEX,
    NUM_TEXTURES,
};

typedef struct {
    int numHumans;          // number of humans on this level
    char *path;             // path to level.txt
    unsigned char *data;    // where to store level.txt content
    Vec2f playerPos;        // player position on map

    Vec2f *zombiesPos;      // zombies positions on map - TODO: use array.c
    int zombiesSize;        // size of zombies positions array
    int zombiesLen;         // number of zombies

    Texture **textures;     // array for textures - TODO: use array.c
    int textureSize;        // textures array size
    int textureLen;         // textures array len
    SpriteBatch *mapBatch;  // holds the map sprite batch
    int maxWidth, maxHeight;// map width and height - top right corner
} Level;

/**
 * Creates a new level
 *
 * @param path The path to the txt file with the level data.
 * @return A new allocated level structure.
 */
Level *levelNew(char *path);

/**
 * Deletes a level
 *
 * @param level The level to delete
 */
void levelDelete(Level *level);

/**
 * Loads the level (map, textures, sprites) from disk.
 *
 * @param level The level to load
 * @param prog The gl program to use for sprites
 * @return true on success, false on error
 */
bool loadLevel(Level *level, GLProgram *prog);

#endif

