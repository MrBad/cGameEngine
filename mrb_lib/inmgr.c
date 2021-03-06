#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "inmgr.h"

InMgr *inMgrNew()
{
    InMgr *input = NULL;

    if (!(input = calloc(1, sizeof(*input)))) {
        fprintf(stderr, "Cannot alloc memory for InputManager\n");
        return NULL;
    }
    input->quitRequested = false;

    return input;
}

static void setKeyBuf(unsigned char *buf, int sdlKey, int val) 
{
    switch (sdlKey) {
        case SDLK_LEFT:     buf[IM_KEY_LEFT]    = val; break;
        case SDLK_RIGHT:    buf[IM_KEY_RIGHT]   = val; break;
        case SDLK_UP:       buf[IM_KEY_UP]      = val; break;
        case SDLK_a:        buf[IM_KEY_A]       = val; break;
        case SDLK_d:        buf[IM_KEY_D]       = val; break;
        case SDLK_w:        buf[IM_KEY_W]       = val; break;
        case SDLK_s:        buf[IM_KEY_S]       = val; break;
        case SDLK_q:        buf[IM_KEY_Q]       = val; break;
        case SDLK_e:        buf[IM_KEY_E]       = val; break;
        case SDLK_x:        buf[IM_KEY_X]       = val; break;
    }
}

//
// Hadle events //
//
void inMgrUpdate(InMgr *input) 
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            input->quitRequested = true;
        }
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                input->quitRequested = true;
            } else {
                setKeyBuf(input->keysDown, e.key.keysym.sym, 1);
            }

        } else if (e.type == SDL_KEYUP) {
            setKeyBuf(input->keysDown, e.key.keysym.sym, 0);
        }
    }
}

bool inMgrIsKeyPressed(InMgr *input, unsigned char key)
{
    return input->keysDown[key] == 1;
}

bool inMgrIsQuitRequested(InMgr *input) 
{
    return input->quitRequested;
}

void inMgrDelete(InMgr *input) 
{
    free(input);
}

