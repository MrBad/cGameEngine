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
	int numHumans;
	char *path;				// path to level.txt
	unsigned char *data; // store level.txt content
	Vec2f playerPos;	// player position on map
	Vec2f *zombiesPos;	// zombie position on map
	int zombiesSize;	// size of array
	int zombiesLen;		// number of zombies

	Texture **textures;	// holds the textures
	int textureSize;
	int textureLen;
	SpriteBatch *mapBatch; // holds the map sprite batch
	int maxWidth, maxHeight; // map width and height - top right corner
} Level;

Level *levelNew(char *path);
void levelDelete(Level *level);
bool loadLevel(Level *level, GLProgram *prog);

#endif
