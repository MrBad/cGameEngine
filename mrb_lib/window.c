#include <stdlib.h>
#include <GL/glew.h>
#include "error.h"
#include "window.h"

Window *windowNew(const char *title, int width, int height, int flags) 
{
    flags = 0;
    Window *window = calloc(1, sizeof(*window));
    if (!window) {
        fprintf(stderr, "Cannot alocate memory for window\n");
        return NULL;
    }
    window->width = width;
    window->height = height;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        windowDelete(window);
        return NULL;
    }
    window->sdlWindow = SDL_CreateWindow(
            title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height, SDL_WINDOW_OPENGL | flags);

    if (window->sdlWindow == NULL) {
        fprintf(stderr, "Cannot create window: %s\n", SDL_GetError());
        windowDelete(window);
        return NULL;
    }
    window->glContext = SDL_GL_CreateContext(window->sdlWindow);
    if (window->glContext == NULL) {
        fprintf(stderr, "Cannot create OpenGL context: %s\n", SDL_GetError());
        windowDelete(window);
        return NULL;
    }
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Cannot init glew\n");
        windowDelete(window);
        return NULL;
    }
    printf("--- OpenGL Version: %s ---\n", glGetString(GL_VERSION));
    windowSetClearColor(0, 0, 0.3, 1);

    // 1 for vsync, 0 for immediate update, -1 for late swap tearing
    SDL_GL_SetSwapInterval(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return window;
}

void windowSetClearColor(float r, float g, float b, float a) 
{
    glClearColor(r, g, b, a);
}

void windowClear()
{
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void windowUpdate(Window *window) 
{
    SDL_GL_SwapWindow(window->sdlWindow);
}

int windowSetUpdateInterval(int type)
{
    return SDL_GL_SetSwapInterval(type);
}

void windowDelete(Window *window)
{
    if (!window) {
        fprintf(stderr, "Cannot destroy empty window\n");
        return;
    }

    SDL_GL_DeleteContext(window->glContext);
    SDL_DestroyWindow(window->sdlWindow);
    SDL_Quit();
    free(window);
}

