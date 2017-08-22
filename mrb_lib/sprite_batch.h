#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H
#include "vertex.h"
#include "sprite.h"
#include "gl_program.h"

typedef struct {
	GLuint textureID;	// this batch texture id
	GLint offset;		// offset into vertices
	GLsizei numVertices; // number of vertices in this batch
} RenderBatch;

typedef struct {
	RenderBatch **renderBatches; // ptr to render batches
	int rbSize;	// size of render batches including unused elements
	int rbLen;	// currently occupied by len batches

	Vertex *vertices; // where we store all vertices;
	int verticesSize;
	int verticesLen;

	Sprite **sprites; // ptr to ptrs to sprites
	int spritesSize;
	int spritesLen;

	GLuint vao, vbo;
	GLProgram *prog;
} SpriteBatch;


SpriteBatch *sbNew(GLProgram *program);
void sbInit(SpriteBatch *sb); //
int sbAddSprite(SpriteBatch *sb, Sprite *sp);
void sbBuildBatches(SpriteBatch *sb);
void sbDrawBatches(SpriteBatch *sb);
void sbDelete(SpriteBatch *sb);

#endif
