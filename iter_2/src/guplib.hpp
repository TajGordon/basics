#pragma once
#include "/Users/tajgordon/raylib-5.0_webassembly/include/raylib.h"

#define Round(x) ((int)(x))
#define Sign(x) (((x) < 0) ? -1 : 1)

#define SLOWDOWN_GRAVITY 0.5
#define GRAVITY 9.81
#define TITLEFONTSIZE 90
#define SUBTITLEFONTSIZE 45
#define SUBSUBTITLEFONTSIZE 20
#define MAX_SOLID_COUNT 100
#define INTERACTION_DISTANCE 20
#define INSERTBATTERYFONTSIZE 10
#define BIGJUMPVEL -4.6
#define NORMALJUMPVEL -3.5
#define DOUBLEJUMPVEL -2.3
#define DOUBLEJUMPSIDEWAYSBONUSVELMULTIPLIER 1.5
#define NORMALSPEED 2
#define BATTERYDROPOFFSET 4
#define FOV_MAX_RADIUS 7
#define DEFAULT_FOV_RADIUS 3
#define FOV_RADIUS_WITH_BATTERY 4
#define RAY_COUNT 720
#define QUICKIESPEED 3

#define WORLD_SIZE_X 400
#define WORLD_SIZE_Y 200

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
   cloud,
   platform,
   sky,
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
    visionup,
    // overdrive, // damage up , move speed up, health down, IMPLEMENT LATER
    batterycount
} Battery;

typedef enum Bullet
{
    none,
    lightbullet,
    heavybullet,
    normalbullet,
    enemylightbullet,
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
    Vector2 persistentVel;

    double timeLastOnGround;
    double timeLastJumpPressed;

    double lastBulletHitTaken;
    Bullet lastBulletHitType;
    bool bulletHitNotAccountedFor;

    double timeLastJumped;
    int jumpCount;
    int maxJumps;
    bool canDoubleJump;

    float FOV;

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
    #define NORMALMAXHEALTH 250
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
