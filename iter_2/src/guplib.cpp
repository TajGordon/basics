#include "raylib.h"
#include "guplib.hpp"
#include <cstdio>
#include <malloc/_malloc.h>
/**************/
/* MATH STUFF */
/**************/
bool AABBsColliding(AABB a, AABB b)
{
    if (a.max.x <= b.min.x || a.min.x >= b.max.x) return false;
    if (a.max.y <= b.min.y || a.min.y >= b.max.y) return false;
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

Color operator*(Color c, float f)
{
    return (Color){(unsigned char)(c.r * f), (unsigned char)(c.g * f), (unsigned char)(c.b * f), (unsigned char)(c.a * f)};
}

/**********************/
/* Constants and shit */
/**********************/
const double physicsDTs = 1. / 60;

/***********/
/* Tilemap */
/***********/
const int TMSIZE = 16;

typedef enum Tile
{
    empty,
    stone,
    breakable,
    tilecount,
} Tile;

Color tileColor[tilecount] =
{
    BLANK,
    BLACK,
    GRAY,
};

#define windowWidth 1280
#define windowHeight 720

const int WORLD_SIZE_X = 200;
const int WORLD_SIZE_Y = 100;

Tile tilemap[WORLD_SIZE_X][WORLD_SIZE_Y] = {};

void LoadTilemap(const char* path)
{
    FILE* fp = fopen(path, "r+");
    for (int y = 0; y < WORLD_SIZE_Y; y++)
    {
        for (int x = 0; x < WORLD_SIZE_X; x++)
        {
            Tile t;
            char c;
            fscanf(fp, " %c", &c);
            switch (c)
            {
                case '.':
                    t = empty;
                    break;
                case 'S':
                    t = stone;
                    break;
                case 'B':
                    t = breakable;
                    break;
            }
            tilemap[x][y] = t;
        }
    }
    fclose(fp);
}

void DrawTilemap(Actor* player)
{
    int minx = (player->pos.x - windowWidth / 2) / TMSIZE;
    int maxx = (player->pos.x + windowWidth / 2) / TMSIZE;
    int miny = (player->pos.y - windowHeight / 2) / TMSIZE;
    int maxy = (player->pos.y + windowHeight / 2) / TMSIZE;

    for (int y = miny; y <= maxy; y++)
    {
        for (int x = minx; x <= maxx; x++)
        {
            if (x >= 0 && x < WORLD_SIZE_X && y >= 0 && y < WORLD_SIZE_Y)
            {
                DrawRectangle(x * TMSIZE, y * TMSIZE, TMSIZE, TMSIZE, tileColor[tilemap[x][y]]);
            }
        }
    }
}

/*****************/
/* Bullets stuff */
/*****************/
typedef enum Bullet
{
    none,
    light,
    heavy,
    bulletcount
} Bullet;

float bulletRadius[bulletcount] =
{
    0,
    2,
    6,
};

float bulletDamage[bulletcount] =
{
    0,
    2,
    10,
};

float bulletSpeed[bulletcount] =
{
    0,
    500,
    350,
};

Color bulletColor[bulletcount] =
{
    BLANK,
    SKYBLUE,
    MAROON,
};

#define MAX_BULLETS 200
Bullet bulletsTs[MAX_BULLETS] = {none};
Vector2 bulletsVs[MAX_BULLETS] = {0};
Vector2 bulletsPs[MAX_BULLETS] = {0};
int bulletCount = 0;

void SpawnBullet(Bullet bulletType, float posx, float posy, float velx, float vely)
{
    /* Temporary! */
    if (bulletCount == 199) bulletCount = 0;


    bulletsTs[bulletCount] = bulletType;
    bulletsPs[bulletCount] = {posx, posy};
    bulletsVs[bulletCount] = {velx, vely};
    bulletCount++;
}

void SpawnBulletV(Bullet bulletType, Vector2 pos, Vector2 vel)
{
    SpawnBullet(bulletType, pos.x, pos.y, vel.x, vel.y);
}

void KillBullet(int idx)
{
    bulletsTs[idx] = bulletsTs[bulletCount - 1];
    bulletsPs[idx] = bulletsPs[bulletCount - 1];
    bulletsVs[idx] = bulletsVs[bulletCount - 1];
    bulletCount--;
}

void DrawBullets()
{
    for (int i = 0; i < bulletCount; i++)
    {
        //                                                     (0, 0, 150, 255) * 0.8
        #define BULLET_CIRCLE_FACTOR 0.8
        DrawCircleV(bulletsPs[i], bulletRadius[bulletsTs[i]], bulletColor[bulletsTs[i]] * BULLET_CIRCLE_FACTOR);
        DrawCircleV(bulletsPs[i], bulletRadius[bulletsTs[i]] * BULLET_CIRCLE_FACTOR, bulletColor[bulletsTs[i]]);
    }
}

void BulletPhysicsProcess(float dt)
{
    // Move bullets
    for (int i = 0; i < bulletCount; i++)
    {
        bulletsPs[i] = bulletsPs[i] + (bulletsVs[i] * dt);
    }

    // Check bullets collision
    {

    }
}

/***************/
/* Actor Stuff */
/***************/


void DrawActor(Actor a)
{
    DrawTextureV(a.tex, a.pos - a.size/2, WHITE);
    DrawRectangleV(a.pos - (a.size / 2), a.size, a.col);
}

bool ActorCollidingAt(Actor* a, Vector2 pos, Solid* solids, int solidCount)
{
    AABB aabb = {.max = pos + (a->size/2), .min = pos - (a->size/2)};
    // Check if colliding with the tilemap
    // we do this first because it is much more likely to be colliding with the tilemap than a solid object, if we even have any solid objects
    {
        int minx = (aabb.min.x / TMSIZE) - 1;
        int miny = (aabb.min.y / TMSIZE) - 1;
        int maxx = (aabb.max.x / TMSIZE) + 1;
        int maxy = (aabb.max.y / TMSIZE) + 1;

        for (int y = miny; y <= maxy; y++)
        {
            for (int x = minx; x <= maxx; x++)
            {
                if (x >= 0 && x < WORLD_SIZE_X && y >= 0 && y < WORLD_SIZE_Y)
                {
                    if (tilemap[x][y] != empty)
                    {
                        AABB taabb = {.min = (Vector2){(float)x * TMSIZE, (float)y * TMSIZE}, .max = (Vector2){(float)x * TMSIZE + TMSIZE, (float)y * TMSIZE + TMSIZE}};
                        if (AABBsColliding(aabb, taabb))
                        {
                            return true;
                        }
                    }
                }
            }
        }
        // int minx = (a->aabb.min.x / TMSIZE) - 1;
        // int miny = (a->aabb.min.y / TMSIZE) - 1;

        // printf("\n");
        // for (int y = miny; y <= a->aabb.max.y / TMSIZE; y++)
        // {
        //     for (int x = minx; x <= a->aabb.max.x / TMSIZE ; x++)
        //     {
        //         printf("Tilemap[%d][%d] = %d | ", x, y, tilemap[x][y]);
        //         if (tilemap[x][y] != empty)
        //         {
        //             AABB taabb = {.min = (Vector2){(float)x * TMSIZE, (float)y * TMSIZE}, .max = (Vector2){(float)x * TMSIZE + TMSIZE, (float)y * TMSIZE + TMSIZE}};
        //             if (AABBsColliding(aabb, taabb))
        //             {
        //                 return true;
        //             }
        //         }
        //     }
        // }
    }
    // Check if colliding with any solid in the scene, currently loosp over every solid
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
                // if (moving up) stop moving up;
                if (sign < 0) a->vel.y = 0;
                break;
            }
        }
    }
}

#define COYOTE_S 0.2
#define JUMP_B_S 0.2

void TryJump(Actor* p, double time)
{
    if (p->pressedJump && p->jumpCount < p->maxJumps
    && (time - p->timeLastJumpPressed <= JUMP_B_S)
    && (time - p->timeLastOnGround <= COYOTE_S))
    {
        p->vel.y = p->jumpVel;
        p->pressedJump = false;
        p->jumpCount++;
        p->timeLastJumped = time;
    }
}

void PlayerInput(Actor* p, double time)
{
    TryJump(p, time);

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && time - p->lastLightShot > p->lightShootDelay)
    {
        SpawnBulletV(light, p->pos, {p->lastDir * bulletSpeed[light], 0});
        p->lastLightShot = time;
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && time - p->lastHeavyShot > p->heavyShootDelay)
    {
        SpawnBulletV(heavy, p->pos, {p->lastDir * bulletSpeed[heavy], 0});
        p->lastHeavyShot = time;
    }

    int dir = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
    if (dir != 0) p->lastDir = dir;
    p->vel.x = dir * p->speed;
}

void PlayerPhysicsProcess(Actor* p, Solid* solids, int solidCount, double time)
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
            p->timeLastOnGround = time;
            p->jumpCount = 0;
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
            gmult = 1.5;
        }
        p->vel.y += GRAVITY * gmult * physicsDTs;
    }

    PlayerInput(p, time);

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

void CameraFollowActor(Camera2D* camera, Actor* a)
{
    camera->target = a->pos;
}
