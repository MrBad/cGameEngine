#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "error.h"
#include "file_get.h"
#include "gl_program.h"

/**
 * Compiles the shaders
 *
 * @param fullPath The path to shader
 * @param shaderId the handle of the shader obj whose source is to be replaced
 * @return true on success, false on error
 */
static bool compileShader(const char *fullPath, GLuint shaderID) 
{
    unsigned char *buff = NULL;
    int size;

    buff = file_get(fullPath, &size);
    if (size < 0) {
        fprintf(stderr, "Cannot open: %s\n", fullPath);
        return false;
    }

    glShaderSource(shaderID, 1, (const GLchar**)&buff, NULL);
    glCompileShader(shaderID);

    // error handling //
    GLint success = 0;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
        char *errorLog = calloc(1, maxLength + 1);
        glGetShaderInfoLog(shaderID, maxLength, &maxLength, errorLog);
        glDeleteShader(shaderID);
        fprintf(stderr, "Cannot create shader %s: %s\n", fullPath, errorLog);
        free(errorLog);
        free(buff);
        return false;
    }
    free(buff);

    return true;
}

/**
 * Creates a new program
 *
 * @return the new allocated GLProgram on success, NULL on error
 */
GLProgram* glProgramNew() 
{
    GLProgram *program;

    if (!(program = calloc(1, sizeof(*program)))) {
        fprintf(stderr, "Out of memory: GLProgram\n");
        return NULL;
    }

    program->programID = glCreateProgram();
    program->vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

    if (!program->vertexShaderID) {
        fprintf(stderr, "Cannot create vertexShaderID\n");
        return NULL;
    }
    program->fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (!program->fragmentShaderID) {
        fprintf(stderr, "Cannot create fragmentShaderID\n");
        return NULL;
    }

    return program;
}	

/**
 * Destroys a program
 *
 * @param program The program to destroy
 */
void glProgramDelete(GLProgram *program)
{
    if (!program) {
        fprintf(stderr, "Trying to free a null GLSLProgram\n");
        return;
    }
    glDeleteShader(program->vertexShaderID);
    glDeleteShader(program->fragmentShaderID);
    glDeleteProgram(program->programID);

    free(program);
    program = NULL;
}

bool glProgramCompileShaders(GLProgram *program, const char *path) 
{
    char *fullPath = calloc(1, strlen(path) + 4);
    strcat(fullPath, path);
    strcat(fullPath, ".vs");
    if (!compileShader(fullPath, program->vertexShaderID)) {
        free(fullPath);
        return false;
    }
    fullPath[0] = 0;
    strcat(fullPath, path);
    strcat(fullPath, ".fs");
    if (!compileShader(fullPath, program->fragmentShaderID)) {
        free(fullPath);
        return false;
    }
    free(fullPath);

    return true;
}

bool glProgramLinkShaders(GLProgram *program) 
{
    glAttachShader(program->programID, program->vertexShaderID);
    glAttachShader(program->programID, program->fragmentShaderID);

    glLinkProgram(program->programID);

    // handle errors //
    GLint success = 0;
    glGetProgramiv(program->programID, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program->programID, GL_INFO_LOG_LENGTH, &maxLength);
        char *errorLog = calloc(1, maxLength + 1);
        glGetProgramInfoLog(program->programID, maxLength, &maxLength, errorLog);
        glDeleteProgram(program->programID);
        fprintf(stderr, "Cannot link program %s\n", errorLog);
        free(errorLog);
        return false;
    }

    glDetachShader(program->programID, program->vertexShaderID);
    glDetachShader(program->programID, program->fragmentShaderID);
    glDeleteShader(program->vertexShaderID);
    glDeleteShader(program->fragmentShaderID);

    return true;
}

/**
 * Use the glprogram
 *
 * @param program The program to use
 */
void glProgramUse(GLProgram *program) 
{
    glUseProgram(program->programID);
    for (int i = 0; i< program->numAttributes; i++)
        glEnableVertexAttribArray(i);
}

/**
 * Unuse the gl program
 *
 * @param progrom The program to release
 */
void glProgramUnuse(GLProgram *program) 
{
    glUseProgram(0);
    for (int i = 0; i < program->numAttributes; i++)
        glEnableVertexAttribArray(0);
}

/**
 * Adds an attribute name to the program
 *
 * @param program The program to add attribute to
 * @param attributeName The name of the attribute
 */
void glProgramAddAttribute(GLProgram *program, const char *attributeName) 
{
    printf("Adding attrib: %d %s\n", program->numAttributes, attributeName);
    glBindAttribLocation(
            program->programID, program->numAttributes++, attributeName);
}

