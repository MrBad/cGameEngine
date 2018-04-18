#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include "game.h"
#include "mrb_lib/timer.h"

#define SIZE(a) sizeof(a)/sizeof(*a)

/**
 * Creates a new game
 */
Game *gameNew() 
{
    Game *game;

    if (!(game = calloc(1, sizeof(*game)))) {
        fprintf(stderr, "Cannot alloc memory for Game\n");
        return NULL;
    }

    return game;
}

/**
 * Compile and link the shaders
 */
static bool gameInitShaders(Game *game) 
{
    if (!glProgramCompileShaders(game->prog, "shaders/sprite_shader"))
        return false;
    glProgramAddAttribute(game->prog, "vertexPosition");
    glProgramAddAttribute(game->prog, "vertexColor");
    if (!glProgramLinkShaders(game->prog))
        return false;

    return true;
}

/**
 * Initialize the game
 */
bool gameInit(Game *game, int winWidth, int winHeight, const char *title) 
{
    game->state = GAME_PLAYING;
    srand(time(NULL));

    if (!(game->win = windowNew(title, winWidth, winHeight, 0))) {
        fprintf(stderr, "Cannot init window\n");
        return false;
    }
    if (!(game->inmgr = inMgrNew())) {
        fprintf(stderr, "Cannot init Input Manager\n");
        return false;
    }
    if (!(game->cam = cameraNew(winWidth, winHeight))) {
        fprintf(stderr, "Cannot init Camera\n");
        return false;
    }
    if (!(game->prog = glProgramNew())) {
        fprintf(stderr, "Cannot init glProgram\n");
        return false;
    }
    if (!gameInitShaders(game)) {
        fprintf(stderr, "Cannot init shaders\n");
        return false;
    }
    game->cam->scale = 0.3f;
    game->scaleSpeed = 1.001f;
    cameraSetPosition(game->cam, 0, 0);

    game->sBatch = sbNew(game->prog);
    sbInit(game->sBatch);

    if (game->onGameInit)
        if (game->onGameInit(game) < 0)
            return -1;

    gameLoop(game);
    gameDelete(game);

    return true;
}

void gameDelete(Game *game) 
{
    if (game->onGameDelete)
        game->onGameDelete(game);

    if (game->prog) {
        glProgramDelete(game->prog);
        game->prog = NULL;
    }
    if (game->inmgr) {
        inMgrDelete(game->inmgr);
        game->inmgr = NULL;
    }
    if (game->cam) {
        cameraDelete(game->cam);
        game->cam = NULL;
    }
    sbDelete(game->sBatch);
    windowDelete(game->win);
    free(game);
}

void printFPS(uint32_t ticks)
{
    static int fpsTicks = 0;
    static int fpsNumFrames = 0;

    fpsTicks += ticks;
    fpsNumFrames++;
    if (fpsTicks >= 1000) {
        fpsTicks = 0;
        printf("FPS: %d\n", fpsNumFrames);
        fpsNumFrames = 0;
    }
}

void gameLoop(Game *game) 
{
    Timer *timer = timerNew(SDL_GetTicks());

    while (game->state == GAME_PLAYING) {
        /* Compute the timer */
        uint32_t diffTicks = timerUpdate(timer, SDL_GetTicks());
        printFPS(diffTicks);
        game->totalFrames++;

        inMgrUpdate(game->inmgr);
        if (game->inmgr->quitRequested) {
            game->state = GAME_OVER;
        }

        game->onGameUpdate(game, diffTicks);

        cameraUpdate(game->cam);

        windowClear();
        glProgramUse(game->prog);
        glActiveTexture(GL_TEXTURE0);

        // send matrix location
        GLint pLocation = glGetUniformLocation(game->prog->programID, "P");
        glUniformMatrix4fv(
                pLocation, 1, GL_FALSE,
                &(game->cam->cameraMatrix.m[0][0]));

        // build vertices //
        sbBuildBatches(game->sBatch);
        sbDrawBatches(game->sBatch);

        glProgramUnuse(game->prog);
        windowUpdate(game->win);
    }
    //
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    timerDelete(timer);
}

