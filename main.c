#if 0
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "window.h"
#include "error.h"
#include "sprite.h"
#include "gl_program.h"
#include "texture.h"
#include "camera.h"
#include "inmgr.h"
#endif


#include "game.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600


int main()
{

	Game *game = gameNew();
	if(!game) 
		return 1;

	if(!gameInit(game, WINDOW_WIDTH, WINDOW_HEIGHT, "A game test")) 
		return 1;

	fprintf(stdout, "Use WSAD to move, QE to zoom\n");	

	gameLoop(game);

	gameDelete(game);

}

