#ifndef WINDOW_H
#define WINDOW_H
#include <stdbool.h>
#include "SDL2/SDL.h"

typedef struct {
	int width, height;
	SDL_Window *sdlWindow;
	SDL_GLContext glContext;
	SDL_Event event;
} Window;

enum {
	WINDOW_NOFLAGS,
	WINDOW_HIDDEN,
} WindowFlags;

Window *windowNew(const char *title, int width, int height, int flags);
void windowSetClearColor(float r, float g, float b, float a);
void windowClear();
void windowUpdate(Window *window);
//bool windowHandleEvents(Window *window);
void windowDestroy(Window *window);

#endif
