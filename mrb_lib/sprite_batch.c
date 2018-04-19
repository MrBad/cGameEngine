#include <string.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include "sprite_batch.h"

#define SB_INIT_RB_LEN 16
#define SB_INIT_SPRITES_LEN 16

SpriteBatch *sbNew(GLProgram *prog)
{
    SpriteBatch *sb = malloc(sizeof(*sb));
    if (!sb) {
        fprintf(stderr, "Cannot malloc SpriteBatch\n");
        return NULL;
    }
    sb->renderBatches = NULL;
    sb->rbSize = 0;
    sb->rbLen = 0; // 0 initial elements

    sb->vertices = NULL;
    sb->verticesSize = 0;
    sb->verticesLen = 0;

    sb->sprites = NULL;
    sb->spritesSize = 0;
    sb->spritesLen = 0;

    sb->needsSort = true;
    sb->vao = sb->vbo = 0;
    sb->prog = prog;

    return sb;
}

void sbInit(SpriteBatch *sb) 
{
    if (sb->vao == 0) {
        glGenVertexArrays(1, &sb->vao);
    }
    glBindVertexArray(sb->vao);

    if (sb->vbo == 0) {
        glGenBuffers(1, &sb->vbo);
    }
    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Position - check shader in //
    glVertexAttribPointer(
            0, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void *) offsetof(Vertex, pos));
    // Color //
    glVertexAttribPointer(
            1, 4, GL_UNSIGNED_BYTE, GL_TRUE,
            sizeof(Vertex), (void *) offsetof(Vertex, color));
    // UV //
    glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void *) offsetof(Vertex, uv));

    glBindBuffer(GL_ARRAY_BUFFER, 0); // do we still need this?
    glBindVertexArray(0);
}

void sbDelete(SpriteBatch *sb)
{
    if (!sb)
        return;
    if (sb->renderBatches)
        free(sb->renderBatches);
    if (sb->vertices)
        free(sb->vertices);
    if (sb->sprites)
        free(sb->sprites);

    //TODO - more cleanup
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //?!glDisableVertexAttribArray(sb->vao);
    free(sb);
}

// adds a pointer to sprite to internal sprite batch //
// and return the index or -1 on error //
int sbAddSprite(SpriteBatch *sb, Sprite *sp)
{
    int i;
    // resize buffer if needed //
    if (sb->spritesLen == sb->spritesSize) {
        i = sb->spritesSize == 0 ? 8 : sb->spritesSize * 2;
        sb->sprites = realloc(sb->sprites, i * sizeof(*sb->sprites));
        if (!sb->sprites) {
            fprintf(stderr, "Cannot realloc sb->sprites\n");
            return -1;
        }
        while (sb->spritesSize < i)
            sb->sprites[sb->spritesSize++] = NULL;
    }
    // find free slot //
    for (i = 0; i < sb->spritesSize; i++) {
        if (! sb->sprites[i]) {
            sb->sprites[i] = sp;
            sb->spritesLen++;
            sb->needsSort = true;
            return i;
        }
    }

    return -1;
}

void sbResetSprites(SpriteBatch *sb)
{
    memset(sb->sprites, 0, sb->spritesSize * sizeof(sb->sprites[0]));
    sb->spritesLen = 0;
}

bool sbDeleteSprite(SpriteBatch *sb, Sprite *sp) 
{
    int i, j;
    bool found = false;

    for (i = 0; i < sb->spritesLen; i++) {
        if (sb->sprites[i] == sp) {
            printf("found sprite at %d index\n", i);
            found = true;
            for (j = i; j < sb->spritesLen; j++) {
                if (sb->sprites[j + 1])
                    sb->sprites[j] = sb->sprites[j + 1];
                else
                    sb->sprites[j] = 0;
            }
            sb->spritesLen--;
            sb->needsSort = true;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "Cannot find sprite: %p\n", (void*) sp);
    }

    return found;
}

static int getFreeRenderBatch(SpriteBatch *sb) 
{
    int i;

    if (sb->rbLen == sb->rbSize) {
        i = sb->rbSize == 0 ? 2 : sb->rbSize * 2;
        sb->renderBatches = realloc(sb->renderBatches, i * sizeof(*sb->renderBatches));
        assert(sb->renderBatches != NULL);
        if (!sb->renderBatches) {
            fprintf(stderr, "Cannot realloc sp->renderBatches\n");
            return -1;
        }
        while (sb->rbSize < i)
            sb->renderBatches[sb->rbSize++] = NULL;
    }
    // find free slot
    for (i = 0; i < sb->rbSize; i++) {
        if (! sb->renderBatches[i]) { // found
            sb->renderBatches[i] = malloc(sizeof(RenderBatch));
            if (!sb->renderBatches[i]) {
                fprintf(stderr, "cannot alloc a render batch\n");
                return -1;
            }
            sb->rbLen++;
            return i;
        }
    }

    return -1;
}

// Sorts sprites by textureID
// TODO - use a self balancing tree, to avoid sorting
// that's it, keep sprites preSorted
static int sortByTexture(const void *a, const void *b) 
{
    if ((*(Sprite **)a)->textureID < (*(Sprite **)b)->textureID)
        return -1;
    else if ((*(Sprite **)a)->textureID > (*(Sprite **)b)->textureID)
        return 1;

    return 0;
}

static void sbSort(SpriteBatch *sb)
{
    if (sb->needsSort) {
        printf("Sorting sprites\n");
        qsort(sb->sprites, sb->spritesLen, sizeof(Sprite *), sortByTexture);
        sb->needsSort = false;
    }
}

void sbBuildBatches(SpriteBatch *sb)
{
    GLuint lastTextureId = 0;
    int numBatch = 0, i, j;

    if (!sb)
        return;

    sbSort(sb);

    int needSize = sb->spritesLen * 6;

    if (sb->verticesSize < needSize) {
        sb->vertices = realloc(sb->vertices, needSize * sizeof(Vertex));
        if (!sb->vertices) {
            fprintf(stderr, "Cannot realloc vertices\n");
            return;
        }
        sb->verticesSize = needSize;
        fprintf(stdout, "Realloc vertices size to: %d\n", sb->verticesSize);
    }

    for (i = 0; i < sb->spritesLen; i++) {
        if (!sb->sprites[i]->textureID) {
            printf("invalid sprite texture at position: %d\n", i);
            continue;
        }
        assert(sb->sprites);
        assert(sb->sprites[i]);
        assert(sb->sprites[i]->textureID);

        if (sb->sprites[i]->textureID != lastTextureId) {
            lastTextureId = sb->sprites[i]->textureID;
            numBatch = getFreeRenderBatch(sb);
            sb->renderBatches[numBatch]->textureID = lastTextureId;
            sb->renderBatches[numBatch]->offset = i * 6;
            sb->renderBatches[numBatch]->numVertices = 0;
        }

        assert(sb->renderBatches);

        Vertex *v;
        Sprite *sp = sb->sprites[i];
        v = sb->vertices + i * 6;
        vertexSetPos(v++, sp->x + sp->width, sp->y + sp->height);
        vertexSetPos(v++, sp->x,             sp->y + sp->height);
        vertexSetPos(v++, sp->x,             sp->y				);
        vertexSetPos(v++, sp->x,             sp->y				);
        vertexSetPos(v++, sp->x + sp->width, sp->y				);
        vertexSetPos(v++, sp->x + sp->width, sp->y + sp->height);

        v = sb->vertices + i * 6;
        vertexSetUV(v++, sp->uv.maxX, sp->uv.maxY);
        vertexSetUV(v++, sp->uv.minX, sp->uv.maxY);
        vertexSetUV(v++, sp->uv.minX, sp->uv.minY);
        vertexSetUV(v++, sp->uv.minX, sp->uv.minY);
        vertexSetUV(v++, sp->uv.maxX, sp->uv.minY);
        vertexSetUV(v++, sp->uv.maxX, sp->uv.maxY);

        for (j = 0; j < 6; j++) {
            vertexSetColor(
                    sb->vertices + i * 6 + j,
                    sp->color.r,
                    sp->color.g,
                    sp->color.b,
                    sp->color.a);
        }
        assert(sb->renderBatches);
        assert(sb->renderBatches[numBatch]);
        sb->renderBatches[numBatch]->numVertices += 6;
    }
    sb->verticesLen = i * 6;
}

void sbDrawBatches(SpriteBatch *sb) 
{
    int i;
    GLint textureLocation;
    
    if (!sb)
        return;

    glBindVertexArray(sb->vao); // bind vertex array
    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo);
    glBufferData(GL_ARRAY_BUFFER, sb->verticesLen * sizeof(Vertex),
            sb->vertices, GL_DYNAMIC_DRAW);	 // send data to GPU

    glProgramUse(sb->prog);

    glActiveTexture(GL_TEXTURE0);

    for (i = 0; i < sb->rbLen; i++) {
        glBindTexture(GL_TEXTURE_2D, sb->renderBatches[i]->textureID);
        textureLocation = glGetUniformLocation(sb->prog->programID, "mySampler");
        glUniform1i(textureLocation, 0);
        glDrawArrays(
                GL_TRIANGLES, sb->renderBatches[i]->offset,
                sb->renderBatches[i]->numVertices);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glProgramUnuse(sb->prog);

    for (i = 0; i < sb->rbLen; i++) {
        free(sb->renderBatches[i]);
        sb->renderBatches[i] = 0;
    }
    sb->rbLen = 0;
}

