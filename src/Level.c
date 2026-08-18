#include "Level.h"
#include <SDL3/SDL.h>

#define NUM_TX    7
const char* texture_paths[] = {
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
    "sprites/npwall.png"
};

static SDL_Texture* textures[NUM_TX] = {};

bool loadLevelTextures(SDL_Renderer* renderer) {
    SDL_Surface* surface = NULL;
    char* pngPath = NULL;

    // Skip first entry in texture_paths
    for (int i = 1; i < NUM_TX; i++) {
        SDL_asprintf(&pngPath, "%s%s", SDL_GetBasePath(), texture_paths[i]);
        surface = SDL_LoadPNG(pngPath);
        if (!surface) {
            SDL_Log("Couldn't load png: %s", SDL_GetError());
            return false;
        }

        SDL_free(pngPath);

        textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        if (!textures[i]) {
            SDL_Log("Couldn't create static texture: %s", SDL_GetError());
            return false;
        }

        SDL_DestroySurface(surface);
    }

    return true;
}

void unloadLevelTextures() {
    for (int i = 0; i < NUM_TX; i++) SDL_DestroyTexture(textures[i]);
}

Level* loadLevel(int num) {
    char* levelPath = NULL;

    SDL_asprintf(&levelPath, "%slevels/%d.txt", SDL_GetBasePath(), num);
    char* mapData = SDL_LoadFile(levelPath, NULL);
    SDL_free(levelPath);
    if (mapData == NULL) {
        SDL_Log("Could not load map: %s", SDL_GetError());
        return NULL;
    }
    Level* newLevel = SDL_calloc(1, sizeof(Level));
    newLevel->levelNum = num;

    int x = 0, y = 0;
    for (int i = 0; mapData[i] != 0; i++) {
        switch (mapData[i]) {
            case '?':
                newLevel->tiles[y][x] = TX_BUTTON;
                break;
            case 'c':
                newLevel->tiles[y][x] = TX_FLOOR;
                newLevel->cubes[newLevel->numCubes].x = x;
                newLevel->cubes[newLevel->numCubes].y = y;
                newLevel->numCubes += 1;
                break;
            case '>':
                newLevel->playerLocation.x = x;
                newLevel->playerLocation.y = y;
                // fallthrough to floor case
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
                SDL_free(newLevel);
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

void drawLevel(SDL_Renderer* renderer, Level* level, Vec2 offset) {
    SDL_FRect dst_rect = {0, 0, TILE_SIZE, TILE_SIZE};

    for (int i = 0; i < level->height; i++) {
        for (int j = 0; j < level->width; j++) {
            int tile = level->tiles[i][j];
            if (tile > 0) {
                dst_rect.x = j*TILE_SIZE + offset.x;
                dst_rect.y = i*TILE_SIZE + offset.y;
                SDL_RenderTexture(renderer, textures[tile], NULL, &dst_rect);
            }
        }
    }

    for (int i = 0; i < level->numCubes; i++) {
        dst_rect.x = level->cubes[i].x*TILE_SIZE + offset.x;
        dst_rect.y = level->cubes[i].y*TILE_SIZE + offset.y;
        SDL_RenderTexture(renderer, textures[TX_CUBE], NULL, &dst_rect);
    }

    dst_rect.x = level->playerLocation.x*TILE_SIZE + offset.x;
    dst_rect.y = level->playerLocation.y*TILE_SIZE + offset.y;
    SDL_RenderTexture(renderer, textures[TX_PLAYER], NULL, &dst_rect);
}

int isCube(Level* level, Vec2 pos) {
    for (int i = 0; i < level->numCubes; i++) {
        if (level->cubes[i].x == pos.x && level->cubes[i].y == pos.y) {
            return i;
        }
    }

    return -1;
}

bool canMove(Level* level, Vec2 pos, Vec2 dir) {
    if (isCube(level, pos) > -1) return canMove(level, (Vec2){pos.x + dir.x, pos.y + dir.y}, dir);

    switch (level->tiles[pos.y][pos.x]) {
        case TX_PWALL:
        case TX_NPWALL:
            return false;
        default:
            return true;
    }
}

void moveCube(Level* level, int cubeId, Vec2 dir) {
    Vec2 targetPos = {level->cubes[cubeId].x + dir.x, level->cubes[cubeId].y + dir.y};
    int nextCubeId = isCube(level, targetPos);
    if (nextCubeId > -1) moveCube(level, nextCubeId, dir);
    level->cubes[cubeId] = targetPos;
}

void move(Level* level, Vec2 dir) {
    Vec2 newPos = {level->playerLocation.x + dir.x, level->playerLocation.y + dir.y};
    if (canMove(level, newPos, dir)) {
        int cubeId = isCube(level, newPos);
        if (cubeId > -1) moveCube(level, cubeId, dir);
        level->playerLocation = newPos;
    }
}

void shoot(Level* level, Vec2 dir) {
    
}

bool isWon(Level* level) {
    for (int i = 0; i < level->height; i++) {
        for (int j = 0; j < level->width; j++) {
            if (level->tiles[i][j] == TX_BUTTON && isCube(level, (Vec2){j, i}) == -1) return false;
        }
    }

    return true;
}
