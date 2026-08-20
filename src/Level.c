#include "Level.h"
#include <SDL3/SDL.h>

const char* Level_texturePaths[] = {
    NULL,
#define TX_BUTTON 1
    "sprites/button.png",
#define TX_CUBE   2
    "sprites/cube.png",
#define TX_PLAYER 3
    "sprites/bendy.png",
#define TX_FLOOR  4
    "sprites/floor.png",
#define TX_PWALL  5
    "sprites/pwall.png",
#define TX_NPWALL 6
    "sprites/npwall.png",
#define TX_ORANGEPORTAL 7
    "sprites/orangePortal.png",
#define TX_BLUEPORTAL 8
    "sprites/bluePortal.png",
#define TX_LEVEL 9
    "sprites/level.png",
#define TX_MOVES 10
    "sprites/moves.png"
};
#define NUM_TX    11

static SDL_Texture* Level_textures[NUM_TX];
static SDL_Texture* Level_numbersTexture = NULL;

SDL_Texture* Level_loadTexture(SDL_Renderer* renderer, const char* path) {
    char* pngPath = NULL;
    SDL_Surface* surface = NULL;
    SDL_Texture* texture = NULL;

    SDL_asprintf(&pngPath, "%s%s", SDL_GetBasePath(), path);
    surface = SDL_LoadPNG(pngPath);
    SDL_free(pngPath);

    if (!surface) {
        SDL_Log("Couldn't load png: %s", SDL_GetError());
        return NULL;
    }
    
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        return NULL;
    }

    SDL_DestroySurface(surface);

    return texture;
}

bool Level_loadTextures(SDL_Renderer* renderer) {
    // Skip first entry in texture_paths
    for (int i = 1; i < NUM_TX; i++) {
        Level_textures[i] = Level_loadTexture(renderer, Level_texturePaths[i]);
        if (Level_textures[i] == NULL) return false;
    }

    Level_numbersTexture = Level_loadTexture(renderer, "sprites/numbers.png");
    if (Level_numbersTexture == NULL) return false;

    return true;
}

void Level_unloadTextures() {
    for (int i = 0; i < NUM_TX; i++)
        if (Level_textures[i] != NULL) SDL_DestroyTexture(Level_textures[i]);
    if (Level_numbersTexture != NULL) SDL_DestroyTexture(Level_numbersTexture);
}

Level* Level_load(int num) {
    char* levelPath = NULL;

    SDL_asprintf(&levelPath, "%slevels/%d.txt", SDL_GetBasePath(), num);
    char* mapData = SDL_LoadFile(levelPath, NULL);
    SDL_free(levelPath);
    if (mapData == NULL) {
        SDL_Log("Could not load map: %s", SDL_GetError());
        return NULL;
    }
    Level* newLevel = SDL_calloc(1, sizeof(Level));
    newLevel->state = SDL_calloc(1, sizeof(LevelState));
    newLevel->levelNum = num;

    int x = 0, y = 0;
    for (int i = 0; mapData[i] != 0; i++) {
        switch (mapData[i]) {
            case '?':
                newLevel->tiles[y][x] = TX_BUTTON;
                break;
            case 'c':
                if (newLevel->numCubes >= MAX_CUBES) {
                    SDL_Log("too many cubes!");
                    Level_free(newLevel);
                    SDL_free(mapData);
                    return NULL;
                }
                newLevel->tiles[y][x] = TX_FLOOR;
                newLevel->state->cubes[newLevel->numCubes].x = x;
                newLevel->state->cubes[newLevel->numCubes].y = y;
                newLevel->numCubes += 1;
                break;
            case '>':
                newLevel->tiles[y][x] = TX_FLOOR;
                newLevel->state->playerLocation.x = x;
                newLevel->state->playerLocation.y = y;
                break;
            case '.':
                newLevel->tiles[y][x] = TX_FLOOR;
                break;
            case '#':
                newLevel->tiles[y][x] = TX_PWALL;
                break;
            case '@':
                newLevel->tiles[y][x] = TX_NPWALL;
                break;
            case '\n':
                newLevel->width = SDL_max(newLevel->width, x);
                x = -1;
                y++;
                break;
            case ' ':
                break;
            default:
                SDL_Log("Unknown character in map file at (%d, %d): %c", x, y, mapData[i]);
                Level_free(newLevel);
                SDL_free(mapData);
                return NULL;
        }
        x += 1;
    }
    newLevel->width = SDL_max(newLevel->width, x);
    newLevel->height = x == 0 ? y : y + 1;

    SDL_free(mapData);

    return newLevel;
}

void Level_freeState(LevelState* state) {
    if (state->lastState != NULL) Level_freeState(state->lastState);
    SDL_free(state);
}

void Level_free(Level* level) {
    SDL_free(level->state);
    SDL_free(level);
}

void Level_drawNumber(SDL_Renderer* renderer, int num, int minDigits, Vec2 pos, Uint64 time) {
    SDL_FRect src_rect = {0, TILE_SIZE*((time/500)%(Level_numbersTexture->h/TILE_SIZE)), TILE_SIZE, TILE_SIZE};
    SDL_FRect dst_rect = {0, pos.y, TILE_SIZE, TILE_SIZE};

    int digits = SDL_max(minDigits, SDL_log10(num) + 1);
    for (int d = 0; d < digits; d++) {
        dst_rect.x = pos.x + (digits - (d + 1))*TILE_SIZE;
        src_rect.x = (num/(int)SDL_pow(10, d)%10)*TILE_SIZE;
        SDL_RenderTexture(renderer, Level_numbersTexture, &src_rect, &dst_rect);
    }
}

void Level_draw(SDL_Renderer* renderer, Level* level, Vec2 offset, Uint64 time) {
    SDL_FRect src_rect = {0, 0, TILE_SIZE, TILE_SIZE};
    SDL_FRect dst_rect = {0, 0, TILE_SIZE, TILE_SIZE};

    // Draw map
    for (int i = 0; i < level->height; i++) {
        for (int j = 0; j < level->width; j++) {
            int tile = level->tiles[i][j];
            if (tile > 0) {
                dst_rect.x = j*TILE_SIZE + offset.x;
                dst_rect.y = i*TILE_SIZE + offset.y;
                SDL_RenderTexture(renderer, Level_textures[tile], NULL, &dst_rect);
            }
        }
    }

    // Draw cubes
    for (int i = 0; i < level->numCubes; i++) {
        dst_rect.x = level->state->cubes[i].x*TILE_SIZE + offset.x;
        dst_rect.y = level->state->cubes[i].y*TILE_SIZE + offset.y;
        SDL_RenderTexture(renderer, Level_textures[TX_CUBE], NULL, &dst_rect);
    }

    // Draw player
    dst_rect.x = level->state->playerLocation.x*TILE_SIZE + offset.x;
    dst_rect.y = level->state->playerLocation.y*TILE_SIZE + offset.y;
    SDL_RenderTexture(renderer, Level_textures[TX_PLAYER], NULL, &dst_rect);

    // Draw level number
    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.w = Level_textures[TX_LEVEL]->w;
    SDL_RenderTexture(renderer, Level_textures[TX_LEVEL], NULL, &dst_rect);
    Level_drawNumber(renderer, level->levelNum, 2, (Vec2){Level_textures[TX_LEVEL]->w, 0}, time);

    // Draw move counter
    dst_rect.x = 0;
    dst_rect.y = Level_textures[TX_LEVEL]->h;
    dst_rect.w = Level_textures[TX_MOVES]->w;
    SDL_RenderTexture(renderer, Level_textures[TX_MOVES], NULL, &dst_rect);
    Level_drawNumber(renderer, level->state->moves, 2, (Vec2){Level_textures[TX_MOVES]->w, Level_textures[TX_LEVEL]->h}, time);
}

int Level_isCube(Level* level, Vec2 pos) {
    for (int i = 0; i < level->numCubes; i++) {
        if (level->state->cubes[i].x == pos.x && level->state->cubes[i].y == pos.y) {
            return i;
        }
    }

    return -1;
}

bool Level_canMove(Level* level, Vec2 pos, Vec2 dir) {
    if (Level_isCube(level, pos) > -1) return Level_canMove(level, (Vec2){pos.x + dir.x, pos.y + dir.y}, dir);

    switch (level->tiles[pos.y][pos.x]) {
        case TX_PWALL:
        case TX_NPWALL:
            return false;
        default:
            return true;
    }
}

void Level_newState(Level* level) {
    LevelState* newState = SDL_malloc(sizeof(LevelState));
    SDL_memcpy(newState, level->state, sizeof(LevelState));
    newState->lastState = level->state;
    level->state = newState;
    level->state->moves++;
}

void Level_moveCube(Level* level, int cubeId, Vec2 dir) {
    Vec2 targetPos = {level->state->cubes[cubeId].x + dir.x, level->state->cubes[cubeId].y + dir.y};
    int nextCubeId = Level_isCube(level, targetPos);
    if (nextCubeId > -1) Level_moveCube(level, nextCubeId, dir);
    level->state->cubes[cubeId] = targetPos;
}

void Level_move(Level* level, Vec2 dir) {
    Vec2 newPos = {level->state->playerLocation.x + dir.x, level->state->playerLocation.y + dir.y};
    if (Level_canMove(level, newPos, dir)) {
        Level_newState(level);
        int cubeId = Level_isCube(level, newPos);
        if (cubeId > -1) Level_moveCube(level, cubeId, dir);
        level->state->playerLocation = newPos;
    }
}

void Level_shoot(Level* level, Vec2 dir) {
    Level_newState(level);
}

void Level_undo(Level* level) {
    if (level->state->lastState != NULL) {
        LevelState* tmp = level->state;
        level->state = level->state->lastState;
        SDL_free(tmp);
    }
}

bool Level_isWon(Level* level) {
    for (int i = 0; i < level->height; i++) {
        for (int j = 0; j < level->width; j++) {
            if (level->tiles[i][j] == TX_BUTTON && Level_isCube(level, (Vec2){j, i}) == -1) return false;
        }
    }

    return true;
}
