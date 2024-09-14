#pragma once
#include "raylib.h"

#define Round(x) ((int)(x))
#define Sign(x) (((x) < 0) ? -1 : 1)

#define SLOWDOWN_GRAVITY 0.5
#define GRAVITY 9.81
#define TITLEFONTSIZE 90
#define SUBTITLEFONTSIZE 45
#define SUBSUBTITLEFONTSIZE 20
#define MAX_SOLID_COUNT 100
typedef struct AABB
{
    Vector2 min;
    Vector2 max;
} AABB;

typedef enum Tile
{
   empty,
   stone,
   breakable,
   spike,
   tilecount,
} Tile;


typedef enum Battery
{
    bigjump,
    doublejump,
    rapidfire,
    heavyfire,
    quickie,
    tanky,
    // overdrive, // damage up , move speed up, health down, IMPLEMENT LATER
    batterycount
} Battery;

typedef enum Bullet
{
    none,
    lightbullet,
    heavybullet,
    normalbullet,
    bulletcount
} Bullet;

#define TMSIZE 16

/* Slots */
/* leg slot 1*/
/* gun slot 2*/
/* chest slot 3*/
/* helmet slot 4*/
/* core slot (for final battery) 5*/
#define BATTERYSLOTS 5
typedef struct Player
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
    float doubleJumpVel;

    bool grounded;
    bool pressedJump;


    double timeLastOnGround;
    double timeLastJumpPressed;

    double timeLastJumped;
    int jumpCount;
    int maxJumps;
    bool canDoubleJump;

    double lastShot;
    double shootDelay;

    Bullet bullettype;

    /* Enemy stuff */
    int score;

    bool standingOnSpike;
    Vector2 damageimpulse;
    #define SPIKEHITCOOLDOWN 0.5
    double lastspikehit;

    #define DAMAGETINTDURATION 0.5
    double lasthittaken;
    Color tint;

    int health;
    int maxHealth; // so we can modify it with effects
    #define NORMALMAXHEALTH 100
    // dont need a bool alive; cuz we can just do if health < 1


    // battery shit
    // bool triedToPickupBattery;
    // Battery batteryToPickup;
    double lastPickedupBattery;

    /* Rendering stuff */
    Color col;
    Texture2D right_tex;
    Texture2D left_tex;
} Player;

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
