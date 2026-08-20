#include "SDL3/SDL.h"
#include "Vec2.h"

#define TILE_SIZE 32
#define MAX_CUBES 32

typedef struct {
    int x;
    int y;
    int r;
} Portal;

typedef struct LevelState {
    int moves;
    Vec2 playerLocation;
    Vec2 cubes[MAX_CUBES];
    Portal orangePortal;
    Portal bluePortal;
    struct LevelState* lastState;
} LevelState;

typedef struct {
    int levelNum;
    int tiles[32][32];
    int width;
    int height;
    int numCubes;
    LevelState* state;
} Level;

bool Level_loadTextures(SDL_Renderer*);
void Level_unloadTextures();
Level* Level_load(int);
void Level_free(Level*);
void Level_draw(SDL_Renderer*, Level*, Vec2, Uint64);
void Level_drawNumber(SDL_Renderer*, int, int, Vec2, Uint64);
void Level_move(Level*, Vec2);
void Level_shoot(Level*, Vec2);
void Level_undo(Level*);
bool Level_isWon(Level*);
