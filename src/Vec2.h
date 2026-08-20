#include <SDL3/SDL.h>

typedef struct {
    int x;
    int y;
} Vec2;

const Vec2 V_UP;
const Vec2 V_DOWN;
const Vec2 V_LEFT;
const Vec2 V_RIGHT;

Vec2 Vec2_add(const Vec2*, const Vec2*);
bool Vec2_equal(const Vec2*, const Vec2*);
