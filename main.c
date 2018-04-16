#include "game.h"
#include "mrb_lib/array.h"
#include "mrb_lib/aabb.h"
#include "mrb_lib/list.h"
#include "mrb_lib/quad_tree.h"
#include "mrb_lib/vec2f.h"
#include "mrb_lib/texture.h"
#include "mrb_lib/inmgr.h"

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

/* game callbacks */
int onGameInit(Game *game);
int onGameUpdate(Game *game, int ticks);
void onGameDelete(Game *game);

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

#define NUM_BALLS 200
#define BRIKSZ 64.0f
const uint16_t mapWidth = 30, mapHeight = 30;

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
    Vec2f velocity;
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

static float frand(float min, float max)
{
    float u = (float) rand() / (float) RAND_MAX;
    return min + u * (max - min);
}
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
    balls.qtree = quadTreeNew(aabb(-64, -64, 64*23, 64*21));

    // construct the map //
    uint16_t i, j;
    for (i = 0; i < mapHeight; i++) {
        for (j = 0; j < mapWidth; j++) {
            if (i == 0 || i == mapHeight - 1 || j == 0 || j == mapWidth - 1) {
                GObj *brick = calloc(1, sizeof(*brick));
                if (!brick)
                    exit(1);
                float x = j * BRIKSZ,
                    y = i * BRIKSZ;
                brick->type = TYPE_STATIC;
                brick->pos = vec2f(x, y);
                brick->dim = vec2f(BRIKSZ, BRIKSZ);
                brick->sprite = spriteNew(
                        x, y, BRIKSZ, BRIKSZ,
                        balls.textures[RBRICK]->id);
                Color col = color(255, 255, 255, 255);
                spriteSetColor(brick->sprite, &col);
                sbAddSprite(game->sBatch, brick->sprite);
                arrayPush(balls.objs, brick);
            }
        }
    }

    cameraSetPosition(game->cam, BRIKSZ*mapWidth/2, BRIKSZ*mapHeight/2);

    // init the balls //
    for (i = 0; i < NUM_BALLS; i++) {
        DynGObj *ball = calloc(1, sizeof(*ball));
        ball->obj.type = TYPE_DYNAMIC;
        ball->obj.pos = vec2f(
                frand(BRIKSZ, (float)(mapWidth - 2) * BRIKSZ),
                frand(BRIKSZ, (float)(mapHeight - 2) * BRIKSZ));
        float radius = frand(16, 46);
        ball->obj.dim = vec2f(radius, radius);
        float speed = frand(0.2, 0.4);
        Vec2f direction = vec2f(frand(-1, 1), frand(-1, 1));
        ball->velocity = vec2fMulS(direction, speed);
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
// bounding check fast collision check
bool isColliding(Rect *a, Rect *b)
{
    return (!(
                (a->x >= b->x + b->width) ||
                (a->x + a->width <= b->x) ||
                (a->y >= b->y + b->height) ||
                (a->y+a->height <= b->y)
    ));
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
    if (!isColliding(a, b))
        return;

    if (fabs(distance.x) > fabs(distance.y)) {
        if (distance.x < 0)             // left collision
            newPos.x = b->x + b->width;
        else if (distance.x > 0)        // right collision
            newPos.x = b->x - a->width;
        ball->velocity.x *= -1;
    } else {
        if (distance.y < 0)             // bottom collision
            newPos.y = b->y + b->height;
        else if (distance.y > 0)        // top collision
            newPos.y = b->y - a->height;
        ball->velocity.y *= -1;
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

    /* Does the balls really collide? */
    if (distance > minDist)
        return;
    // step back //
    float depth = minDist - distance;
    Vec2f colDeptVec = vec2fMulS(vec2fNormalize(distVec), depth);
    colDeptVec = vec2fDivS(colDeptVec, 2.0f);
    gobjSetPos((GObj *)a, vec2fAdd(a->obj.pos, colDeptVec));
    gobjSetPos((GObj *)b, vec2fSub(b->obj.pos, colDeptVec));

    /**
     * Change directions and speeds
     * Let's say density of all balls equals 1
     * mass = volume / density => mass == volume
     * sphere volume = 4*PI*r^3/3
     * v1New = (v1(vol1 - vol2) + 2vol2v2) / (vol1 + vol2)
     * v1New = (v1(4*PI*r1^3/3 - 4*PI*r2^3/3) + 2*v2*4*PI*r2^3/3) 
     *          / (4*PI*r1^3/3 + 4*PI*r2^3/3)
     * v1New = (4*PI/3 * v1( r1^3 - r2^3) + 2v2r2^3) / 4*PI/3(r1^3 + r2^3)
     * v1New = v1(r1^3 - r2^3) / (r1^3 + r2^3)
     */
    float r1 = a->obj.dim.x / 2;
    float r1cb = r1 * r1 * r1;
    float r2 = b->obj.dim.x / 2;
    float r2cb = r2 * r2 * r2;

    Vec2f v1New = vec2fMulS(a->velocity, r1 - r2);
    v1New = vec2fAdd(v1New, vec2fMulS(vec2fMulS(b->velocity, 2), r2));
    v1New = vec2fDivS(v1New, (r1 + r2));

    Vec2f v2New = vec2fMulS(b->velocity, r2 - r1);
    v2New = vec2fAdd(v2New, vec2fMulS(vec2fMulS(a->velocity, 2), r1));
    v2New = vec2fDivS(v2New, (r1 + r2));
    a->velocity = v1New;
    b->velocity = v2New;
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

void updateCamera(Game *game, int ticks)
{
    InMgr *mgr = game->inmgr;
    Camera *cam = game->cam;
    Vec2f velocity = vec2f(0, 0);
    Vec2f pos;

    if (inMgrIsKeyPressed(mgr, IM_KEY_W))
        velocity.y = 1;
    if (inMgrIsKeyPressed(mgr, IM_KEY_S))
        velocity.y = -1;
    if (inMgrIsKeyPressed(mgr, IM_KEY_A))
        velocity.x = -1;
    if (inMgrIsKeyPressed(mgr, IM_KEY_D))
        velocity.x = 1;
    if (inMgrIsKeyPressed(mgr, IM_KEY_Q))
        cameraSetScale(cam, cam->scale * game->scaleSpeed);
    if (inMgrIsKeyPressed(mgr, IM_KEY_E))
        cameraSetScale(cam, cam->scale / game->scaleSpeed);

    pos = vec2fMulS(velocity, (float)ticks);
    pos = vec2fAdd(vec2f(cam->position.x, cam->position.y), pos);
    cameraSetPosition(cam, pos.x, pos.y);
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
        Vec2f newPos = vec2fMulS(ent->velocity, (float)ticks);
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
#if 0
    int j;
    for (i = 0; i < balls.objs->len; i++) {
        for (j = 0; j < balls.objs->len; j++) {
            DynGObj *a = balls.objs->data[i];
            DynGObj *b = balls.objs->data[j];
            checkCollision(a, b);
        }
    }
#endif
    updateCamera(game, ticks);
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

