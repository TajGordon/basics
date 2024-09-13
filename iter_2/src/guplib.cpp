#include "raylib.h"
#include "raymath.h"
#include "guplib.hpp"
#include <cstdio>
#include <malloc/_malloc.h>
#include "math.h"
/**************/
/* MATH STUFF */
/**************/
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
Vector2 operator+(Vector2 v, float f)
{
    return {v.x + f, v.y + f};
}

Color operator*(Color c, float f)
{
    return (Color){(unsigned char)(c.r * f), (unsigned char)(c.g * f), (unsigned char)(c.b * f), (unsigned char)(c.a * f)};
}

Vector2 vabs(Vector2 v)
{
    return {fabs(v.x), fabs(v.y)};
}

Vector2 vmax(Vector2 v1, Vector2 v2)
{
    return {fmax(v1.x, v2.x), fmax(v1.y, v2.y)};
}

bool AABBsColliding(AABB a, AABB b)
{
    if (a.max.x <= b.min.x || a.min.x >= b.max.x) return false;
    if (a.max.y <= b.min.y || a.min.y >= b.max.y) return false;
    return true;
}
/**********************/
/* Constants and shit */ // did not get fucking used at all lmao
/**********************/
const double physicsDTs = 1. / 60;

/*****************/
/* Battery Stuff */
/*****************/
// jump velocity needs to be negative retard
#define BIGJUMPVEL -4
#define NORMALJUMPVEL -3
#define DOUBLEJUMPVEL -2

#define NORMALSPEED 2
#define QUICKIESPEED 4

float bulletDelays[bulletcount] =
{
    0,
    0.1, // light
    0.6, // heavy
    0.3, // normal
};

Texture batteryTextures[batterycount] =
{

};

Vector2 batteryPositions[batterycount] = {};
bool batteryInUse[batterycount] = {};
bool batteryCanBePickedUp[batterycount] = {};

void DropBattery(Player* p, Battery battery)
{
    if (!batteryInUse[battery]) return;
    switch (battery)
    {
        case bigjump:
        {
            batteryInUse[bigjump] = false;
            batteryPositions[bigjump] = p->pos;
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case doublejump:
        {
            batteryInUse[doublejump] = false;
            batteryPositions[bigjump] = p->pos;
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case rapidfire:
        {
            batteryInUse[rapidfire] = false;
            batteryPositions[rapidfire] = p->pos;
            p->shootDelay = bulletDelays[normalbullet];
            p->bullettype = normalbullet;
            break;
        }
        case heavyfire:
        {
            batteryInUse[heavyfire] = false;
            batteryPositions[heavyfire] = p->pos;
            p->shootDelay = bulletDelays[normalbullet];
            p->bullettype = normalbullet;
            break;
        }
        case quickie:
        {
            batteryInUse[quickie] = false;
            batteryPositions[quickie] = p->pos;
            p->speed = NORMALSPEED;
            break;
        }
        case tanky:
        {
            batteryInUse[tanky] = false;
            batteryPositions[quickie] = p->pos;
            p->maxHealth = NORMALMAXHEALTH;
            break;
        }
    }
}

void PickupBattery(Player* p, Battery battery)
{
    switch (battery)
    {
        case bigjump:
        {
            batteryInUse[bigjump] = true;
            p->jumpVel = BIGJUMPVEL;
            p->maxJumps = 1;
            // removal of conflicting one
            if (batteryInUse[doublejump])
            {
                batteryInUse[doublejump] = false;
                batteryPositions[doublejump] = p->pos;
            }
            break;
        }
        case doublejump:
        {
            batteryInUse[doublejump] = true;
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 2;
            p->canDoubleJump = true;
            // removal of conflicting one
            if (batteryInUse[bigjump])
            {
                batteryInUse[bigjump] = false;
                batteryPositions[bigjump] = p->pos;
            }
            break;
        }
        case rapidfire:
        {
            batteryInUse[rapidfire] = true;
            p->shootDelay = bulletDelays[lightbullet];
            p->bullettype = lightbullet;
            // removal of conflicting one
            if (batteryInUse[heavyfire])
            {
                batteryInUse[heavyfire] = false;
                batteryPositions[heavyfire] = p->pos;
            }
            break;
        }
        case heavyfire:
        {
            batteryInUse[heavyfire] = true;
            p->shootDelay = bulletDelays[heavybullet];
            p->bullettype = heavybullet;
            // removal of conflicting one
            if (batteryInUse[rapidfire])
            {
                batteryInUse[rapidfire] = false;
                batteryPositions[rapidfire] = p->pos;
            }
            break;
        }
        case quickie:
        {
            batteryInUse[quickie] = true;
            p->speed = QUICKIESPEED;
            break;
        }
        case tanky:
        {
            batteryInUse[tanky] = true;
            #define TANKYHEALTHFACTOR 1.2
            // tankyhealthfactor = 1 + bonus max hp %
            // set the hp to be in respect to the starting max health
            // lets not add any other bonuses for now
            p->maxHealth = NORMALMAXHEALTH * TANKYHEALTHFACTOR;
            p->health = p->maxHealth;
            break;
        }
    }
}

bool TileOnScreen(Vector2 tilepos, Camera2D camera)
{
    Vector2 screenMin = GetScreenToWorld2D((Vector2){0, 0}, camera);
    Vector2 screenMax = GetScreenToWorld2D((Vector2){(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

    Vector2 tileMin = tilepos * TMSIZE;
    Vector2 tileMax = tileMin + TMSIZE;

    return !(tileMax.x < screenMin.x || tileMin.x > screenMax.x || tileMax.y < screenMin.y || tileMin.y > screenMax.y);
}

void DrawBatteries(Player* p, Camera2D camera)
{
    for (int i = bigjump; i < batterycount; i++)
    {
        if (!batteryInUse[i])
        {
            if (TileOnScreen(batteryPositions[i] / TMSIZE, camera))
            {
                DrawTextureV(batteryTextures[i], batteryPositions[i], WHITE);
                DrawRectangleV(batteryPositions[i], {5.f, 8.f}, LIGHTGRAY);
            }
            #define INTERACTION_DISTANCE 20
            if (Vector2DistanceSqr(batteryPositions[i], p->pos) < (INTERACTION_DISTANCE * INTERACTION_DISTANCE))
            {
                batteryCanBePickedUp[i] = true;
                const char* batteryname;
                switch (i)
                {
                    case bigjump:
                    {
                        batteryname = "bigjump";
                        break;
                    }
                    case doublejump:
                    {
                        batteryname = "doublejump";
                        break;
                    }
                    case rapidfire:
                    {
                        batteryname = "rapidfire";
                        break;
                    }
                    case heavyfire:
                    {
                        batteryname = "heavyfire";
                        break;
                    }
                    case quickie:
                    {
                        batteryname = "quickie";
                        break;
                    }
                    case tanky:
                    {
                        batteryname = "tanky";
                        break;
                    }
                }
                const char* string = TextFormat("Press 'E' to pickup %s battery", batteryname);

                int stringsize = MeasureText(string, 10);
                DrawText(string, batteryPositions[i].x - stringsize, batteryPositions[i].y - 20, 10, WHITE);
            }
            else // if the battery is too far
            {
                batteryCanBePickedUp[i] = false;
            }
        }
    }
}


/***********/
/* Tilemap */
/***********/
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
    DARKGRAY,
    GRAY,
};

#define windowWidth 1280
#define windowHeight 720

const int WORLD_SIZE_X = 200;
const int WORLD_SIZE_Y = 100;

Tile tilemap[WORLD_SIZE_X][WORLD_SIZE_Y] = {};

Vector2 player_spawnpoint = {};

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
                case 'S':
                    player_spawnpoint = {(float)x * TMSIZE, (float)y * TMSIZE};
                case '.':
                    t = empty;
                    break;
                case '#':
                    t = stone;
                    break;
                case 'B':
                    t = breakable;
                    break;
                case '0': // bigjump
                    batteryPositions[0] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                case '1': // doublejump
                    batteryPositions[1] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                case '2': // rapidfire
                    batteryPositions[2] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                case '3': // heavyfire
                    batteryPositions[3] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
            }
            tilemap[x][y] = t;
        }
    }
    fclose(fp);
}

void DrawTilemap(Player* player)
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

void BreakTile(int x, int y)
{
    tilemap[x][y] = empty;
}

/*****************/
/* Bullets stuff */
/*****************/
 float bulletRadius[bulletcount] =
{
    0,
    2,
    6,
    3,
};

float bulletDamage[bulletcount] =
{
    0,
    2,
    10,
    3,
};

void SetBulletDamages()
{
    // float normalbulletdps = 3.5; idfk man
    float lightbulletdps = 10;
    float heavybulletdps = 10;
    // bulletDamage[normalbullet] = normalbulletdps * bulletDelays[normalbullet];
    bulletDamage[normalbullet] = 2; // fuck the other shit
    bulletDamage[lightbullet] = lightbulletdps * bulletDelays[lightbullet];
    bulletDamage[heavybullet] = heavybulletdps * bulletDelays[heavybullet];
    printf("normalbullet damage = %f\n", bulletDamage[normalbullet]);
    printf("lightbullet damage = %f\n", bulletDamage[lightbullet]);
    printf("heavybullet damage = %f\n", bulletDamage[heavybullet]);
}

float bulletSpeed[bulletcount] =
{
    0,
    500,
    200,
    350,
};

Color bulletColor[bulletcount] =
{
    BLANK,
    SKYBLUE,
    MAROON,
    DARKBLUE,
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

void BulletPhysicsProcess()
{
    // Move bullets
    for (int i = 0; i < bulletCount; i++)
    {
        bulletsPs[i] = bulletsPs[i] + (bulletsVs[i] * physicsDTs);
    }

    // Check bullets collision
    {
        for (int i = 0; i < bulletCount; i++)
        {
            int minx = (bulletsPs[i].x / TMSIZE) - 1;
            int miny = (bulletsPs[i].y / TMSIZE) - 1;
            int maxx = (bulletsPs[i].x / TMSIZE) + 1;
            int maxy = (bulletsPs[i].y / TMSIZE) + 1;

            for (int y = miny; y <= maxy; y++)
            {
                for (int x = minx; x <= maxx; x++)
                {
                    if (x >= 0 && x < WORLD_SIZE_X && y >= 0 && y < WORLD_SIZE_Y)
                    {
                        if (tilemap[x][y] != empty)
                        {
                            Rectangle trect = (Rectangle){(float)x * TMSIZE, (float)y * TMSIZE, TMSIZE, TMSIZE};
                            if (CheckCollisionCircleRec(bulletsPs[i], bulletRadius[bulletsTs[i]], trect))
                            {
                                // colliding
                                bulletsVs[i] = {};
                                switch (tilemap[x][y])
                                {
                                    case breakable:
                                        if (bulletsTs[i] == heavybullet) BreakTile(x, y);
                                    case stone:
                                        KillBullet(i);
                                        break;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

/***************/
/* Player Stuff */
/***************/


void DrawPlayer(Player a)
{
    DrawTextureV(a.tex, a.pos - a.size/2, WHITE);
    DrawRectangleV(a.pos - (a.size / 2), a.size, a.col);
}

bool PlayerCollidingAt(Player* a, Vector2 pos, Solid* solids, int solidCount)
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

void MoveX(Player* a, float amount, Solid* solids, int solidCount)
{
    a->xRemainder += amount;
    int move = Round(a->xRemainder);
    if (move != 0)
    {
        a->xRemainder -= move;
        int sign = Sign(move);
        while (move != 0)
        {
            if (!PlayerCollidingAt(a, {a->pos.x + sign, a->pos.y}, solids, solidCount))
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

void MoveY(Player* a, float amount, Solid* solids, int solidCount)
{
    a->yRemainder += amount;
    int move = Round(a->yRemainder);
    if (move != 0)
    {
        a->yRemainder -= move;
        int sign = Sign(move);
        while (move != 0)
        {
            if (!PlayerCollidingAt(a, {a->pos.x, a->pos.y + sign}, solids, solidCount))
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

void TryJump(Player* p, double time)
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
    // Double jump check
    else if (p->pressedJump && p->jumpCount < p->maxJumps
    && (time - p->timeLastJumpPressed <= JUMP_B_S)
    && !(time - p->timeLastOnGround <= COYOTE_S) // not-ing this makes it a check if we aren't on ground and can double jump
    && p->canDoubleJump)
    {
        p->vel.y = p->doubleJumpVel;
        p->pressedJump = false;
        p->jumpCount++;
        p->timeLastJumped = time;
    }
}

void PlayerInput(Player* p, double time)
{
    TryJump(p, time);

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && time - p->lastShot > p->shootDelay)
    {
        Vector2 dir = (p->dir.x != 0 || p->dir.y != 0) ? p->dir : p->lastDir;
        dir = Vector2Normalize(dir);
        SpawnBulletV(p->bullettype, p->pos, dir * bulletSpeed[p->bullettype]);
        p->lastShot = time;
    }

    Vector2 dir = {};
    dir.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
    dir.y = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
    if (dir.x != 0) p->lastDir = {dir.x, 0};
    p->dir = dir;
    p->vel.x = dir.x * p->speed;
}

void PlayerPhysicsProcess(Player* p, Solid* solids, int solidCount, double time)
{
    // Still on Ground check
    {
        // if we move the player down EPSILON, would they be colliding? if so, they're on ground
        // if we move down a bit, and aren't colliding, then we aren't grounded
        // if we move down a bit, are are oclliding, we're on the ground
        // instead of epsilon we need to do 1, since we have pixel movement
        if (PlayerCollidingAt(p, {p->pos.x, p->pos.y + 1}, solids, solidCount))
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

Player** GetRidingPlayers(Solid* s)
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
        Player** riders = GetRidingPlayers(s);

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

void CameraFollowPlayer(Camera2D* camera, Player* a)
{
    camera->target = a->pos;
}
