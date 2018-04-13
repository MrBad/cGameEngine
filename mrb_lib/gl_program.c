#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "error.h"
#include "file_get.h"
#include "gl_program.h"

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
    if(success == GL_FALSE) {
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

GLProgram* glProgramNew() 
{
    GLProgram* program;
    if(!(program = calloc(1, sizeof(GLProgram)))) {
        fprintf(stderr, "Out of memory: GLProgram\n");
        return NULL;
    }

    program->programID = glCreateProgram();
    program->vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

    if(!program->vertexShaderID) {
        fprintf(stderr, "Cannot create vertexShaderID\n");
        return NULL;
    }
    program->fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if(!program->fragmentShaderID) {
        fprintf(stderr, "Cannot create fragmentShaderID\n");
        return NULL;
    }

    return program;
}	

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

void glProgramCompileShaders(GLProgram *program, const char *path) 
{
    char *fullPath = calloc(1, strlen(path) + 4);
    strcat(fullPath, path);
    strcat(fullPath, ".vs");
    compileShader(fullPath, program->vertexShaderID);

    fullPath[0] = 0;
    strcat(fullPath, path);
    strcat(fullPath, ".fs");
    compileShader(fullPath, program->fragmentShaderID);

    free(fullPath);
}

void glProgramLinkShaders(GLProgram *program) 
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
        exit(1);
    }

    glDetachShader(program->programID, program->vertexShaderID);
    glDetachShader(program->programID, program->fragmentShaderID);
    glDeleteShader(program->vertexShaderID);
    glDeleteShader(program->fragmentShaderID);
}

void glProgramUse(GLProgram *program) 
{
    glUseProgram(program->programID);
    for (int i = 0; i< program->numAttributes; i++)
        glEnableVertexAttribArray(i);
}

void glProgramUnuse(GLProgram *program) 
{
    glUseProgram(0);
    for (int i = 0; i < program->numAttributes; i++)
        glEnableVertexAttribArray(0);
}


void glProgramAddAttribute(GLProgram *program, const char *attributeName) 
{
    printf("Adding attrib: %d %s\n", program->numAttributes, attributeName);
    glBindAttribLocation(
            program->programID, program->numAttributes++, attributeName);
}

