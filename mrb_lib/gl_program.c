#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "error.h"
#include "file_get.h"
#include "gl_program.h"


static void compileShader(const char *fullPath, GLuint shaderID) 
{
	unsigned char *buff = NULL;
	int size;
	buff = file_get(fullPath, &size);
	if(size < 0) {
		fatalError("Cannot open: %s\n", fullPath);
	}

	//glShaderSource(shaderID, 1, (const GLchar**)&content, NULL);
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
		exit(1);
	}

	//free(content);
	//fclose(fp);
	free(buff);
}

GLProgram* glProgramNew() 
{
	GLProgram* program;
	if(!(program = calloc(1, sizeof(GLProgram)))) {
		fatalError("Out of memory: GLSLProgram\n");
	}
	
	program->programID = glCreateProgram();
	program->vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	if(!program->vertexShaderID)
		fatalError("Cannot create vertexShaderID\n");
	program->fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	if(!program->fragmentShaderID)
		fatalError("Cannot create fragmentShaderID\n");
	
	return program;
}	

void glProgramCompileShaders(GLProgram *program, const char *path) {
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
	if(success == GL_FALSE) {
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
	for(int i = 0; i< program->numAttributes; i++)
		glEnableVertexAttribArray(i);
}

void glProgramUnuse(GLProgram *program) 
{
	glUseProgram(0);
	for(int i = 0; i < program->numAttributes; i++)
		glEnableVertexAttribArray(0);
}

void glProgramDelete(GLProgram *program)
{
	if(!program) {
		fatalError("Trying to free a null GLSLProgram\n");
	}

	free(program);
	program = NULL;
}

void glProgramAddAttribute(GLProgram *program, const char *attributeName) 
{
	printf("Adding attrib: %d %s\n", program->numAttributes, attributeName);
	glBindAttribLocation(
			program->programID, program->numAttributes++, attributeName);
}
