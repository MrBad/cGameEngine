#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

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
} InputManager;

InputManager * inputManagerNew();
// hadle events //
void inputManagerUpdate(InputManager *input);
bool inputManagerIsKeyPressed(InputManager *input, unsigned char key);
bool inputManagerIsQuitRequested(InputManager *input);
void inputManagerDelete(InputManager *input);

#endif
