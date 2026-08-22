#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Level.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 544

static Level* currentLevel = NULL;
static SDL_Gamepad* gamepad = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char* argv[]) {
    SDL_SetAppMetadata("Portalban", "0.1", "net.ivoah.portalban");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Portalban", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    if (!SDL_SetRenderVSync(renderer, 1)) SDL_Log("Could not set vsync: %s", SDL_GetError());

    if (!Level_loadTextures(renderer)) return SDL_APP_FAILURE;

    currentLevel = Level_load(0);
    if (currentLevel == NULL) return SDL_APP_FAILURE;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    int levelToLoad = -1;
    Vec2* moveDir = NULL;
    Vec2* shotDir = NULL;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
            switch (event->key.scancode) {
                case SDL_SCANCODE_ESCAPE: return SDL_APP_SUCCESS;
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_W: moveDir = &V_UP; break;
                case SDL_SCANCODE_DOWN:
                case SDL_SCANCODE_S: moveDir = &V_DOWN; break;
                case SDL_SCANCODE_LEFT:
                case SDL_SCANCODE_A: moveDir = &V_LEFT; break;
                case SDL_SCANCODE_RIGHT:
                case SDL_SCANCODE_D: moveDir = &V_RIGHT; break;
                case SDL_SCANCODE_I: shotDir = &V_UP; break;
                case SDL_SCANCODE_K: shotDir = &V_DOWN; break;
                case SDL_SCANCODE_J: shotDir = &V_LEFT; break;
                case SDL_SCANCODE_L: shotDir = &V_RIGHT; break;
                default: break;
            }
            switch (event->key.key) {
                case SDLK_R: Level_undo(currentLevel, -1); break;
                case SDLK_COMMA: levelToLoad = currentLevel->levelNum - 1; break;
                case SDLK_PERIOD: levelToLoad = currentLevel->levelNum + 1; break;
                case SDLK_U: Level_undo(currentLevel, 1); break;
                default: break;
            }
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            switch (event->gbutton.button) {
                case SDL_GAMEPAD_BUTTON_DPAD_UP:    moveDir = &V_UP; break;
                case SDL_GAMEPAD_BUTTON_DPAD_DOWN:  moveDir = &V_DOWN; break;
                case SDL_GAMEPAD_BUTTON_DPAD_LEFT:  moveDir = &V_LEFT; break;
                case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: moveDir = &V_RIGHT; break;
                case SDL_GAMEPAD_BUTTON_NORTH: shotDir = &V_UP; break;
                case SDL_GAMEPAD_BUTTON_SOUTH: shotDir = &V_DOWN; break;
                case SDL_GAMEPAD_BUTTON_WEST:  shotDir = &V_LEFT; break;
                case SDL_GAMEPAD_BUTTON_EAST:  shotDir = &V_RIGHT; break;
                case SDL_GAMEPAD_BUTTON_BACK:  Level_undo(currentLevel, 1); break;
                case SDL_GAMEPAD_BUTTON_START: Level_undo(currentLevel, -1); break;
                default: break;
            }
            break;
        case SDL_EVENT_GAMEPAD_ADDED:
            SDL_Log("Gamepad added: %d", event->gdevice.which);
            if (gamepad == NULL) gamepad = SDL_OpenGamepad(event->gbutton.which);
            break;
    }

    if (moveDir != NULL) Level_move(currentLevel, *moveDir);
    if (shotDir != NULL) Level_shoot(currentLevel, *shotDir);

    if (levelToLoad != -1) {
        levelToLoad = levelToLoad%10;
        Level_free(currentLevel);
        currentLevel = Level_load(levelToLoad);
        if (currentLevel == NULL) return SDL_APP_SUCCESS;
    }

    if (currentLevel->state->lastShotBlue) SDL_SetGamepadLED(gamepad, 0, 0, 255);
    else                                   SDL_SetGamepadLED(gamepad, 255, 35, 0);

    return SDL_APP_CONTINUE;
}

Uint64 lastTime = 0, currentTime = 0;
SDL_AppResult SDL_AppIterate(void* appstate) {
    currentTime = SDL_GetTicks();
    Uint64 delta = currentTime - lastTime;
    lastTime = currentTime;

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Draw level
    const Vec2 center = {WINDOW_WIDTH/2 - currentLevel->width*TILE_SIZE/2, WINDOW_HEIGHT/2 - currentLevel->height*TILE_SIZE/2};
    Level_draw(renderer, currentLevel, center);

    SDL_RenderPresent(renderer);

    // Advance level if won
    if (Level_isWon(currentLevel)) {
        int nextLevel = currentLevel->levelNum + 1;
        Level_free(currentLevel);
        currentLevel = Level_load(nextLevel);
        if (currentLevel == NULL) return SDL_APP_SUCCESS;
    } 

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    if (currentLevel != NULL) Level_free(currentLevel);
    if (gamepad != NULL) SDL_CloseGamepad(gamepad);
    Level_unloadTextures();
    /* SDL will clean up the window/renderer for us. */
}
