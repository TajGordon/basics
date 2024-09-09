#include "raylib.h"
#include <cmath>

/**************/
/* MATH STUFF */
/**************/
#define Round(x) (int)(x)
#define Sign(x) ((x) < 0) ? -1 : 1

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

float Vector2Length(Vector2 v)
{
    return sqrt(v.x*v.x + v.y*v.y);
}

Vector2 Vector2Normalize(Vector2 v)
{
    return v / Vector2Length(v);
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

    Vector2 dir;

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
                break;
            }
        }
    }
}

void PlayerInput(Actor* p)
{
    Vector2 dir = {(float)IsKeyDown(KEY_D) - IsKeyDown(KEY_A), (float)IsKeyDown(KEY_S) - IsKeyDown(KEY_W)};
    if (dir.x > 0 || dir.y > 0) dir = Vector2Normalize(dir);
    p->dir = dir * 10;
}

void PlayerPhysicsProcess(Actor* p, Solid* solids, int solidCount)
{
    MoveX(p, p->dir.x, solids, solidCount);
    MoveY(p, p->dir.y, solids, solidCount);
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

int main(void)
{
    const int windowWidth = 1280;
    const int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "A window");
    SetTargetFPS(120);

    #define MAX_SOLID_COUNT 100
    Solid* solids = (Solid*)malloc(sizeof(Solid) * MAX_SOLID_COUNT);
    int solidCount = 0;

    Actor p = {0};
    {
        p.pos = {400, 400};
        p.size = {40, 40};
        p.col = YELLOW;
        p.aabb.max = p.pos + (p.size/2);
        p.aabb.min = p.pos - (p.size/2);
    }

    Camera2D camera;
    {
        camera.zoom = 2;
        camera.rotation = 0;
        camera.offset = {0};
        camera.target = {0};
    }

    solids[solidCount++] = MakeSolid(640, 720, 1280, 200, GRAY, true);
    solids[solidCount++] = MakeSolid(640, 0, 1280, 200, GRAY, true);

    double physicsAccumulator = 0;
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        physicsAccumulator += dt;
        if (physicsAccumulator > physicsDTs)
        {
            physicsAccumulator -= physicsDTs;
            PhysicsUpdate(&p, solids, solidCount);
        }
        BeginMode2D(camera);
        BeginDrawing();
        {
            ClearBackground(BLUE);
            DrawSolids(solids, solidCount);
            DrawActor(p);
        }
        EndDrawing();
        EndMode2D();
    }
    CloseWindow();

    return 0;
}
