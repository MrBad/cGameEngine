#ifndef INMGR_H
#define INMGR_H
// input manager //
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
	IM_KEY_LEN,
};


typedef struct {
	unsigned char keysDown[IM_KEY_LEN];
	bool quitRequested;
} InMgr;

InMgr *inMgrNew();
// hadle events //
void inMgrUpdate(InMgr *inmgr);
bool inMgrIsKeyPressed(InMgr *inmgr, unsigned char key);
bool inMgrIsQuitRequested(InMgr *inmgr);
void inMgrDelete(InMgr *inMgr);

#endif
