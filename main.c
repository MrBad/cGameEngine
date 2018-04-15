#include "game.h"
#include "mrb_lib/array.h"
#include "mrb_lib/aabb.h"
#include "mrb_lib/list.h"
#include "mrb_lib/quad_tree.h"
#include "mrb_lib/vec2f.h"
#include "mrb_lib/texture.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#ifdef COMPILE_TESTS
void doTests()
{
    arrayTest();
    aabbTest();
    listTest();
    quadTreeTest();
    printf("\n");
}
#endif // COMPILE_TESTS

int onGameInit(Game *game);
int onGameUpdate(Game *game, int ticks);
void onGameDelete(Game *game);

#define NUM_BALLS 50

int main()
{

#ifdef COMPILE_TESTS
    doTests();
#endif

    Game *game;

    if (!(game = gameNew()))
        return 1;

    game->onGameInit =   onGameInit;
    game->onGameUpdate = onGameUpdate;
    game->onGameDelete = onGameDelete;

    if (!gameInit(game, WINDOW_WIDTH, WINDOW_HEIGHT, "Some test"))
        return 1;

    return 0;
}

enum { TYPE_STATIC, TYPE_DYNAMIC } objType;

typedef struct {
    Vec2f pos;      // Position of the object
    Vec2f dim;      // Dimension
    Sprite *sprite; // Attached sprite
    QTObject *qtObj; // Reference to QuadTree object
    uint8_t type;
} GObj;

typedef struct {
    GObj obj;
    float speed;
    Vec2f direction;
} DynGObj;

#define ARR_LEN(a) (sizeof(a)/sizeof(a[0]))

enum { CIRCLE, RBRICK, LBRICK, GLASS, NUM_TEXTURES } texture_types;

/* timeout queue stuff - WIP */
typedef int (*CallBack) (void *context);
typedef struct TimeOut {
    CallBack fn;
    void *context;
} TimeOutObj;

/* A structure to avoid globals */
struct BallsGame {
    Array *objs;
    Texture *textures[NUM_TEXTURES];
    DynGObj *player;
    QuadTree *qtree;
    Array *tmoQueue;    // Time out queue
} balls;

/**
 * Called by the game engine, on start
 *
 * @param game The passed game
 * @return 0 on success, -1 on error
 */
int onGameInit(Game *game)
{
    balls.textures[RBRICK] = loadTexture("resources/red_bricks.png");
    balls.textures[CIRCLE] = loadTexture("resources/circle.png");
    balls.textures[LBRICK] = loadTexture("resources/light_bricks.png");
    balls.textures[GLASS] = loadTexture("resources/glass.png");
    balls.objs = arrayNew();
    balls.qtree = quadTreeNew(aabb(0, 0, 64*23, 64*21));

    // construct the map //
    uint16_t i, j, 
             mapWidth = 22, 
             mapHeight = 20;

    for (i = 0; i < mapHeight; i++) {
        for (j = 0; j < mapWidth; j++) {
            if (i == 0 || i == mapHeight - 1 || j == 0 || j == mapWidth - 1) {
                GObj *brick = calloc(1, sizeof(*brick));
                if (!brick)
                    exit(1);
                int x = j * 64,
                    y = i * 64;
                brick->type = TYPE_STATIC;
                brick->pos = vec2f(x, y);
                brick->dim = vec2f(64, 64);
                brick->sprite = spriteNew(
                        x, y, 64, 64, 
                        balls.textures[RBRICK]->id);
                Color col = color(255, 255, 255, 255);
                spriteSetColor(brick->sprite, &col);
                sbAddSprite(game->sBatch, brick->sprite);
                arrayPush(balls.objs, brick);
            }
        }
    }

    cameraSetPosition(game->cam, 64*10, 64*10);

    // init the balls //
    for (i = 0; i < NUM_BALLS; i++) {
        DynGObj *ball = calloc(1, sizeof(*ball));
        ball->obj.type = TYPE_DYNAMIC;
        ball->obj.pos = vec2f(
                64 + rand() % (mapWidth - 2) * 64, 
                64 + rand() % (mapHeight - 2) * 64);
        ball->obj.dim = vec2f(64, 64);
        ball->speed = 0.2f + (float)rand() / RAND_MAX / 2;
        ball->direction = vec2f(
                (float) rand() / RAND_MAX,
                (float) rand() / RAND_MAX);
        ball->obj.sprite = spriteNew(
                ball->obj.pos.x, ball->obj.pos.y, 
                ball->obj.dim.x, ball->obj.dim.y,
                balls.textures[CIRCLE]->id);
        Color col = color(rand() % 255, rand() % 255, rand() % 255, 180);
        spriteSetColor(ball->obj.sprite, &col);
        sbAddSprite(game->sBatch, ball->obj.sprite);
        arrayPush(balls.objs, ball);
    }

    // add all objects to quad tree //
    GObj *ent;
    arrayForEach(balls.objs, ent, i) {
        AABB box = aabb(
                ent->pos.x, ent->pos.y,
                ent->pos.x + ent->dim.x, ent->pos.y + ent->dim.y);
        ent->qtObj = quadTreeAdd(balls.qtree, box, ent);
    }

    return 0;
}

/**
 * Gets the distance between centerpoints of two rectangles
 *
 * @param a First rectangle
 * @param b Second rectangle
 * @return distVect The distance vector between their centers
 */
Vec2f getDistance(Rect *a, Rect *b)
{
    Vec2f centerA = {a->x + a->width / 2, a->y + a->height / 2};
    Vec2f centerB = {b->x + b->width / 2, b->y + b->height / 2};
    Vec2f distVec = vec2fSub(centerB, centerA);

    return distVec;
}

/**
 * Sets the Game object position
 */
void gobjSetPos(GObj *obj, Vec2f newPos)
{
    obj->pos = newPos;
    AABB box = aabb(
            obj->pos.x,  obj->pos.y, 
            obj->pos.x + obj->dim.x, 
            obj->pos.y + obj->dim.y);

    qtObjectUpdate(obj->qtObj, box);

    if (obj->sprite) {
        obj->sprite->x = newPos.x;
        obj->sprite->y = newPos.y;
    }
}

/**
 * Handle ball-brick collision
 */
void ballBrickCollision(DynGObj *ball, GObj *brick)
{
    Rect *a = (Rect *) ball;    // C polymorphic thing
    Rect *b = (Rect *) brick;   // by casting an aligned structure
    Vec2f newPos = ball->obj.pos;
    Vec2f distance = getDistance(a, b);
    float minLen = ball->obj.dim.x / 2 + brick->dim.x / 2;

    /* Does the ball really hit the brick? */
    if (vec2fSquaredLength(distance) > minLen*minLen)
        return;

    if (fabs(distance.x) > fabs(distance.y)) {
        if (distance.x < 0)             // left collision
            newPos.x = b->x + b->width;
        else if (distance.x > 0)        // right collision
            newPos.x = b->x - a->width;
        ball->direction.x *= -1.f;
    } else {
        if (distance.y < 0)             // bottom collision
            newPos.y = b->y + b->height;
        else if (distance.y > 0)        // top collision
            newPos.y = b->y - a->height;
        ball->direction.y *= -1.f;
    }
    gobjSetPos((GObj *)ball, newPos);
    
    Color red = color(255, 128, 128, 255);
    spriteSetColor(brick->sprite, &red);
}

void ballBallCollision(DynGObj *a, DynGObj *b)
{
    Vec2f distVec = vec2fSub(a->obj.pos, b->obj.pos);
    float distance = vec2fLength(distVec);
    float minDist = a->obj.dim.x / 2 + b->obj.dim.x / 2;

    if (distance > minDist)
        return;
    // step back //
    float depth = minDist - distance;
    Vec2f colDeptVec = vec2fMulS(vec2fNormalize(distVec), depth);
    colDeptVec = vec2fDivS(colDeptVec, 2.0f);
    gobjSetPos((GObj *)a, vec2fAdd(a->obj.pos, colDeptVec));
    gobjSetPos((GObj *)b, vec2fSub(b->obj.pos, colDeptVec));

    /**
     * TODO: add masses to balls
     * Change directions and speeds
     * v1New = (v1(m1 - m2) + 2m2v2) / (m1 + m2)
     * considering m1 == m2 == 1
     * v1New = (v1 * 0 + 2 * 1 v2) / 2 => v1New = v2, 
     * v2New = v1
     */
    Vec2f dir = a->direction;
    float speed = a->speed;
    a->direction = b->direction;
    a->speed = b->speed;
    b->direction = dir;
    b->speed = speed;
}

/**
 * Checks if 2 objects collide and call the specialized function
 */
void checkCollision(DynGObj *a, DynGObj *b)
{
    if (a->obj.type == TYPE_DYNAMIC && b->obj.type == TYPE_STATIC)
        ballBrickCollision(a, (GObj *) b);
    else if (a->obj.type == TYPE_STATIC && b->obj.type == TYPE_DYNAMIC)
        ballBrickCollision(b, (GObj*) a);
    else if (a->obj.type == TYPE_DYNAMIC && b->obj.type == TYPE_DYNAMIC)
        ballBallCollision(a, b);
}

/**
 * Called when game updates, on each frame
 *
 * @param game The game structure passed in
 * @param ticks How many ms passed since last call
 * @return 0 on success, -1 on error
 */
int onGameUpdate(Game *game, int ticks)
{
    (void) game;
    int i;
    static int nTicks = 0;

    nTicks += ticks;
    if (nTicks >= 1000) {
        //printf("One second passed %d\n", nTicks);
        nTicks = 0;
    }
    // update objects //
    DynGObj *ent;
    arrayForEach(balls.objs, ent, i) {
        if (ent->obj.type != TYPE_DYNAMIC)
            continue;
        Vec2f newPos = vec2fMulS(ent->direction, ent->speed * ticks);
        newPos = vec2fAdd(ent->obj.pos, newPos);
        gobjSetPos((GObj *)ent, newPos);
    }
    Array *res = arrayNew();
    // check collisions //
    arrayForEach(balls.objs, ent, i) {
        if (ent->obj.type != TYPE_DYNAMIC)
            continue;
        AABB queryBox = aabb(
                ent->obj.pos.x, ent->obj.pos.y, 
                ent->obj.pos.x + ent->obj.dim.x, 
                ent->obj.pos.y + ent->obj.dim.y);
        quadTreeGetIntersections(balls.qtree, queryBox, res);
        if (res->len > 1) {
            QTObject *qtObj;
            int j;
            arrayForEach(res, qtObj, j) {
                DynGObj *obj = qtObj->data;
                if (ent != obj)
                    checkCollision(ent, obj);
            }
        }
        arrayReset(res);
    }
    arrayDelete(&res);

    return 0;
}

/**
 * Called when the game ends, to free up resources
 * 
 * @param game The game
 */
void onGameDelete(Game *game)
{
    (void) game;
    int i;

    for (i = 0; i < NUM_TEXTURES; i++) {
        if (balls.textures[i])
            textureDelete(balls.textures[i]);
    }
    DynGObj *ent;
    arrayForEach(balls.objs, ent, i) {
        if (ent->obj.sprite)
            spriteDelete(ent->obj.sprite);
        free(ent);
    }
    quadTreeDelete(balls.qtree);
    printf("gameDeleted\n");
}

