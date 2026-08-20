#include "Vec2.h"
#include "SDL3/SDL.h"

const Vec2 V_UP = {0, -1};
const Vec2 V_DOWN = {0, 1};
const Vec2 V_LEFT = {-1, 0};
const Vec2 V_RIGHT = {1, 0};

Vec2 Vec2_add(Vec2 v1, Vec2 v2) {
    return (Vec2){v1.x + v2.x, v1.y + v2.y};
}

bool Vec2_equal(Vec2 v1, Vec2 v2) {
    return v1.x == v2.x && v1.y == v2.y;
}
