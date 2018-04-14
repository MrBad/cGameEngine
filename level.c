#include <stdio.h>
#include "mrb_lib/file_get.h"
#include "mrb_lib/texture.h"
#include "mrb_lib/sprite_batch.h"
#include "mrb_lib/strdup.h"
#include "level.h"

Level *levelNew(char *path) 
{
    Level *level = NULL;

    if ((level = malloc(sizeof(*level)))) {
        level->path = strdup(path);
        level->data = NULL;
        level->numHumans = 0;
        level->playerPos= (Vec2f) {0, 0};
        level->zombiesPos= NULL;
        level->zombiesLen = 0;
        level->zombiesSize = 0;
        level->textures = NULL;
        level->textureLen = 0;
        level->textureSize = 0;
        level->mapBatch = NULL;
        level->maxWidth = level->maxHeight = 0;
    }

    return level;
}

void levelDelete(Level *level) 
{
    int i;

    if (level->path)
        free(level->path);
    if (level->data)
        free(level->data);
    if (level->zombiesPos)
        free(level->zombiesPos);
    if (level->textures) {
        for (i = 0; i < NUM_TEXTURES; i++) {
            if (level->textures[i])
                glDeleteTextures(1, &level->textures[i]->id);
            free(level->textures[i]);
        }
        free(level->textures);
    }
    for (i = 0; i < level->mapBatch->spritesLen; i++) {
        if (level->mapBatch->sprites[i])
            spriteDelete(level->mapBatch->sprites[i]);
    }
    sbDelete(level->mapBatch);
    free(level);
}

// TODO - use mrb_lib/array.c
static int allocZombiePosition(Level *level)
{
    int numElements;

    // do we need to expand buffer?
    if (level->zombiesLen == level->zombiesSize) {
        numElements = level->zombiesSize == 0 ? 2 : level->zombiesSize * 2;
        if (!(level->zombiesPos = realloc(
                        level->zombiesPos,
                        numElements * sizeof(* level->zombiesPos)))) {
            fprintf(stderr, "Cannot realloc zombie buffer\n");
            return -1;
        }
        level->zombiesSize = numElements;
    }
    int index = level->zombiesLen++;

    return index;
}

bool loadLevel(Level *level, GLProgram *prog)
{
    int size, x, y, i;
    char str[512];

    // get file
    if (!(level->data = file_get(level->path, &size))) {
        fprintf(stderr, "Cannot load level\n");
        return false;
    }

    // load textures //
    level->textures = malloc(NUM_TEXTURES * sizeof(*level->textures));
    if (!level->textures) {
        fprintf(stderr, "Cannot alloc textures\n");
        return false;
    }
    level->textures[CIRCLE_TEX] = loadTexture("resources/circle.png");
    level->textures[GLASS_TEX] = loadTexture("resources/glass.png");
    level->textures[LIGHT_BRICKS_TEX] = loadTexture("resources/light_bricks.png");
    level->textures[RED_BRICKS_TEX] = loadTexture("resources/red_bricks.png");

    // create map sprite batch //
    if (!(level->mapBatch = sbNew(prog))) {
        fprintf(stderr, "Cannot create sprite batch\n");
        return false;
    }

    sscanf((const char *)level->data, "%s %d\n", str, &level->numHumans);

    for (i = 0; i < size && level->data[i]!='\n'; i++);
    x = 0; y = 0;
    int idx;
    for (; level->data[i]; i++, x+=64) {
        Texture *texture = NULL;
        Sprite *sp = NULL;
        Color color;
        color.r = color.g = color.b = color.a = 255;

        switch (level->data[i]) {
            case 'R':
                texture = level->textures[RED_BRICKS_TEX];
                break;
            case 'B':
                texture = level->textures[RED_BRICKS_TEX];
                break;
            case 'G':
                texture = level->textures[GLASS_TEX];
                break;
            case 'L':
                texture = level->textures[LIGHT_BRICKS_TEX];
                break;
            case '@': // player - save position
                level->playerPos.x = x;
                level->playerPos.y = y;
                continue;
            case 'Z': // zombie - add it's position to zombies positions
                idx = allocZombiePosition(level);
                level->zombiesPos[idx].x = x;
                level->zombiesPos[idx].y = y;
                continue;
            case '.':
                continue;
            case '\r':
                continue;
            case '\n':
                x = 0;
                y+= 64;
                continue;
            default:
                fprintf(stderr, "Unknown identifier @ x:%d y:%d [%c]-[%i]\n",
                        x, y, level->data[i], level->data[i]);
                continue;
        }
        sp = spriteNew(x, y, 64, 64, texture->id);
        spriteSetColor(sp, &color);
        sbAddSprite(level->mapBatch, sp);
        level->maxWidth = x > level->maxWidth ? x : level->maxWidth;
        level->maxHeight = y > level->maxHeight ? y : level->maxHeight;
    }
    level->maxWidth += 64;
    level->maxHeight += 64;

    return true;
}

