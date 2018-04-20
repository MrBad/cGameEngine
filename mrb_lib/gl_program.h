#ifndef GL_PROGRAM_H
#define GL_PROGRAM_H

#include <stdbool.h>
#include <GL/glew.h>

typedef struct {
    GLuint programID;
    GLuint vertexShaderID;
    GLuint fragmentShaderID;
    GLint numAttributes;
} GLProgram;

GLProgram* glProgramNew();

/**
 * Compiles the opengl shaders
 *
 * @param program The program to compile into
 * @param path The path to shaders (without extension)
 * @return true on success, false on error
 */
bool glProgramCompileShaders(GLProgram *program, const char *path);

/**
 * Link the shaders
 *
 * @param program The program to link
 * @return true on success, false on error
 */
bool glProgramLinkShaders(GLProgram *program);

void glProgramAddAttribute(GLProgram *program, const char *name);

void glProgramUse(GLProgram *program);

void glProgramUnuse(GLProgram *program);

void glProgramDelete(GLProgram *program);

#endif

