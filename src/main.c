#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Level.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 544

static Level* currentLevel = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char* argv[]) {
    SDL_SetAppMetadata("Portalban", "0.1", "net.ivoah.portalban");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Portalban", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    if (!loadLevelTextures(renderer)) return SDL_APP_FAILURE;

    currentLevel = loadLevel("levels/0.txt");
    if (currentLevel == NULL) return SDL_APP_FAILURE;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    } else if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_UP:
                move(currentLevel, V_UP);
                break;
            case SDL_SCANCODE_DOWN:
                move(currentLevel, V_DOWN);
                break;
            case SDL_SCANCODE_LEFT:
                move(currentLevel, V_LEFT);
                break;
            case SDL_SCANCODE_RIGHT:
                move(currentLevel, V_RIGHT);
                break;
            default:
                break;
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    SDL_SetRenderDrawColor(renderer, 0, isWon(currentLevel) ? 255 : 0, 0, SDL_ALPHA_OPAQUE);  /* black, full alpha */
    SDL_RenderClear(renderer);  /* start with a blank canvas. */

    const Vec2 center = {WINDOW_WIDTH/2 - currentLevel->width*TILE_SIZE/2, WINDOW_HEIGHT/2 - currentLevel->height*TILE_SIZE/2};
    drawLevel(renderer, currentLevel, center);

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    unloadLevelTextures();
    /* SDL will clean up the window/renderer for us. */
}
