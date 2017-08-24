#include "user.h"
#include "game.h"

User *userNew(Vec2f pos, float speed, Sprite *sprite, UserType type) 
{
	User *user = NULL;
	if(!(user = malloc(sizeof(*user)))) {
		fprintf(stderr, "Cannot alloc user\n");
		return NULL;
	}

	user->pos = pos;
	user->speed = speed;
	user->sprite = sprite;
	user->type = type;
	user->direction = (Vec2f) {rand() % 100, rand() % 100};
	user->direction = vec2fNormalize(user->direction);
	user->velocity = vec2fMulS(user->direction, speed);
	return user;
}

void userSetPos(User *user, Vec2f pos) 
{
	user->sprite->x = pos.x;
	user->sprite->y = pos.y;
	user->pos = pos;
}
inline Vec2f userGetPos(User *user) {
	return (Vec2f) {user->pos.x, user->pos.y};
}

void userDelete(User *user) 
{
	free(user);
}


static int allocZombie(Game *game) 
{
	int numElements;
	if(game->zombiesLen == game->zombiesSize) {
		numElements = game->zombiesSize == 0 ? 2 : game->zombiesSize * 2;
		if(!(game->zombies = realloc(
						game->zombies, 
						numElements * sizeof(*game->zombies)))) 
		{
			fprintf(stderr, "Cannot realloc zombies list\n");
			return -1;
		}
		game->zombiesSize = numElements;
	}
	int index = game->zombiesLen++;
	return index;
}

static int allocHuman(Game *game)
{
    int numElements;
    if(game->humansLen == game->humansSize) {
        numElements = game->humansSize == 0 ? 2 : game->humansSize * 2;
        if(!(game->humans = realloc(
                        game->humans,
                        numElements * sizeof(*game->humans))))
        {
            fprintf(stderr, "Cannot realloc humans list\n");
            return -1;
        }
        game->humansSize = numElements;
    }
    int index = game->humansLen++;
    return index;
}
 
	
void initZombies(Game *game) 
{
	int i, idx;
	Vec2f pos;
	float speed;
	Sprite *sprite;		
	Color color;
	for(i = 0; i < game->level->zombiesLen; i++) {
		idx = allocZombie(game);
		pos = game->level->zombiesPos[idx];
		speed = 1.5f;
		sprite = spriteNew(
				pos.x, pos.y, USER_WIDTH, USER_HEIGHT,
				game->level->textures[CIRCLE_TEX]->id);
		color = (Color){255, 0, 0, 255};
		spriteSetColor(sprite, &color);
		game->zombies[idx] = userNew(pos, speed, sprite, ZOMBIE);
		sbAddSprite(game->usersBatch, sprite);
	}

}

void initHumans(Game *game)
{
	int i, idx;
	Vec2f pos;
	float speed;
	Sprite *sprite;
	Color color;
	for(i = 0; i < game->level->numHumans; i++) {
		// init humans //
		idx = allocHuman(game);
		pos = (Vec2f) {128 + rand() % (game->level->maxWidth - 256), 128 + rand() % (game->level->maxHeight - 256)};
		speed = 1.0f;
		sprite = spriteNew(
				pos.x, pos.y, USER_WIDTH, USER_HEIGHT,
				game->level->textures[CIRCLE_TEX]->id);
		color = (Color){rand()%128, rand()%255, rand()%128, 255};
		spriteSetColor(sprite, &color);
		game->humans[idx] = userNew(pos, speed, sprite, HUMAN);
		// add the sprite to users batches
		sbAddSprite(game->usersBatch, sprite);
	}
}

void initPlayer(Game *game) 
{
	Vec2f pos;
	float speed;
	Sprite *sprite;
	Color color;

	pos = game->level->playerPos;
	speed = 5.0f;
	sprite = spriteNew(
			pos.x, pos.y, USER_WIDTH, USER_HEIGHT,
			game->level->textures[CIRCLE_TEX]->id);
	color = (Color){128, 128, 255, 255};
	spriteSetColor(sprite, &color);
	game->player = userNew(pos, speed, sprite, PLAYER);
	// add the sprite to users batches
	sbAddSprite(game->usersBatch, game->player->sprite);
}

inline Rect userGetRect(User *user)
{
	return (Rect) {
		user->pos.x, user->pos.y, 
		user->sprite->width, user->sprite->height};
}
