/**
 * Input Manager - A SDL event wrapper
 *  Docs: https://wiki.libsdl.org/SDL_Keycode
 */
#ifndef INMGR_H
#define INMGR_H

#include <stdbool.h>

#define KBUF_LEN 256

enum {
    IM_KEY_NONE,
    IM_KEY_LEFT,
    IM_KEY_RIGHT,
    IM_KEY_UP,
    IM_KEY_DOWN,
    IM_KEY_A,
    IM_KEY_D,
    IM_KEY_W,
    IM_KEY_S,
    IM_KEY_Q,
    IM_KEY_E,
    IM_KEY_X,
    // TODO define more keys //
    IM_KEY_LEN,
};

typedef struct {
    unsigned char keysDown[IM_KEY_LEN];
    bool quitRequested;
} InMgr;

InMgr *inMgrNew();

/**
 * Updates the input manager internals
 * this should be called on each frame to update
 * the input manager internals
 * 
 * @param inmgr The input manager
 */
void inMgrUpdate(InMgr *inmgr);
bool inMgrIsKeyPressed(InMgr *inmgr, unsigned char key);
bool inMgrIsQuitRequested(InMgr *inmgr);

/**
 * Destroys the input manager
 * @param inmgr The input manager
 */
void inMgrDelete(InMgr *inMgr);

#endif

