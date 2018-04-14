#include "user.h"
#include "game.h"

User *userNew(Vec2f pos, float speed, Sprite *sprite, UserType type) 
{
    User *user = NULL;

    if (!(user = malloc(sizeof(*user)))) {
        fprintf(stderr, "Cannot alloc user\n");
        return NULL;
    }

    user->pos = pos;
    user->speed = speed;
    user->sprite = sprite;
    user->type = type;
    user->direction = vec2f(rand() % 100, rand() % 100);
    user->direction = vec2fNormalize(user->direction);

    if (user->direction.x == 0 && user->direction.y == 0) {
        user->direction = vec2f(10, 120);
    }

    user->velocity = vec2fMulS(user->direction, speed);
    user->qtObj = NULL;

    return user;
}

void userSetPos(User *user, Vec2f pos) 
{
    user->pos = pos;
    user->sprite->x = pos.x;
    user->sprite->y = pos.y;

    AABB limits = aabb(pos.x, pos.y, pos.x + USER_WIDTH, pos.y + USER_HEIGHT);
    qtObjectUpdate(user->qtObj, limits);
}

inline Vec2f userGetPos(User *user) 
{
    return vec2f(user->pos.x, user->pos.y);
}

void userDelete(User *user) 
{
    if (user) {
        free(user);
    }
}

void initZombies(Game *game) 
{
    int i;
    Vec2f pos;
    float speed;
    Sprite *sprite;
    Color c;
    game->zombies = listNew(NULL);

    for (i = 0; i < game->level->zombiesLen; i++) {
        pos = game->level->zombiesPos[i];
        speed = 1.5f;
        sprite = spriteNew(
                pos.x, pos.y, USER_WIDTH, USER_HEIGHT,
                game->level->textures[CIRCLE_TEX]->id);

        c = color(255, 0, 0, 255);
        spriteSetColor(sprite, &c);
        listAdd(game->zombies, userNew(pos, speed, sprite, ZOMBIE));
        sbAddSprite(game->usersBatch, sprite);
    }
}

void initHumans(Game *game)
{
    int i;
    Vec2f pos;
    float speed;
    Sprite *sprite;
    Color c;
    game->humans = listNew(NULL);

    for (i = 0; i < game->level->numHumans; i++) {
        pos = vec2f(
                128 + rand() % (game->level->maxWidth - 256),
                128 + rand() % (game->level->maxHeight - 256));

        speed = 1.0f;
        sprite = spriteNew(
                pos.x, pos.y, USER_WIDTH, USER_HEIGHT,
                game->level->textures[CIRCLE_TEX]->id);

        c = color(rand() % 128, rand() % 255, rand() % 128, 255);
        spriteSetColor(sprite, &c);
        listAdd(game->humans, userNew(pos, speed, sprite, HUMAN));
        // add the sprite to users batches
        sbAddSprite(game->usersBatch, sprite);
    }
}

/**
 * Initialize the player
 */
void initPlayer(Game *game) 
{
    Vec2f pos;
    float speed;
    Sprite *sprite;
    Color c;

    pos = game->level->playerPos;
    speed = 10.0f;
    sprite = spriteNew(
            pos.x, pos.y, USER_WIDTH, USER_HEIGHT,
            game->level->textures[CIRCLE_TEX]->id);

    c = color(128, 128, 255, 255);
    spriteSetColor(sprite, &c);
    game->player = userNew(pos, speed, sprite, PLAYER);
    // add the sprite to users batches
    sbAddSprite(game->usersBatch, game->player->sprite);
}

/**
 * Returns a user rectangle
 */
inline Rect userGetRect(User *user)
{
    return (Rect) {
        user->pos.x, user->pos.y,
        user->sprite->width, user->sprite->height
    };
}

/**
 * Returns nearest user relative to user
 */
User *getNearUser(Game *game, User *user) 
{
    Vec2f distVec;
    float minDistance = 0xFFFFFF, sqLen;
    User *closestUser = user;
    User *other;
    ListNode *node;

    listForEach(game->users, node, other) {
        if (other->type != HUMAN)
            continue;
        distVec = vec2fSub(other->pos, user->pos);
        sqLen = vec2fSquaredLength(distVec);
        if (sqLen < minDistance) {
            closestUser = other;
            minDistance = sqLen;
        }
    }

    return closestUser;
}

/**
 * Updates humans
 */
void humansUpdate(Game *game) 
{
    ListNode *node;
    User *human;

    listForEach(game->humans, node, human) {
        // Change his direction every 30 frames
        if (game->totalFrames % 30 == 0) {
            human->direction = vec2fRotate(human->direction, rand() % 10);
        }
        Vec2f newPos = vec2fMulS(human->direction, human->speed);
        newPos = vec2fAdd(human->pos, newPos);
        userSetPos(human, newPos);
    }
}

/**
 * Updates zombies
 */
void zombiesUpdate(Game *game)
{
    ListNode *node;
    User *zombie;

    listForEach(game->zombies, node, zombie) {
        // Hunt near human, once a second
        if (game->totalFrames % 60 == 0) {
            User *nearHuman = getNearUser(game, zombie);
            Vec2f newDir = vec2fSub(nearHuman->pos, zombie->pos);
            zombie->direction = vec2fNormalize(newDir);
        } else {
            zombie->direction =
                vec2fRotate(zombie->direction, (rand() % 10) * sin(rand() % 7));
        }
        Vec2f newPos = vec2fMulS(zombie->direction, zombie->speed);
        newPos = vec2fAdd(zombie->pos, newPos);
        userSetPos(zombie, newPos);
    }
}

/**
 * Updates player
 */
void playerUpdate(Game *game)
{
    InMgr *inmgr = game->inmgr;
    Camera *camera = game->cam;
    User *player = game->player;
    Vec2f pos;

    if (inMgrIsKeyPressed(inmgr, IM_KEY_A)) {
        pos = vec2f(player->pos.x-player->speed, player->pos.y);
        userSetPos(player, pos);
        cameraSetPosition(camera, pos.x, pos.y);
    }
    if (inMgrIsKeyPressed(inmgr, IM_KEY_D)) {
        pos = vec2f(player->pos.x + player->speed, player->pos.y);
        userSetPos(player, pos);
        cameraSetPosition(camera, pos.x, pos.y);
    }
    if (inMgrIsKeyPressed(inmgr, IM_KEY_W)) {
        pos = vec2f(player->pos.x, player->pos.y + player->speed);
        userSetPos(player, pos);
        cameraSetPosition(camera, pos.x, pos.y);
    }
    if (inMgrIsKeyPressed(inmgr, IM_KEY_S)) {
        pos = vec2f(player->pos.x, player->pos.y - player->speed);
        userSetPos(player, pos);
        cameraSetPosition(camera, pos.x, pos.y);
    }

    if (inMgrIsKeyPressed(inmgr, IM_KEY_LEFT)) {
        cameraSetPosition(camera, 2000, 2000);
    }
    if (inMgrIsKeyPressed(inmgr, IM_KEY_Q)) {
        cameraSetScale(camera, camera->scale * game->scaleSpeed);
    }
    if (inMgrIsKeyPressed(inmgr, IM_KEY_E)) {
        cameraSetScale(camera, camera->scale / game->scaleSpeed);
    }
}

void usersInit(Game *game)
{
    ListNode *node;
    User *human, *zombie, *user;

    initHumans(game);
    initZombies(game);
    initPlayer(game);

    // populate game->users; //
    game->users = listNew(NULL);
    listAdd(game->users, game->player);

    listForEach(game->humans, node, human)
        listAdd(game->users, human);

    listForEach(game->zombies, node, zombie)
        listAdd(game->users, zombie);

    listForEach(game->users, node, user) {
        AABB uBox = aabb(
                user->pos.x, user->pos.y,
                user->pos.x + USER_WIDTH, user->pos.y + USER_HEIGHT);

        user->qtObj = quadTreeAdd(game->usersTree, uBox, user);
    }
}

void usersUpdate(Game *game) 
{
    playerUpdate(game);
    humansUpdate(game);
    zombiesUpdate(game);
}

