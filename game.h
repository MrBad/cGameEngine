#ifndef GAME_H
#define GAME_H

#include "window.h"
#include "sprite.h"
#include "texture.h"
#include "gl_program.h"
#include "camera.h"
#include "inmgr.h"


typedef enum {
	GAME_PLAYING,
	GAME_OVER,
} GameStates;

typedef struct {
	Window *win;
	GLProgram *prog;
	Camera *cam;
	InMgr *inmgr;
	Sprite *sprite; // will be removed
	Texture *texture;

	GameStates state;

	float camSpeed;
	float scaleSpeed;

} Game;


Game *gameNew();
bool gameInit(Game *game, int winWidth, int winHeight, const char *title);
void gameLoop(Game *game);
void gameDelete(Game *game);

#endif
