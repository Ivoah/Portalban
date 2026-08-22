#include "Vec2.h"
#include "SDL3/SDL.h"

Vec2 Vec2_add(const Vec2* v1, const Vec2* v2) {
    return (Vec2){v1->x + v2->x, v1->y + v2->y};
}

bool Vec2_equal(const Vec2* v1, const Vec2* v2) {
    return v1->x == v2->x && v1->y == v2->y;
}

int Vec2_toR(const Vec2* dir) {
    if (Vec2_equal(dir, &V_UP)) return 2;
    else if (Vec2_equal(dir, &V_DOWN)) return 0;
    else if (Vec2_equal(dir, &V_LEFT)) return 1;
    else if (Vec2_equal(dir, &V_RIGHT)) return 3;
    return -1;
}

Vec2 Vec2_fromR(int r) {
    return (Vec2[]){V_DOWN, V_LEFT, V_UP, V_RIGHT}[r%4];
}

Vec2 Vec2_180(const Vec2* v) {
    return (Vec2){-v->x, -v->y};
}
