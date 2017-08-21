#include <stdlib.h>
#include <GL/glew.h>
#include "error.h"
#include "window.h"


Window *windowNew(const char *title, int width, int height, int flags) 
{
	flags = 0;
	Window *window = calloc(1, sizeof(*window));
	if(!window)
		fatalError("Cannot alocate memory for window\n");

	window->width = width;
	window->height = height;
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) 
		fatalError("SDL_Init: %s\n", SDL_GetError());
	
	window->sdlWindow = SDL_CreateWindow(
			title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			width, height, SDL_WINDOW_OPENGL | flags);
	
	if(window->sdlWindow == NULL) {
		fatalError("Cannot create window: %s\n", SDL_GetError());
	}
	window->glContext = SDL_GL_CreateContext(window->sdlWindow);
	if(window->glContext == NULL) {
		fatalError("Cannot create OpenGL context: %s\n", SDL_GetError());
	}
	if(glewInit() != GLEW_OK) {
		fatalError("Cannot init glew\n");
	}
	printf("--- OpenGL Version: %s ---\n", glGetString(GL_VERSION));
	windowSetClearColor(0, 0, 0.3, 1);

	SDL_GL_SetSwapInterval(0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return window;
	

}
void windowSetClearColor(float r, float g, float b, float a) 
{
	glClearColor(r, g, b, a);
}
void windowClear() {
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

#if 0
bool windowHandleEvents(Window *window) 
{
	SDL_Event event = window->event;
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT) {
			return true;
		} else if(event.type == SDL_KEYDOWN) {
			if(event.key.keysym.sym == SDLK_ESCAPE) {
				return true;
			}
		}
	}

	return false;
}
#endif
void windowUpdate(Window *window) 
{
	SDL_GL_SwapWindow(window->sdlWindow);
}
void windowDelete(Window *window) 
{
	if(!window) {
		fatalError("Cannot destroy empty window\n");
	}	

	SDL_GL_DeleteContext(window->glContext);
	SDL_DestroyWindow(window->sdlWindow);
	SDL_Quit();
	free(window);
	window = NULL;
}

