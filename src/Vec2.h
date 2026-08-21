#include <SDL3/SDL.h>

typedef struct {
    int x;
    int y;
} Vec2;

// const Vec2 V_UP;
// const Vec2 V_DOWN;
// const Vec2 V_LEFT;
// const Vec2 V_RIGHT;
#define V_UP (Vec2){0, -1}
#define V_DOWN (Vec2){0, 1}
#define V_LEFT (Vec2){-1, 0}
#define V_RIGHT (Vec2){1, 0}


Vec2 Vec2_add(const Vec2*, const Vec2*);
bool Vec2_equal(const Vec2*, const Vec2*);
int Vec2_toR(const Vec2*);
Vec2 Vec2_fromR(int);
Vec2 Vec2_180(const Vec2*);
