#include "game.h"
#include "mrb_lib/array.h"
#include "mrb_lib/aabb.h"
#include "mrb_lib/list.h"
#include "mrb_lib/quad_tree.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600


#ifdef DO_TESTS
void doTests()
{
	arrayTest();
	aabbTest();
	listTest();
	quadTreeTest();
}
#endif

int main()
{

#ifdef DO_TESTS	
	doTests();
#endif // DO_TESTS

	Game *game = gameNew();
	if(!game) 
		return 1;

	if(!gameInit(game, WINDOW_WIDTH, WINDOW_HEIGHT, "A game test")) 
		return 1;

	fprintf(stdout, "The planet earth is invaded by zombies. They infect everyone with their contagious plague\n");
	fprintf(stdout, "There are zombies marked with red dot, humans multicolored and you - blue dot\n");	
	fprintf(stdout, "You need to resurect zombies, by touching them\n");
	fprintf(stdout, "Use WSAD to move, QE to zoom\n\n");	

	gameLoop(game);

	gameDelete(game);

}

