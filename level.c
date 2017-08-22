#include <stdio.h>
#include "level.h"
#include "mrb_lib/file_get.h"
#include "mrb_lib/texture.h"

enum {
	CIRCLE_TEX,
	GLASS_TEX,
	LIGHT_BRICKS_TEX,
	RED_BRICKS_TEX,
	NUM_TEXTURES,
};

bool loadLevel(Game *game, char *path) 
{
	unsigned char *buff;
	int size, x, y, i;

	if(!(buff = file_get(path, &size))) {
		fprintf(stderr, "Cannot load level\n");
		return false;
	}	
	char str[512]; 
	sscanf((const char *)buff, "%s %d\n", str, &game->level.numHumans);

	// load textures //
	Texture **textures = game->level.textures;
	textures[CIRCLE_TEX] = loadTexture("resources/circle.png");
	textures[GLASS_TEX] = loadTexture("resources/glass.png");
	textures[LIGHT_BRICKS_TEX] = loadTexture("resources/light_bricks.png");
	textures[RED_BRICKS_TEX] = loadTexture("resources/red_bricks.png");


	for(i = 0; i < size && buff[i]!='\n'; i++);
	x = 0; y = 0;
	for(; buff[i]; i++, x+=64) {
		Texture *texture = NULL;
		Sprite *sp = NULL;
		Color color;
		color.r = color.g = color.b = color.a = 255;
		bool isPlayer = false, isZombie = false;	
		switch(buff[i]) {
			case 'R':
				texture = textures[RED_BRICKS_TEX];
				break;
			case 'B':
				texture = textures[RED_BRICKS_TEX];
				break;
			case 'G':
				texture = textures[GLASS_TEX];
				break;
			case 'L': 
				texture = textures[LIGHT_BRICKS_TEX];
				break;
			case '@': 
				texture = textures[CIRCLE_TEX];
				color.r = color.g = 128; color.b = 255;
				isPlayer = true;
				break;
			case 'Z':
				texture = textures[CIRCLE_TEX];
				color.r = 255; color.g = color.b = 64;	
				isZombie = true;
				break;
			case '.': 
				continue;
			case '\r':
				continue;
			case '\n': 
				x = 0;
				y+= 64;
				continue;

			default:
				fprintf(stderr, "Unknown identifier @ x:%d y:%d [%c]-[%i]\n", x, y, buff[i], buff[i]);
				continue;
		}
		sp = spriteNew(x, y, 64, 64, texture->id);
		spriteSetColor(sp, &color);
		sbAddSprite(game->spriteBatch, sp);
		if(isPlayer) {
			spriteSetDimensions(sp, 50, 50);
			game->player = sp;
			isPlayer = false;
		} else if(isZombie) {
			spriteSetDimensions(sp, 50, 50);
		}
	}

	free(buff);
	return true;	
}
