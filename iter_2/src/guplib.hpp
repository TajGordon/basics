#pragma once
#include "raylib.h"

#define Round(x) ((int)(x))
#define Sign(x) (((x) < 0) ? -1 : 1)

typedef struct AABB
{
    Vector2 min;
    Vector2 max;
} AABB;

typedef struct Actor
{
    Vector2 pos;
    Vector2 size;
    AABB aabb;
    float xRemainder;
    float yRemainder;

    Vector2 vel;
    Vector2 dir;
    Vector2 lastDir;
    float speed;

    float jumpVel;

    bool grounded;
    bool pressedJump;

    double timeLastOnGround;
    double timeLastJumpPressed;

    double timeLastJumped;
    int jumpCount;
    int maxJumps;

    double lastLightShot;
    double lastHeavyShot;

    float lightShootDelay;
    float heavyShootDelay;

    /* Rendering stuff */
    Color col;
    Texture2D tex;
} Actor;

typedef struct Solid
{
    Vector2 pos;
    Vector2 size;
    AABB aabb;
    float xRemainder;
    float yRemainder;
    bool collideable;

    Color color;
} Solid;
