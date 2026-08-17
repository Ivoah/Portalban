#include "SDL3/SDL.h"
#include "Vec2.h"

#define TILE_SIZE 32

typedef struct {
    int x;
    int y;
    int r;
} Portal;

#define MAX_CUBES 10
typedef struct {
    int levelNum;
    int tiles[32][32];
    int width;
    int height;
    Vec2 playerLocation;
    int numCubes;
    Vec2 cubes[10];
    Portal orangePortal;
    Portal bluePortal;
} Level;

bool loadLevelTextures(SDL_Renderer*);
void unloadLevelTextures();
Level* loadLevel(int);
void drawLevel(SDL_Renderer*, Level*, Vec2);
// bool canMove(Level*, Vec2, Vec2);
void move(Level*, Vec2);
bool isWon(Level*);
