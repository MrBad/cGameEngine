#include "game.h"
#include "mrb_lib/strdup.h"
#include "mrb_lib/vec2f.h"
#include "mrb_lib/inmgr.h"
#include "mrb_lib/text_renderer.h"

int onGameInit(Game *game);
int onGameUpdate(Game *game, int ticks);
void onGameDelete(Game *game);

typedef struct Entity Entity;
typedef int (*entityUpdateFn) (Game *game, Entity *ent, int ticks);
int playerUpdate(Game *game, Entity *ent, int ticks);

struct Entity {
    Vec2f pos;
    Vec2f dim;
    Sprite *sprite;
    int type;
};

typedef struct {
    Entity ent;
    int numSprX, numSprY;
    Vec2f velocity;
    float speed;            // normal speed, when it's walking
    entityUpdateFn update;
    int ticks;
} Player;

char *textures[] = {
    "resources/red_bricks.png",
    "resources/hero.png",
    "resources/bfont.png"
};
enum { RBRICK, HERO, FONT, NUM_TEXTURES };

#define BRICKSZ 64.0f
#define PLAYER_NFRAMES_X 6
#define PLAYER_NFRAMES_Y 4

typedef struct {
    int mapWidth, mapHeight;
    char *map;
    Texture *textures[NUM_TEXTURES];
    Array *entities;
    Player *player;
} UsrGame;

int main()
{
    Game *game = gameNew();
    if (!game)
        return -1;

    game->onGameInit = onGameInit;
    game->onGameUpdate = onGameUpdate;
    game->onGameDelete = onGameDelete;

    if (!gameInit(game, 800, 600, "Sprite Animation"))
        return -1;

    return 0;
}

int onGameInit(Game *game)
{
    int i, x, y;
    UsrGame *usrGame;
    Player *player;
    if (!(usrGame = calloc(1, sizeof(*usrGame))))
        return -1;

    game->priv = usrGame;
    usrGame->map = strdup(
      "######################\n"
      "#                    #\n"
      "#                    #\n"
      "#           #        #\n"
      "#    @ #    #        #\n"
      "#       #   #        #\n"
      "#        #            #\n"
      "#                    #\n"
      "######################\n"
    );

    usrGame->entities = arrayNew();

    for (i = 0; i < NUM_TEXTURES; i++)
        if (!(usrGame->textures[i] = loadTexture(textures[i])))
            return -1;

    int mapLen = strlen(usrGame->map) - 1;
    Entity *brick;
    Color col;

    for (i = 0, x = 0, y = 0; i < mapLen; i++) {
        switch (usrGame->map[i]) {
            case '\n':
                x = 0;
                y++;
                continue;
            case '#':
                brick = calloc(1, sizeof(*brick));
                brick->pos = vec2f(x * BRICKSZ, y * BRICKSZ);
                brick->dim = vec2f(BRICKSZ, BRICKSZ);
                brick->sprite = spriteNew(
                       brick->pos.x, brick->pos.y, BRICKSZ, BRICKSZ,
                       usrGame->textures[RBRICK]->id
                );
                col = color(255, 255, 255, 255);
                spriteSetColor(brick->sprite, &col);
                sbAddSprite(game->sBatch, brick->sprite);
                arrayPush(usrGame->entities, brick);
                break;
            case '@':
                player = calloc(1, sizeof(*player));
                player->update = playerUpdate;
                player->speed = 0.3;
                player->ent.pos = vec2f(x * BRICKSZ, y * BRICKSZ);
                player->ent.dim = vec2f(
                        usrGame->textures[HERO]->width / PLAYER_NFRAMES_X,
                        usrGame->textures[HERO]->height / PLAYER_NFRAMES_Y
                );
                player->ent.sprite = spriteNew(
                       player->ent.pos.x, player->ent.pos.y,
                       player->ent.dim.x, player->ent.dim.y,
                       usrGame->textures[HERO]->id
                );
                col = color(255, 255, 255, 255);
                spriteSetColor(player->ent.sprite, &col);
                spriteSetNumFrames(
                        player->ent.sprite,
                        PLAYER_NFRAMES_X, PLAYER_NFRAMES_Y
                );
                spriteSetFrame(player->ent.sprite, 1, 1);
                sbAddSprite(game->sBatch, player->ent.sprite);
                arrayPush(usrGame->entities, player);
                usrGame->player = player;
                break;
            case ' ':
                break;
            default:
                printf("Uknown map type element at %d, %d\n", x, y);
                break;
        }
        x++;
    }
    usrGame->mapWidth = x;
    usrGame->mapHeight = y;

    return 0;
}

int playerUpdate(Game *game, Entity *ent, int ticks)
{
    Player *player = (Player *) ent;
    InMgr *mgr = game->inmgr;
    Camera *cam = game->cam;
    Vec2f velocity = player->velocity;
    static int frameX = 0;

    player->velocity = vec2f(0, 0);
    if (inMgrIsKeyPressed(mgr, IM_KEY_W))
        player->velocity.y = 1;
    else if (inMgrIsKeyPressed(mgr, IM_KEY_S))
        player->velocity.y = -1;
    else if (inMgrIsKeyPressed(mgr, IM_KEY_A))
        player->velocity.x = -1;
    else if (inMgrIsKeyPressed(mgr, IM_KEY_D))
        player->velocity.x = 1;
    else
        player->velocity = vec2f(0, 0);

    if (inMgrIsKeyPressed(mgr, IM_KEY_Q))
        cameraSetScale(cam, cam->scale * game->scaleSpeed);
    if (inMgrIsKeyPressed(mgr, IM_KEY_E))
        cameraSetScale(cam, cam->scale / game->scaleSpeed);

    Vec2f pos = vec2fMulS(player->velocity, ticks * player->speed);
    pos = vec2fAdd(player->ent.pos, pos);
    player->ent.pos = pos;
    spriteSetPos(player->ent.sprite, pos.x, pos.y);

    enum {P_STOP, P_WALK};
    enum {F_DOWN, F_LEFT, F_UP, F_RIGHT};
    int pState = P_WALK, pFace = 0;

        if (player->velocity.x < 0)
            pFace = F_LEFT;
        else if (player->velocity.x > 0)
            pFace = F_RIGHT;
        else if (player->velocity.y < 0)
            pFace = F_DOWN;
        else if (player->velocity.y > 0)
            pFace = F_UP;
        else
            pState = P_STOP;

        if (pState == P_STOP)
            frameX = 0;
        else
            frameX = (frameX + 1) % 5;
    bool changedDirection =
        velocity.x != player->velocity.x
        || velocity.y != player->velocity.y;
    player->ticks+=ticks;
    if (player->ticks > 80 || changedDirection) {
        spriteSetFrame(player->ent.sprite, 1 + frameX, pFace);
        player->ticks = 0;
    }
    
    return 0;
}

static bool isColliding(Rect *a, Rect *b)
{
    return (!(
                (a->x >= b->x + b->width) ||
                (a->x + a->width <= b->x) ||
                (a->y >= b->y + b->height) ||
                (a->y+a->height <= b->y)
    ));
}

static Vec2f getDistance(Rect *a, Rect *b)
{
    Vec2f centerA = { a->x + a->width / 2, a->y + a->height / 2 };
    Vec2f centerB = { b->x + b->width / 2, b->y + b->height / 2 };
    Vec2f distVec = vec2fSub(centerB, centerA);

    return distVec;
}

int checkCollisions(Game *game)
{
    int i;
    UsrGame *usrGame = game->priv;
    Player *player = usrGame->player;
    Entity *ent;

    arrayForEach(usrGame->entities, ent, i) {
        if ((Entity *) player == ent)
            continue;
        if (isColliding((Rect *) player, (Rect *) ent)) {
            Vec2f distVec = getDistance((Rect *) ent, (Rect *) player);
            if (fabs(distVec.x) > fabs(distVec.y)) {
                if (distVec.x < 0)
                    player->ent.pos.x = ent->pos.x - player->ent.dim.x;
                else
                    player->ent.pos.x = ent->pos.x + ent->dim.x;
            } else {
                if (distVec.y < 0)
                    player->ent.pos.y = ent->pos.y - player->ent.dim.y;
                else
                    player->ent.pos.y = ent->pos.y + ent->dim.y;
            }
            spriteSetPos(
                    player->ent.sprite, 
                    player->ent.pos.x, player->ent.pos.y
            );
        }
    }

    return 0;
}

void printFPS(Game *game)
{
    char str[64];
    trSetFontSize(game->tr, 24);
    trSetSpacing(game->tr, 0.5f);
    trSetColor(game->tr, color(0, 128, 0, 255));
    if (game->fps) {
        snprintf(str, sizeof(str), "FPS: %d", game->fps);
        trTextAt(game->tr, 0, 0, str);
    }
}

int onGameUpdate(Game *game, int ticks)
{
    UsrGame *usrGame = game->priv;
    Player *player = usrGame->player;
    if (player->update)
        player->update(game, (Entity *) player, ticks);
    checkCollisions(game);
    cameraSetPosition(game->cam, player->ent.pos.x, player->ent.pos.y);
    printFPS(game);

    return 0;
}

void onGameDelete(Game *game)
{
    int i;
    UsrGame *usrGame = game->priv;
    Entity *entity;

    free(usrGame->map);
    for (i = 0; i < NUM_TEXTURES; i++)
        free(usrGame->textures[i]);

    arrayForEach(usrGame->entities, entity, i) {
        if (entity->sprite)
            spriteDelete(entity->sprite);
        free(entity);
    }

    arrayDelete(&usrGame->entities);
    free(usrGame);
}

