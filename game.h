#ifndef GAME_H
#define GAME_H

#include <assert.h>
#include <signal.h>
#include "mrb_lib/window.h"
#include "mrb_lib/gl_program.h"
#include "mrb_lib/camera.h"
#include "mrb_lib/inmgr.h"
#include "mrb_lib/sprite_batch.h"
#include "mrb_lib/list.h"

typedef enum {
	GAME_PLAYING,
	GAME_OVER,
} GameStates;

typedef struct Game Game;
typedef int (*onGameInitFn) (Game *game);
typedef int (*onGameUpdateFn) (Game *game, int ticks);
typedef void (*onGameDeleteFn) (Game *game);

struct Game {
	Window *win;
	GLProgram *prog;
	Camera *cam;
	InMgr *inmgr;
	GameStates state;
	float scaleSpeed;

//    QuadTree *qtree;
	SpriteBatch *sBatch;

    onGameInitFn onGameInit; 
    onGameUpdateFn onGameUpdate; 
    onGameDeleteFn onGameDelete; 
	unsigned long totalFrames;
	void *usr;
};

Game *gameNew();
bool gameInit(Game *game, int winWidth, int winHeight, const char *title);
void gameLoop(Game *game);
void gameDelete(Game *game);

// defined in user.c
extern void usersInit(Game *game);
extern void usersUpdate(Game *game);
// defined in collision.c
extern void checkAllCollisions(Game *game);

#endif

