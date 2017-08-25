#ifndef GAME_H
#define GAME_H

#include "mrb_lib/window.h"
#include "mrb_lib/gl_program.h"
#include "mrb_lib/camera.h"
#include "mrb_lib/inmgr.h"
#include "mrb_lib/sprite_batch.h"
#include "mrb_lib/array.h"
#include "user.h"
#include "level.h"


typedef enum {
	GAME_PLAYING,
	GAME_OVER,
} GameStates;


typedef struct {
	Window *win;
	GLProgram *prog;
	Camera *cam;
	InMgr *inmgr;

	SpriteBatch *usersBatch;
	GameStates state;

	float camSpeed;
	float scaleSpeed;

	Level *level;
	User *player;
	Array *zombies;
	Array *humans;
	Array *users;	// ptr to a list of all users, used in collide
} Game;


Game *gameNew();
bool gameInit(Game *game, int winWidth, int winHeight, const char *title);
void gameLoop(Game *game);
void gameDelete(Game *game);


// defined in user.c
extern void initZombies(Game *game);
extern void initHumans(Game *game);
extern void initPlayer(Game *game);
//extern int allocZombie(Game *game);
//extern int allocHuman(Game *game);
#endif
