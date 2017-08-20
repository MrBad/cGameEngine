#ifndef GL_PROGRAM_H
#define GL_PROGRAM_H


#include <GL/glew.h>

typedef struct {
	GLuint programID;
	GLuint vertexShaderID;
	GLuint fragmentShaderID;
	GLint numAttributes;
} GLProgram;


GLProgram* glProgramNew();
void glProgramCompileShaders(GLProgram *program, const char *path);
void glProgramAddAttribute(GLProgram *program, const char *name);
void glProgramLinkShaders(GLProgram *program);
void glProgramUse(GLProgram *program);
void glProgramUnuse(GLProgram *program);
void glProgramDelete(GLProgram *program);

#endif

