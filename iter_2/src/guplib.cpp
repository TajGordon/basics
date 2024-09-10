#pragma once
#include "raylib.h"

/**************/
/* MATH STUFF */
/**************/
#define Round(x) (int)(x)
#define Sign(x) ((x) < 0) ? -1 : 1
#define abs(x) ((x) < 0) ? -(x) : (x)

typedef struct AABB
{
    Vector2 min;
    Vector2 max;
} AABB;

bool AABBsColliding(AABB a, AABB b)
{
    if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
    return true;
}

Vector2 operator+(Vector2 v1, Vector2 v2)
{
    return {v1.x + v2.x, v1.y + v2.y};
}
Vector2 operator-(Vector2 v1, Vector2 v2)
{
    return {v1.x - v2.x, v1.y - v2.y};
}
Vector2 operator*(Vector2 v, float f)
{
    return {v.x * f, v.y * f};
}
Vector2 operator/(Vector2 v, float f)
{
    return {v.x / f, v.y / f};
}

/**********************/
/* Constants and shit */
/**********************/
const double physicsDTs = 1. / 60;

/***************/
/* Actor Stuff */
/***************/

typedef struct Actor
{
    Vector2 pos;
    Vector2 size;
    AABB aabb;
    float xRemainder;
    float yRemainder;

    Vector2 vel;

    bool grounded;
    bool pressedJump;

    /* Rendering stuff */
    Color col;
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

void DrawActor(Actor a)
{
    DrawRectangleV(a.pos - (a.size / 2), a.size, a.col);
}

bool ActorCollidingAt(Actor* a, Vector2 pos, Solid* solids, int solidCount)
{
    AABB aabb = {.max = pos + (a->size/2), .min = pos - (a->size/2)};
    for (int i = 0; i < solidCount; i++)
    {
        Solid* s = &solids[i];
        if (s->collideable && AABBsColliding(aabb, s->aabb))
        {
            return true;
        }
    }
    return false;
}

void MoveX(Actor* a, float amount, Solid* solids, int solidCount)
{
    a->xRemainder += amount;
    int move = Round(a->xRemainder);
    if (move != 0)
    {
        a->xRemainder -= move;
        int sign = Sign(move);
        while (move != 0)
        {
            if (!ActorCollidingAt(a, {a->pos.x + sign, a->pos.y}, solids, solidCount))
            {
                a->pos.x += sign;
                move -= sign;
            }
            else
            {
                // colliding
                break;
            }
        }
    }
}

void MoveY(Actor* a, float amount, Solid* solids, int solidCount)
{
    a->yRemainder += amount;
    int move = Round(a->yRemainder);
    if (move != 0)
    {
        a->yRemainder -= move;
        int sign = Sign(move);
        while (move != 0)
        {
            if (!ActorCollidingAt(a, {a->pos.x, a->pos.y + sign}, solids, solidCount))
            {
                a->pos.y += sign;
                move -= sign;
            }
            else
            {
                // colliding
                // if (sign > 0) a->grounded = true;
                break;
            }
        }
    }
}

void PlayerInput(Actor* p)
{
    if (p->pressedJump) { p->vel.y = -5; p->pressedJump = false; }

    int dir = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
    p->vel.x = dir * 10;
}

void PlayerPhysicsProcess(Actor* p, Solid* solids, int solidCount)
{
    // Still on Ground check
    {
        // if we move the player down EPSILON, would they be colliding? if so, they're on ground
        // if we move down a bit, and aren't colliding, then we aren't grounded
        // if we move down a bit, are are oclliding, we're on the ground
        // instead of epsilon we need to do 1, since we have pixel movement
        if (ActorCollidingAt(p, {p->pos.x, p->pos.y + 1}, solids, solidCount))
        {
            p->grounded = true;
            // i don't know if this needs to be here, but just in case, set velocity to 0 if we are on ground
            // only if we're going down tho, if we're going up, it doesn't matter if we would hit the ground, we moving up
            if (p->vel.y > 0) p->vel.y = 0;
        }
        else
        {
            p->grounded = false;
        }
    }
    // Gravity
    if (!p->grounded)
    {
        #define SLOWDOWN_GRAVITY 0.5
        #define GRAVITY 9.81
        float gmult = 1.;
        if (abs(p->vel.y) < SLOWDOWN_GRAVITY)
        {
            gmult = 0.8;
        }
        else if (p->vel.y > 0)
        {
            gmult = 2.;
        }
        p->vel.y += GRAVITY * gmult * physicsDTs;
    }

    MoveX(p, p->vel.x, solids, solidCount);
    MoveY(p, p->vel.y, solids, solidCount);
}

Actor** GetRidingActors(Solid* s)
{
    return 0;
}

void Move(Solid* s, float x, float y)
{
    s->xRemainder += x;
    s->yRemainder += y;
    int moveX = Round(s->xRemainder);
    int moveY = Round(s->yRemainder);
    if (moveX != 0 || moveY != 0)
    {
        s->collideable = false;

        int riderCount = 0;
        Actor** riders = GetRidingActors(s);

        if (moveX != 0)
        {
            s->xRemainder -= moveX;
            s->pos.x += moveX;
            if (moveX > 0)
            { // not handling collision logic rn, we just move
            }
            else
            {
            }
        }
        if (moveY != 0)
        {
            s->yRemainder -= moveY;
            s->pos.y += moveY;
            if (moveY > 0) // moving down
            {
            }
            else
            {
            }
        }
    }
}

void DrawSolids(Solid* solids, int solidCount)
{
    for (int i = 0; i < solidCount; i++)
    {
        Solid* s = &solids[i];
        DrawRectangleV(s->pos - (s->size/2), s->size, s->color);
    }
}

Solid MakeSolid(float x, float y, float width, float height, Color color, bool collideable)
{
    Solid s = {0};
    s.pos = {x, y};
    s.size = {width, height};
    s.aabb.max = s.pos + (s.size/2);
    s.aabb.min = s.pos - (s.size/2);
    s.color = color;
    s.collideable = collideable;
    return s;
}

void PhysicsUpdate(Actor* p, Solid* solids, int solidCount)
{
    /* Player */
    PlayerInput(p);
    PlayerPhysicsProcess(p, solids, solidCount);
}

void CameraFollowActor(Camera2D* camera, Actor* a)
{
    camera->target = a->pos;
}
