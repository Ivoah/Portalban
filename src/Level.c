#include "Level.h"

const char* Level_texturePaths[] = {
    NULL,
#define L_BUTTON 1
    "sprites/button.png",
#define L_CUBE   2
    "sprites/cube.png",
#define L_PLAYER 3
    "sprites/bendy.png",
#define L_FLOOR  4
    "sprites/floor.png",
#define L_PWALL  5
    "sprites/pwall.png",
#define L_NPWALL 6
    "sprites/npwall.png",
#define L_ORANGEPORTAL 7
    "sprites/orangePortal.png",
#define L_BLUEPORTAL 8
    "sprites/bluePortal.png",
#define L_LEVEL 9
    "sprites/level.png",
#define L_MOVES 10
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
                newLevel->tiles[y][x] = L_BUTTON;
                break;
            case 'c':
                if (newLevel->numCubes >= MAX_CUBES) {
                    SDL_Log("too many cubes!");
                    Level_free(newLevel);
                    SDL_free(mapData);
                    return NULL;
                }
                newLevel->tiles[y][x] = L_FLOOR;
                newLevel->state->cubes[newLevel->numCubes].x = x;
                newLevel->state->cubes[newLevel->numCubes].y = y;
                newLevel->numCubes += 1;
                break;
            case '>':
                newLevel->tiles[y][x] = L_FLOOR;
                newLevel->state->playerLocation.x = x;
                newLevel->state->playerLocation.y = y;
                break;
            case '.':
                newLevel->tiles[y][x] = L_FLOOR;
                break;
            case '#':
                newLevel->tiles[y][x] = L_PWALL;
                break;
            case '@':
                newLevel->tiles[y][x] = L_NPWALL;
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

void Level_drawNumber(SDL_Renderer* renderer, int num, int minDigits, Vec2 pos) {
    SDL_FRect src_rect = {0, TILE_SIZE*((SDL_GetTicks()/500)%(Level_numbersTexture->h/TILE_SIZE)), TILE_SIZE, TILE_SIZE};
    SDL_FRect dst_rect = {0, pos.y, TILE_SIZE, TILE_SIZE};

    int digits = SDL_max(minDigits, SDL_log10(num) + 1);
    for (int d = 0; d < digits; d++) {
        dst_rect.x = pos.x + (digits - (d + 1))*TILE_SIZE;
        src_rect.x = (num/(int)SDL_pow(10, d)%10)*TILE_SIZE;
        SDL_RenderTexture(renderer, Level_numbersTexture, &src_rect, &dst_rect);
    }
}

void Level_draw(SDL_Renderer* renderer, Level* level, Vec2 offset) {
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
        SDL_RenderTexture(renderer, Level_textures[L_CUBE], NULL, &dst_rect);
    }

    // Draw portals
    src_rect.x = (SDL_GetTicks()/100)%TILE_SIZE;
    if (level->state->bluePortal.exists) {
        dst_rect.x = level->state->bluePortal.pos.x*TILE_SIZE + offset.x;
        dst_rect.y = level->state->bluePortal.pos.y*TILE_SIZE + offset.y;
        SDL_RenderTextureRotated(renderer, Level_textures[L_BLUEPORTAL], &src_rect, &dst_rect, level->state->bluePortal.r*90, NULL, SDL_FLIP_NONE);
    }
    if (level->state->orangePortal.exists) {
        dst_rect.x = level->state->orangePortal.pos.x*TILE_SIZE + offset.x;
        dst_rect.y = level->state->orangePortal.pos.y*TILE_SIZE + offset.y;
        SDL_RenderTextureRotated(renderer, Level_textures[L_ORANGEPORTAL], &src_rect, &dst_rect, level->state->orangePortal.r*90, NULL, SDL_FLIP_NONE);
    }

    // Draw player
    dst_rect.x = level->state->playerLocation.x*TILE_SIZE + offset.x;
    dst_rect.y = level->state->playerLocation.y*TILE_SIZE + offset.y;
    SDL_RenderTexture(renderer, Level_textures[L_PLAYER], NULL, &dst_rect);

    // Draw level number
    src_rect = (SDL_FRect){0, TILE_SIZE*((SDL_GetTicks()/500)%(Level_textures[L_LEVEL]->h/TILE_SIZE)), Level_textures[L_LEVEL]->w, TILE_SIZE};
    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.w = Level_textures[L_LEVEL]->w;
    SDL_RenderTexture(renderer, Level_textures[L_LEVEL], &src_rect, &dst_rect);
    Level_drawNumber(renderer, level->levelNum, 2, (Vec2){Level_textures[L_LEVEL]->w, 0});

    // Draw move counter
    src_rect = (SDL_FRect){0, TILE_SIZE*((SDL_GetTicks()/500)%(Level_textures[L_MOVES]->h/TILE_SIZE)), Level_textures[L_MOVES]->w, TILE_SIZE};
    dst_rect.x = 0;
    dst_rect.y = TILE_SIZE;
    dst_rect.w = Level_textures[L_MOVES]->w;
    SDL_RenderTexture(renderer, Level_textures[L_MOVES], &src_rect, &dst_rect);
    Level_drawNumber(renderer, level->state->moves, 2, (Vec2){Level_textures[L_MOVES]->w, TILE_SIZE});
}

void Level_newState(Level* level) {
    LevelState* newState = SDL_malloc(sizeof(LevelState));
    SDL_memcpy(newState, level->state, sizeof(LevelState));
    newState->lastState = level->state;
    level->state = newState;
    level->state->moves++;
}

int Level_isCube(Level* level, Vec2 pos) {
    for (int i = 0; i < level->numCubes; i++) {
        if (level->state->cubes[i].x == pos.x && level->state->cubes[i].y == pos.y) {
            return i;
        }
    }

    return -1;
}

bool Level_isPassable(Level* level, Vec2 pos) {
    if (Level_isCube(level, pos) > -1) return false;

    switch (level->tiles[pos.y][pos.x]) {
        case L_PWALL:
        case L_NPWALL:
            return false;
        default:
            return true;
    }
}

bool Level_canMove(Level* level, Vec2 pos, Vec2 dir) {
    Vec2 newPos = Vec2_add(&pos, &dir);
    if (Level_isCube(level, newPos) > -1) return Level_canMove(level, newPos, dir);

    switch (level->tiles[newPos.y][newPos.x]) {
        case L_PWALL:
        case L_NPWALL:
            return false;
        default:
            return true;
    }
}

void Level_moveEntity(Level* level, Vec2* ent, Vec2 dir) {
    Vec2 targetPos = Vec2_add(ent, &dir);
    int nextCubeId = Level_isCube(level, targetPos);
    if (nextCubeId > -1) Level_moveEntity(level, &level->state->cubes[nextCubeId], dir);
    *ent = targetPos;
}

void Level_move(Level* level, Vec2 dir) {
    if (Level_canMove(level, level->state->playerLocation, dir)) {
        Level_newState(level);
        Level_moveEntity(level, &level->state->playerLocation, dir);
    }
}

void Level_shoot(Level* level, Vec2 dir) {
    Vec2 pos = level->state->playerLocation;
    while (true) {
        if (Level_isPassable(level, pos)) {
            pos = Vec2_add(&pos, &dir);
        } else {
            switch (level->tiles[pos.y][pos.x]) {
                case L_PWALL:
                    Level_newState(level);
                    Portal* newPortal = (level->state->lastShotBlue) ? &level->state->orangePortal : &level->state->bluePortal;
                    level->state->lastShotBlue = !level->state->lastShotBlue;
                    newPortal->exists = true;
                    newPortal->pos = pos;
                    if (Vec2_equal(&dir, &V_UP)) newPortal->r = 2;
                    else if (Vec2_equal(&dir, &V_DOWN)) newPortal->r = 0;
                    else if (Vec2_equal(&dir, &V_LEFT)) newPortal->r = 1;
                    else if (Vec2_equal(&dir, &V_RIGHT)) newPortal->r = 3;
            }
            break;
        }
    }
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
            if (level->tiles[i][j] == L_BUTTON && Level_isCube(level, (Vec2){j, i}) == -1) return false;
        }
    }

    return true;
}
