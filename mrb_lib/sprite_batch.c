#include "vertex.h"
#include "sprite_batch.h"
#include <GL/glew.h>


/*
 create a vertex attribute array
	GLuint vao;
	glGenVertexArrays(1, vao);

 to use it
	glBindVertexArrays(vao);

	-- every call to glVertexAttribPointer will be stored in vao
	-- This makes switching between different vertex data and vertex formats as easy 
		as binding a different VAO! 
		Just remember that a VAO doesn't store any vertex data by itself, 
		it just references the VBOs you've created and how to retrieve the attribute 
		values from them.
	deaw arrays - type, offset, number of them
	glDrawArrays(GL_TRIANGLES, 0, 3);

 */


typedef struct {
	GLuint vao;
	GLuint vbo;
	Vertex *vertices;
	int vertLen;
	int vertSize;
} SpriteBatch;

void spriteBatchInit(SpriteBatch *sp) 
{
	// create vertex array
	
	if(sp->vao == 0) {
		glGenVertexArrays(1, &sp->vao);
	}
	glBindVertexArray(sp->vao);

	if(sp->vbo == 0) {
		glGenBuffers(1, &sp->vbo);
	}
	glBindBuffer(GL_ARRAY_BUFFER, sp->vbo);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
			0, 2, GL_FLOAT, GL_FALSE, 
			sizeof(Vertex), (void *)offsetof(Vertex, pos));
	glVertexAttribPointer(
			1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
			sizeof(Vertex), (void *)offsetof(Vertex, color));
	glVertexAttribPointer(
			2, 2, GL_FLOAT, GL_FALSE, 
			sizeof(Vertex), (void *)offsetof(Vertex, uv));

}

void spriteBatchSendData(SpriteBatch *sp) 
{
	glBindBuffer(GL_ARRAY_BUFFER, sp->vbo);

	glBufferData(
			GL_ARRAY_BUFFER, sp->vertLen * sizeof(Vertex), 
			NULL, GL_DYNAMIC_DRAW);
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, sp->vertLen * sizeof(Vertex), sp->vertices);
	
	// unbind buff
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
