#include <math.h>
#include "collision.h"
#include "user.h"
#include "game.h"
#include "mrb_lib/vec2f.h"
#include "mrb_lib/quad_tree.h"

// bounding check fast collision check
inline bool isColliding(Rect *a, Rect *b) 
{
    return (!(
                (a->x >= b->x + b->width) ||
                (a->x + a->width <= b->x) ||
                (a->y >= b->y + b->height) ||
                (a->y+a->height <= b->y)
    ));
}

/**
 * Gets the distance between centerpoint of two rectangles
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
 * Handles user-brick collision
 *
 * @param user User to check collision
 * @param brick Second object to check collision to.
 */
void userBrickCollision(User *user, Sprite *brick) 
{
    Vec2f distance;
    Vec2f newPos;
    Rect a = userGetRect(user);
    Rect b = {brick->x, brick->y, brick->width, brick->height};
    distance = getDistance(&a, &b);
    newPos.x = user->pos.x;
    newPos.y = user->pos.y;
    if (fabs(distance.x) > fabs(distance.y)) {
        if (distance.x < 0)             // left collision
            newPos.x = b.x + b.width;
        else if (distance.x > 0)        // right collision
            newPos.x = b.x - USER_WIDTH;
    }
    else {
        if (distance.y < 0)             // bottom collision
            newPos.y = b.y + b.height;
        else if (distance.y > 0)
            newPos.y = b.y - USER_HEIGHT;
    }
    userSetPos(user, newPos);
}

/**
 * Handles user-user collision
 */
void userUserCollision(User *a, User *b, Game *game)
{
    Vec2f distVec = vec2fSub(a->pos, b->pos);

    if (a == b)
        return;

    if (distVec.x * distVec.x + distVec.y * distVec.y < USER_WIDTH * USER_WIDTH) {
        float minDistance = USER_WIDTH;
        float distance = vec2fLength(distVec);
        float depth = minDistance - distance;

        Vec2f colDeptVec = vec2fMulS(vec2fNormalize(distVec), depth);
        // if a or b is the player, don't push
        // instead move twice the other player
        if (a->type == PLAYER) {
            userSetPos(b, vec2fSub(userGetPos(b), colDeptVec));
        } else if (b->type == PLAYER) {
            userSetPos(a, vec2fSub(userGetPos(a), colDeptVec));
        } else {
            colDeptVec = vec2fDivS(colDeptVec, 2.0f);
            userSetPos(a, vec2fAdd(userGetPos(a), colDeptVec));
            userSetPos(b, vec2fSub(userGetPos(b), colDeptVec));
        }

        if (a->type == ZOMBIE || b->type == ZOMBIE) {
            User *zombie = a->type == ZOMBIE ? a : b;
            User *other = a->type == ZOMBIE ? b : a;

            // zombie collided with human, transform human into zombie
            if (other->type == HUMAN) {
                listDel(game->humans, other);
                // transform human into zombie
                other->type = ZOMBIE;
                other->speed = 1.5f;
                spriteSetColor(other->sprite, &zombie->sprite->color);
                // and add it to zombie list
                listAdd(game->zombies, other);

                if (false && game->humans->items == 0) {
                    fprintf(stdout, "YOU LOOSE\n");
                    SDL_Delay(2000);
                    game->state = GAME_OVER;
                }
            }

            if (other->type == PLAYER) {
                // resurect zombie - transform it to human
                listDel(game->zombies, zombie);
                // transform zombie to human
                zombie->type = HUMAN;
                zombie->speed = 1.0f;
                Color color = { rand() % 128, rand() % 255, rand() % 128, 255 };
                spriteSetColor(zombie->sprite, &color);
                listAdd(game->humans, zombie);

                if (game->zombies->items == 0) {
                    fprintf(stdout, "YOU WIN!\n");
                    SDL_Delay(2000);
                    game->state = GAME_OVER;
                }
            }
        }
    }
}

/**
 * Checking all collisions
 * XXX TODO - Use only one tree, no need for two - only one and keep type
 *
 */
void checkAllCollisions(Game *game)
{
    int i;
    ListNode *node; User *user;
    AABB queryBox;
    Array *res = arrayNew();

    // check user-user collision by querying user qtree
    listForEach(game->users, node, user) {
        queryBox = aabb(
                user->pos.x, user->pos.y,
                user->pos.x + USER_WIDTH, user->pos.y + USER_HEIGHT);

        quadTreeGetIntersections(game->usersTree, queryBox, res);
        for (i = 0; i < res->len; i++) {
            userUserCollision(user, ((QTObject *)res->data[i])->data, game);
        }
        arrayReset(res);
    }

    // check user-brick collision by querying the qtree
    listForEach(game->users, node, user) {
        queryBox = aabb(
                user->pos.x, user->pos.y,
                user->pos.x + USER_WIDTH, user->pos.y + USER_HEIGHT);

        quadTreeGetIntersections(game->bricksTree, queryBox, res);
        for (i = 0; i < res->len; i++) {
            userBrickCollision(user, ((QTObject *)res->data[i])->data);
            user->direction = vec2fRotate(user->direction, rand() % 45);
        }
        arrayReset(res);
    }

    arrayDelete(&res);
}

