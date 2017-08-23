
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

