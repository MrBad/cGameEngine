#ifndef GAME_H
#define GAME_H

#include "mrb_lib/window.h"
#include "mrb_lib/sprite.h"
#include "mrb_lib/texture.h"
#include "mrb_lib/gl_program.h"
#include "mrb_lib/camera.h"
#include "mrb_lib/inmgr.h"


typedef enum {
	GAME_PLAYING,
	GAME_OVER,
} GameStates;

typedef struct {
    GLuint textureID;
    GLint offset;
    GLsizei numVertices;
} SpriteBatch;

typedef struct {
	Window *win;
	GLProgram *prog;
	Camera *cam;
	InMgr *inmgr;
	Texture *earthTex;
	Texture *circleTex;
	Sprite *sprites[1024]; // will be improved
	SpriteBatch spriteBatches[1024]; // to be generated dynamically
	GameStates state;

	float camSpeed;
	float scaleSpeed;

} Game;


Game *gameNew();
bool gameInit(Game *game, int winWidth, int winHeight, const char *title);
void gameLoop(Game *game);
void gameDelete(Game *game);

#endif
