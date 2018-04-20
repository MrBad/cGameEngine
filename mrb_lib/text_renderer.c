#include <stdio.h>
#include <stdlib.h>
#include "texture.h"
#include "text_renderer.h"

TextRenderer *trNew(char *texturePath, int numX, int numY, GLProgram *prog)
{
    TextRenderer *tr;

    if (!(tr = calloc(1, sizeof(*tr)))) {
        fprintf(stderr, "trNew: calloc\n");
        return NULL;
    }
    if (!(tr->texture = loadTexture(texturePath))) {
        fprintf(stderr, "trNew: could not load texture %s\n", texturePath);
        free(tr);
        return NULL;
    }
    if (numX > tr->texture->width || numY > tr->texture->height) {
        fprintf(stderr, "trNew: invalid number of sprites: %d %d\n",
                numX, numY);
        textureDelete(tr->texture);
        free(tr);
        return NULL;
    }
    if (!(tr->sprites = arrayNew())) {
        fprintf(stderr, "trNew: could not alocate sprites array\n");
        textureDelete(tr->texture);
        free(tr);
    }
    tr->numX = numX;
    tr->numY = numY;
    tr->prog = prog;
    tr->fontSize = 16;
    tr->spacing = 1.0f;
    tr->currColor = color(0, 0, 0, 255);
    if (!(tr->sb = sbNew(prog))) {
        fprintf(stderr, "trNew: could not init sprite batch\n");
        textureDelete(tr->texture);
        free(tr);
        return NULL;
    }
    sbInit(tr->sb);

    return tr;
}

void trDelete(TextRenderer *tr)
{
    Sprite *sp;
    int i; 

    arrayForEach(tr->sprites, sp, i)
        spriteDelete(sp);
    arrayDelete(&tr->sprites);
    sbDelete(tr->sb);
    textureDelete(tr->texture);
    free(tr);
}

void trSetFontSize(TextRenderer *tr, int fontSize)
{
    tr->fontSize = fontSize;
}

void trSetSpacing(TextRenderer *tr, float spacing)
{
    tr->spacing = spacing;
}

void trSetCamera(TextRenderer *tr, Camera *cam)
{
    tr->camera = cam;
}

void trSetColor(TextRenderer *tr, Color color)
{
    tr->currColor = color;
}

int trTextAt(TextRenderer *tr, int x, int y, char *str)
{
    int i;
    AABB caabb = cameraGetAABB(tr->camera);
    float uvW = 1.0 / tr->numX;
    float uvH = 1.0 / tr->numY;

    for (i = 0; str[i]; i++) {
        float scale = tr->camera->scale;
        float height = tr->fontSize / scale;
        float width = height;
        float posX = x / scale + caabb.minX + width * tr->spacing * i;
        float posY = caabb.maxY - y / scale - height;

        Sprite *sp = spriteNew(posX, posY, width, height, tr->texture->id);

        int idx = str[i] % tr->numX;
        int idy = tr->numY - 1 - (str[i] / tr->numX);
        AABB uv = aabb(idx * uvW, idy * uvH, (idx + 1) * uvW, (idy + 1) *uvH);
        spriteSetUV(sp, uv);

        spriteSetColor(sp, &tr->currColor);

        arrayPush(tr->sprites, sp);
        sbAddSprite(tr->sb, sp);
    }

    return 0;
}

//AABB trGetBox(TextRenderer *tr, char *text){}

void trRender(TextRenderer *tr)
{
    Sprite *sp;
    int i;

    tr->sb->needsSort = false;
    sbBuildBatches(tr->sb);
    sbDrawBatches(tr->sb);

    arrayForEach(tr->sprites, sp, i)
        spriteDelete(sp);
    arrayReset(tr->sprites);
    sbResetSprites(tr->sb);
}

