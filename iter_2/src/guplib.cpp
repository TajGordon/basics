#include "raylib.h"
#include "raymath.h"
#include "guplib.hpp"
#include <cstdio>
#include <cstring>
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

typedef enum Gamestate
{
    running,
    gamestatecount
} Gamestate;

Gamestate gamestate = running;

#define MAX_DISPLAY_MESSAGES 100
#define MAX_DISPLAYMESSAGE_SIZE 100
#define DISPLAYMESSAGEFONTSIZE 5
double displayMessagesTimers[MAX_DISPLAY_MESSAGES] = {};
Vector2 displayMessagesPositions[MAX_DISPLAY_MESSAGES] = {};
char displayMessages[MAX_DISPLAY_MESSAGES][MAX_DISPLAYMESSAGE_SIZE] = {};
int displayMessagesCount = 0;

void RenderDisplayMessages()
{
    for (int i = 0; i < displayMessagesCount; i++)
    {
        float offset = MeasureText(displayMessages[i], DISPLAYMESSAGEFONTSIZE) / 2.;
        DrawText(displayMessages[i], displayMessagesPositions[i].x - offset, displayMessagesPositions[i].y, DISPLAYMESSAGEFONTSIZE, WHITE);
    }
}

// goes in physics update because it does
void TickDisplayMessagesTimers()
{
    for (int i = 0; i < displayMessagesCount; i++)
    {
        displayMessagesTimers[i] -= physicsDTs;
        if (displayMessagesTimers[i] < 0.)
        { // Remove the message
            displayMessagesTimers[i] = displayMessagesTimers[displayMessagesCount];
            displayMessagesPositions[i] = displayMessagesPositions[displayMessagesCount];
            strcpy(displayMessages[i], displayMessages[displayMessagesCount]);
        }
    }
}

void AddDisplayMessage(Vector2 position, double durationSeconds, const char* message)
{
    if (displayMessagesCount < MAX_DISPLAY_MESSAGES)
    {
        displayMessagesTimers[displayMessagesCount] = durationSeconds;
        displayMessagesPositions[displayMessagesCount] = position;
        strncpy(displayMessages[displayMessagesCount], message, MAX_DISPLAYMESSAGE_SIZE - 1);
        displayMessages[displayMessagesCount][MAX_DISPLAYMESSAGE_SIZE - 1] = '\0'; // adding a null terminator to the string
        displayMessagesCount++;
    }
    else
    {
        printf("\033[1;31mError:\033[0m Display messages limit reached. Cannot add new message.\n");
    }
}

/****************************/
/* For putting in batteries */
/****************************/
bool showingMessage = false;
int showingMessageIndex = 0;


/*****************/
/* Battery Stuff */
/*****************/
// jump velocity needs to be negative retard
#define BIGJUMPVEL -4
#define NORMALJUMPVEL -3
#define DOUBLEJUMPVEL -2

#define NORMALSPEED 2
#define QUICKIESPEED 3

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
bool batteryInDoor[batterycount] = {};
bool batteryCanBePickedUp[batterycount] = {};

void DropBattery(Player* p, Battery battery)
{
    #define BATTERYDROPOFFSET 4
    if (!batteryInUse[battery]) return;
    switch (battery)
    {
        case bigjump:
        {
            batteryInUse[bigjump] = false;
            batteryPositions[bigjump] = (Vector2){p->pos.x, p->pos.y - Round(TMSIZE / 2)};
            printf("Dropped bigjump battery\n");
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case doublejump:
        {
            batteryInUse[doublejump] = false;
            batteryPositions[doublejump] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            printf("Dropped doublejump battery\n");
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case rapidfire:
        {
            batteryInUse[rapidfire] = false;
            batteryPositions[rapidfire] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->shootDelay = bulletDelays[normalbullet];
            p->bullettype = normalbullet;
            break;
        }
        case heavyfire:
        {
            batteryInUse[heavyfire] = false;
            batteryPositions[heavyfire] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->shootDelay = bulletDelays[normalbullet];
            p->bullettype = normalbullet;
            break;
        }
        case quickie:
        {
            batteryInUse[quickie] = false;
            batteryPositions[quickie] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->speed = NORMALSPEED;
            break;
        }
        case tanky:
        {
            batteryInUse[tanky] = false;
            batteryPositions[quickie] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
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
            if (batteryInUse[doublejump])
            {
                DropBattery(p, doublejump);
            }
            batteryInUse[bigjump] = true;
            p->jumpVel = BIGJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case doublejump:
        {
            if (batteryInUse[bigjump])
            {
                DropBattery(p, bigjump);
            }
            batteryInUse[doublejump] = true;
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 2;
            p->canDoubleJump = true;
            break;
        }
        case rapidfire:
        {
            if (batteryInUse[heavyfire])
            {
                DropBattery(p, heavyfire);
            }
            batteryInUse[rapidfire] = true;
            p->shootDelay = bulletDelays[lightbullet];
            p->bullettype = lightbullet;
            break;
        }
        case heavyfire:
        {
            if (batteryInUse[rapidfire])
            {
                DropBattery(p, rapidfire);
            }
            batteryInUse[heavyfire] = true;
            p->shootDelay = bulletDelays[heavybullet];
            p->bullettype = heavybullet;
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

void DrawBatteries(Player* p, Camera2D camera, double time)
{
    bool batteryPickedUp = false;
    for (int i = bigjump; i < batterycount; i++)
    {
        if (!batteryInUse[i] && !batteryInDoor[i])
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
                const char* string = TextFormat("Press 'E' to pickup %s battery\0", batteryname);

                int stringsize = MeasureText(string, 10);
                DrawText(string, batteryPositions[i].x - stringsize/2.f, batteryPositions[i].y - TMSIZE, 10, WHITE);

                // We are showing the prompt to pickup the battery
                if (IsKeyPressed(KEY_E) && !batteryPickedUp)
                {
                    PickupBattery(p, (Battery)i);
                    batteryPickedUp = true;
                }
            }
            else // if the battery is too far
            {
                batteryCanBePickedUp[i] = false;
            }
        }
    }
}

const int WORLD_SIZE_X = 200;
const int WORLD_SIZE_Y = 100;

Tile tilemap[WORLD_SIZE_X][WORLD_SIZE_Y] = {};
/*********/
/* Doors */
/*********/
#define MAXDOORS 100
Vector2 doorStartPos[MAXDOORS] = {};
Vector2 doorEndPos[MAXDOORS] = {};
bool doorOpen[MAXDOORS] = {};
Vector2 doorKeyPos[MAXDOORS] = {};
bool doorKeyholeHasBattery[MAXDOORS] = {};
Battery doorKeyholebattery[MAXDOORS] = {};
int doorCount;

Vector2 doorStartingLoadPositions[MAXDOORS] = {};
int doorloadcount = 0;
Vector2 doorStartingLoadKeyPositions[MAXDOORS] = {};
int doorkeycount = 0;

/* Door loading */
void LoadDoors()
{
    if (doorkeycount != doorloadcount)
    {
        printf("\033[1;33mWARN:\033[0m Door key count does not match door count. Check your map file.\n");
    }
    doorCount = doorloadcount;
    for (int i = 0; i < doorloadcount; i++)
    {
        doorStartPos[i] = doorStartingLoadPositions[i];
        int numSpaces = 0;
        while (tilemap[(int)doorStartPos[i].x][(int)doorStartPos[i].y - 1 - numSpaces] == empty)
        {
            numSpaces++;
        }
        doorEndPos[i] = {doorStartPos[i].x, doorStartPos[i].y - numSpaces};

        // get key pos
        Vector2 totheleft = doorStartPos[i] + (Vector2){-1, 0};
        Vector2 totheright = doorStartPos[i] + (Vector2){1, 0};
        for (int j = 0; j < doorkeycount; j++)
        {
            if (doorStartingLoadKeyPositions[j].x == totheleft.x && doorStartingLoadKeyPositions[j].y == totheleft.y)
            {
                doorKeyPos[i] = doorStartingLoadKeyPositions[j];
                break;
            }
            if (doorStartingLoadKeyPositions[j].x == totheright.x && doorStartingLoadKeyPositions[j].y == totheright.y)
            {
                doorKeyPos[i] = doorStartingLoadKeyPositions[j];
                break;
            }
        }
    }
}

void PickupBatteryFromKeyhole(Player* p, int dooridx)
{
    Battery battery = doorKeyholebattery[dooridx];
    PickupBattery(p, battery);
    doorKeyholeHasBattery[dooridx] = false;
}

void PlaceBatteryIntoKeyhole(Player* p, Battery battery, int dooridx)
{
    doorKeyholeHasBattery[dooridx] = true;
    doorKeyholebattery[dooridx] = battery;
    batteryInDoor[battery] = true;

    switch (battery)
    {
        case bigjump:
        {
            batteryInUse[bigjump] = false;
            batteryInDoor[bigjump] = true;
            batteryPositions[bigjump] = (Vector2){p->pos.x, p->pos.y - Round(TMSIZE / 2)};
            printf("Dropped bigjump battery\n");
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case doublejump:
        {
            batteryInUse[doublejump] = false;
            batteryInDoor[doublejump] = true;
            batteryPositions[doublejump] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            printf("Dropped doublejump battery\n");
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case rapidfire:
        {
            batteryInUse[rapidfire] = false;
            batteryInDoor[rapidfire] = true;
            batteryPositions[rapidfire] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->shootDelay = bulletDelays[normalbullet];
            p->bullettype = normalbullet;
            break;
        }
        case heavyfire:
        {
            batteryInUse[heavyfire] = false;
            batteryInDoor[heavyfire] = true;
            batteryPositions[heavyfire] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->shootDelay = bulletDelays[normalbullet];
            p->bullettype = normalbullet;
            break;
        }
        case quickie:
        {
            batteryInUse[quickie] = false;
            batteryInDoor[quickie] = true;
            batteryPositions[quickie] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->speed = NORMALSPEED;
            break;
        }
        case tanky:
        {
            batteryInUse[tanky] = false;
            batteryInDoor[tanky] = true;
            batteryPositions[tanky] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->maxHealth = NORMALMAXHEALTH;
            break;
        }
    }
}

void RenderDoors(Camera2D camera, Player* p)
{
    bool didSomething = false;
    for (int i = 0; i < doorCount; i++)
    {
        doorOpen[i] = doorKeyholeHasBattery[i]; // cbf to find and replace
        // door itself
        if (TileOnScreen(doorStartPos[i], camera)) {
            Color doorColor = doorOpen[i] ? GREEN : RED;
            #define DOORTHICKNESS 4
            DrawLineEx(doorStartPos[i] * TMSIZE + (Vector2){DOORTHICKNESS/2., TMSIZE}, doorEndPos[i] * TMSIZE + (Vector2){DOORTHICKNESS/2., 0}, DOORTHICKNESS, doorColor);
        }
        // keyhole
        if (TileOnScreen(doorKeyPos[i], camera))
        {
            DrawRectangleV(doorKeyPos[i] * TMSIZE, {8, 12}, LIGHTGRAY);

            if (Vector2DistanceSqr(doorKeyPos[i] * TMSIZE, p->pos) < (INTERACTION_DISTANCE * INTERACTION_DISTANCE) && !didSomething)
            {
                if (!doorKeyholeHasBattery[i]) // if no battery already in
                {
                    if (showingMessage)
                    {
                        #define INSERTBATTERYFONTSIZE 10
                        const char* text = "Press a key to insert a battery: ";
                        if (batteryInUse[bigjump]) text = TextFormat("%s'1' bigjump ", text);
                        if (batteryInUse[doublejump]) text = TextFormat("%s'2' doublejump ", text);
                        if (batteryInUse[rapidfire]) text = TextFormat("%s'3' rapidfire ", text);
                        if (batteryInUse[heavyfire]) text = TextFormat("%s'4' heavyfire ", text);
                        if (batteryInUse[quickie]) text = TextFormat("%s'5' quickie ", text);
                        if (batteryInUse[tanky]) text = TextFormat("%s'6' tanky ", text);
                        int atleastone = (batteryInUse[bigjump] || batteryInUse[doublejump] || batteryInUse[rapidfire] || batteryInUse[heavyfire] || batteryInUse[quickie] || batteryInUse[tanky]);
                        if (!atleastone) text = "You need a battery to do this!";
                        int offset = MeasureText(text, INSERTBATTERYFONTSIZE)/2;
                        DrawText(text, doorKeyPos[i].x * TMSIZE - offset, doorKeyPos[i].y * TMSIZE - TMSIZE, INSERTBATTERYFONTSIZE, WHITE);

                        if (atleastone)
                        {
                            if (IsKeyPressed(KEY_ONE) && batteryInUse[bigjump]) {
                                PlaceBatteryIntoKeyhole(p, bigjump, i);
                                showingMessage = false;
                                didSomething = true;
                            }
                            else if (IsKeyPressed(KEY_TWO) && batteryInUse[doublejump]) {
                                PlaceBatteryIntoKeyhole(p, doublejump, i);
                                showingMessage = false;
                                didSomething = true;
                            }
                            else if (IsKeyPressed(KEY_THREE) && batteryInUse[rapidfire]) {
                                PlaceBatteryIntoKeyhole(p, rapidfire, i);
                                showingMessage = false;
                                didSomething = true;
                            }
                            else if (IsKeyPressed(KEY_FOUR) && batteryInUse[heavyfire]) {
                                PlaceBatteryIntoKeyhole(p, heavyfire, i);
                                showingMessage = false;
                                didSomething = true;
                            }
                            else if (IsKeyPressed(KEY_FIVE) && batteryInUse[quickie]) {
                                PlaceBatteryIntoKeyhole(p, quickie, i);
                                showingMessage = false;
                                didSomething = true;
                            }
                            else if (IsKeyPressed(KEY_SIX) && batteryInUse[tanky]) {
                                PlaceBatteryIntoKeyhole(p, tanky, i);
                                showingMessage = false;
                                didSomething = true;
                            }
                        }
                    }
                    else
                    {
                        const char* string = "Press E to place battery\0";
                        int stringsize = MeasureText(string, 10);
                        DrawText(string, doorKeyPos[i].x*TMSIZE - stringsize/2.f, doorKeyPos[i].y*TMSIZE - TMSIZE, 10, WHITE);

                        if (IsKeyPressed(KEY_E) && !didSomething && !showingMessage)
                        {
                            showingMessage = true;
                            showingMessageIndex = i;
                            didSomething = true;
                        }
                    }
                }
                else // has a battery already, we need to have option to remove
                {
                    const char* string = "Press E to pickup battery\0";
                    int stringsize = MeasureText(string, 10);
                    DrawText(string, doorKeyPos[i].x *TMSIZE - stringsize/2.f, doorKeyPos[i].y*TMSIZE - TMSIZE, 10, WHITE);

                    if (IsKeyPressed(KEY_E) && !didSomething)
                    {
                        PickupBatteryFromKeyhole(p, i);
                        didSomething = true;
                    }
                }
            }
            else if (showingMessage && showingMessageIndex == i && !didSomething)
            {
                showingMessage = false;
            }
        }
    }
}

/***********/
/* Enemies */
/***********/
#define MAX_ENEMIES 200
typedef enum Enemy
{
    regular,
    heavy,
    boss,
    enemytypecount
} Enemy;

float enemySpeeds[enemytypecount] =
{
    1,
    0.5,
    0,
};

float enemyMaxHealth[enemytypecount] =
{
    10,
    20,
    50,
};

Vector2 enemySize[enemytypecount] =
{
    {8, 10},
    {10, 16},
    {20, 20},
};

Color enemyColor[enemytypecount] =
{
    MAROON,
    GREEN,
    BLACK
};

#define ENEMYSPAWNCOOLDOWNSECONDS 5

double enemySpawnTimers[MAX_ENEMIES] = {};

// as tiles
Vector2 enemySpawnPoints[MAX_ENEMIES] = {};

// as position
Vector2 enemyPositions[MAX_ENEMIES] = {};
Vector2 enemyMovementRemainders[MAX_ENEMIES] = {};
int enemyDirections[MAX_ENEMIES] = {};
bool enemyFlying[MAX_ENEMIES] = {};
bool enemyOnGround[MAX_ENEMIES] = {};
int enemyHealth[MAX_ENEMIES] = {};
bool enemyDead[MAX_ENEMIES] = {};
Enemy enemyTypes[MAX_ENEMIES] = {};
int enemyCount = 0;

// debug info
int aliveEnemyCount = 0;

void SetEnemyStatsProperBecauseInitValueNotWorking()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemyDirections[i] = (rand() % 2 == 0) ? 1 : -1;
        enemyDead[i] = true;
    }
}

bool EnemyCollidingAt(int enemyidx, Vector2 pos, Solid* solids, int solidCount)
{
    Rectangle enemyRect = {pos.x - enemySize[enemyTypes[enemyidx]].x / 2, pos.y - enemySize[enemyTypes[enemyidx]].y / 2, enemySize[enemyTypes[enemyidx]].x, enemySize[enemyTypes[enemyidx]].y};

    // Check if colliding with the tilemap
    {
        // prevent enemy from walking on spikes
        if (tilemap[(int)(enemyRect.x + enemyRect.width / 2) / TMSIZE][(int)(enemyRect.y + enemyRect.height) / TMSIZE + 1] == spike)
        {
            return true;
        }
        if (tilemap[(int)(enemyRect.x - enemyRect.width / 2) / TMSIZE][(int)(enemyRect.y + enemyRect.height) / TMSIZE + 1] == spike)
        {
            return true;
        }

        int minx = (enemyRect.x / TMSIZE) - 1;
        int miny = (enemyRect.y / TMSIZE) - 1;
        int maxx = (enemyRect.x + enemyRect.width / TMSIZE) + 1;
        int maxy = (enemyRect.y + enemyRect.height / TMSIZE) + 1;

        for (int y = miny; y <= maxy; y++)
        {
            for (int x = minx; x <= maxx; x++)
            {
                if (x >= 0 && x < WORLD_SIZE_X && y >= 0 && y < WORLD_SIZE_Y)
                {
                    if (tilemap[x][y] != empty)
                    {
                        Rectangle tileRect = {(float)x * TMSIZE, (float)y * TMSIZE, TMSIZE, TMSIZE};
                        if (CheckCollisionRecs(enemyRect, tileRect))
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }

    // Check if colliding with any solid in the scene, currently loops over every solid object
    for (int i = 0; i < solidCount; i++)
    {
        Solid* s = &solids[i];
        if (s->collideable)
        {
            Rectangle solidRect = {s->pos.x - s->size.x / 2, s->pos.y - s->size.y / 2, s->size.x, s->size.y};
            if (CheckCollisionRecs(enemyRect, solidRect))
            {
                return true;
            }
        }
    }

    // Check if colliding with any door
    for (int j = 0; j < doorCount; j++)
    {
        if (!doorOpen[j])
        {
            Rectangle doorRect = {doorStartPos[j].x * TMSIZE, doorEndPos[j].y * TMSIZE, DOORTHICKNESS, (doorStartPos[j].y - doorEndPos[j].y) * TMSIZE + TMSIZE};
            if (CheckCollisionRecs(enemyRect, doorRect))
            {
                return true;
            }
        }
    }

    return false;
}


void EnemyPhysicsProcess(Solid* solids, int solidCount, double time)
{
    // Print all enemy arrays for debugging
    printf("\n\n\n\n\n\n");
    printf("Enemy Positions:\n");
    for (int i = 0; i < enemyCount; i++) {
        printf("Enemy %d: Position = (%f, %f)\n", i, enemyPositions[i].x, enemyPositions[i].y);
    }
    printf("Enemy Sizes:\n");
    for (int i = 0; i < enemyCount; i++) {
        printf("Enemy %d: Size = (%f, %f)\n", i, enemySize[enemyTypes[i]].x, enemySize[enemyTypes[i]].y);
    }

    printf("Enemy Directions:\n");
    for (int i = 0; i < enemyCount; i++) {
        printf("Enemy %d: Direction = %d\n", i, enemyDirections[i]);
    }

    printf("Enemy Health:\n");
    for (int i = 0; i < enemyCount; i++) {
        printf("Enemy %d: Health = %d\n", i, enemyHealth[i]);
    }

    printf("Enemy Dead Status:\n");
    for (int i = 0; i < enemyCount; i++) {
        printf("Enemy %d: Dead = %d\n", i, enemyDead[i]);
    }

    printf("Enemy Types:\n");
    for (int i = 0; i < enemyCount; i++) {
        printf("Enemy %d: Type = %d\n", i, enemyTypes[i]);
    }

    printf("Enemy Spawn Timers:\n");
    for (int i = 0; i < enemyCount; i++) {
        printf("Enemy %d: Spawn Timer = %f\n", i, enemySpawnTimers[i]);
    }

    printf("Enemy Spawn Points:\n");
    for (int i = 0; i < enemyCount; i++) {
        printf("Enemy %d: Spawn Point = (%f, %f)\n", i, enemySpawnPoints[i].x, enemySpawnPoints[i].y);
    }
    // Iterate through each enemy to process their physics
    for (int i = 0; i < enemyCount; i++)
    {
        // Check if the enemy is not dead
        if (!enemyDead[i])
        {
            // If the enemy's health is less than 1, mark it as dead and set a respawn timer
            if (enemyHealth[i] < 1)
            {
                enemySpawnTimers[i] = ENEMYSPAWNCOOLDOWNSECONDS;
                enemyDead[i] = true;
                aliveEnemyCount--;
            }

            if (!enemyFlying[i] && !enemyOnGround[i])
            {
                if (!EnemyCollidingAt(i, enemyPositions[i] + (Vector2){0, 1}, solids, solidCount))
                {
                    enemyPositions[i].y += 1;
                }
                else
                {
                    enemyOnGround[i] = true;
                }
            }

            enemyMovementRemainders[i].x += enemySpeeds[enemyTypes[i]] * enemyDirections[i];
            int moveX = Round(enemyMovementRemainders[i].x);
            if (moveX != 0)
            {
                enemyMovementRemainders[i].x -= moveX;
                int sign = Sign(moveX);
                while (moveX != 0)
                {
                    Vector2 newPos = enemyPositions[i] + (Vector2){(float)sign, 0};
                    if (!enemyFlying[i] && tilemap[(int)newPos.x / TMSIZE][(int)newPos.y / TMSIZE + 1] == spike)
                    {
                        enemyDirections[i] *= -1;
                        break;
                    }
                    if (!EnemyCollidingAt(i, newPos, solids, solidCount))
                    {
                        enemyPositions[i].x += sign;
                        moveX -= sign;
                    }
                    else
                    {
                        enemyDirections[i] *= -1;
                        break;
                    }
                }
            }

            enemyMovementRemainders[i].y += enemySpeeds[enemyTypes[i]];
            int moveY = Round(enemyMovementRemainders[i].y);
            if (moveY != 0)
            {
                enemyMovementRemainders[i].y -= moveY;
                int sign = Sign(moveY);
                while (moveY != 0)
                {
                    Vector2 newPos = enemyPositions[i] + (Vector2){0, (float)sign};
                    if (!EnemyCollidingAt(i, newPos, solids, solidCount))
                    {
                        enemyPositions[i].y += sign;
                        moveY -= sign;
                    }
                    else
                    {
                        break;
                    }
                }
            }

        }
        else if (enemyDead[i]) // If the enemy is dead
        {
            // Decrease the respawn timer
            enemySpawnTimers[i] -= physicsDTs;
            // If the respawn timer has run out, respawn the enemy
            if (enemySpawnTimers[i] < 0 + EPSILON)
            {
                enemySpawnTimers[i] = 0;
                enemyHealth[i] = enemyMaxHealth[enemyTypes[i]];
                enemyPositions[i] = enemySpawnPoints[i] * TMSIZE;
                enemyDead[i] = false;
                aliveEnemyCount++;
            }
        }
    }
}

void RenderEnemies()
{
    for (int i = 0; i < enemyCount; i++)
    {
        if (!enemyDead[i])
        {
            DrawRectangleV(enemyPositions[i] - enemySize[enemyTypes[i]] / 2, enemySize[enemyTypes[i]], enemyColor[enemyTypes[i]]);
        }
    }
}


/***********/
/* Tilemap */
/***********/
Color tileColor[tilecount] =
{
    BLANK,
    DARKGRAY,
    GRAY,
};

Texture tileTex[tilecount] = {};

void LoadTileTextures()
{
    tileTex[stone] = LoadTexture("assets/stone.png");
    tileTex[breakable] = LoadTexture("assets/breakable.png");
    tileTex[spike] = LoadTexture("assets/spike.png");
}

#define windowWidth 1280
#define windowHeight 720


Vector2 player_spawnpoint = {};

void LoadTilemap(const char* path)
{
    FILE* fp = fopen(path, "r+");
    for (int y = 0; y < WORLD_SIZE_Y; y++)
    {
        for (int x = 0; x < WORLD_SIZE_X; x++)
        {
            Tile t = empty; // if an invalid character is given itl just be empty
            char c;
            fscanf(fp, " %c", &c);
            switch (c)
            {
                case 'S':
                {
                    player_spawnpoint = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                }
                case '.':
                {
                    t = empty;
                    break;
                }
                case '#':
                {
                    t = stone;
                    break;
                }
                case 'B':
                {
                    t = breakable;
                    break;
                }
                case '^':
                {
                    t = spike;
                    break;
                }
                case 'e':
                {
                    enemyTypes[enemyCount] = regular;
                    enemySpawnPoints[enemyCount++] = {(float)x, (float)y};
                    break;
                }
                case 'E':
                {
                    enemyTypes[enemyCount] = heavy;
                    enemySpawnPoints[enemyCount++] = {(float)x, (float)y};
                    break;
                }
                case 'D':
                {
                    // door
                    doorStartingLoadPositions[doorloadcount++] = (Vector2){(float)x, (float)y};
                    break;
                }
                case 'k':
                {
                    // key
                    doorStartingLoadKeyPositions[doorkeycount++] = (Vector2){(float)x, (float)y};
                    break;
                }
                case '0': // bigjump
                {
                    batteryPositions[0] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                }
                case '1': // doublejump
                {
                    batteryPositions[1] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                }
                case '2': // rapidfire
                {
                    batteryPositions[2] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                }
                case '3': // heavyfire
                {
                    batteryPositions[3] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                }
                case '4': // quickie
                {
                    batteryPositions[4] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                }
                case '5': // tanky
                {
                    batteryPositions[5] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                }
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
                DrawTexture(tileTex[tilemap[x][y]], x * TMSIZE, y * TMSIZE, WHITE);
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
    bulletDamage[lightbullet] = (int)(lightbulletdps * bulletDelays[lightbullet]);
    bulletDamage[heavybullet] = (int)(heavybulletdps * bulletDelays[heavybullet]);
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
            // tilemap collisions
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

            // door collisions
            for (int j = 0; j < doorCount; j++)
            {
                if (!doorOpen[j])
                {
                    Rectangle doorRect = (Rectangle){doorStartPos[j].x * TMSIZE, doorEndPos[j].y * TMSIZE, DOORTHICKNESS, (doorStartPos[j].y - doorEndPos[j].y) * TMSIZE + TMSIZE};
                    if (CheckCollisionCircleRec(bulletsPs[i], bulletRadius[bulletsTs[i]], doorRect))
                    {
                        KillBullet(i);
                        break;
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
    DrawTextureV(a.tex, a.pos - a.size/2, a.tint);
    DrawRectangleV(a.pos - (a.size / 2), a.size, a.col);
}

void PlayerStandingOnSpikeCheck(Player* p)
{
    Vector2 tmpos = {p->pos.x/TMSIZE, p->pos.y/TMSIZE + 1};
    tmpos = tmpos * TMSIZE;
    AABB tmposaabb = {tmpos, tmpos + TMSIZE};

    if (tmposaabb.min.x < p->pos.x && tmposaabb.max.x > p->pos.x
     && tmposaabb.min.y <= p->pos.y + p->size.y/2 + 1 && tmposaabb.max.y >= p->pos.y + p->size.y/2 + 1)
    { p->standingOnSpike = true; }
    else
    { p->standingOnSpike = false; }
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

    // Check if colliding with any door
    // collisiondoor
    for (int j = 0; j < doorCount; j++)
    {
        if (!doorOpen[j])
        {
            AABB daabb = {.min = doorEndPos[j] * TMSIZE, .max = doorStartPos[j] * TMSIZE + (Vector2){DOORTHICKNESS, TMSIZE}};
            if (AABBsColliding(aabb, daabb))
            {
                return true;
            }
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
    // Player alive check, cuz death is part of physics
    {
        if (p->health < 1) // dead
        {

        }
        else // alive
        {

        }
    }

    PlayerStandingOnSpikeCheck(p);
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

    // Spike standing logic
    {
        if (p->standingOnSpike && time - p->lastspikehit > SPIKEHITCOOLDOWN - EPSILON)
        {
            bool goright = rand() % 2; // 0 or 1
            int dir = (goright) ? 1 : -1;
            #define SPIKEUPVEL -4
            #define SPIKESIDEVEL 1
            #define SPIKEDAMAGE 40
            p->vel.y = SPIKEUPVEL;
            p->damageimpulse.x = SPIKESIDEVEL * dir;
            p->health -= SPIKEDAMAGE;
            p->lastspikehit = time;
            p->lasthittaken = time;
        }
    }
    // Hit-taking logic
    {
        if (time - p->lasthittaken < DAMAGETINTDURATION)
        {
            double timeleft = DAMAGETINTDURATION - (time - p->lasthittaken);
            p->tint = (Color){(unsigned char)(255 * timeleft * 10), 0, 0, 0xff};
        }
        else // if the damage tint duration is done
        {
            p->tint = WHITE;
            // reset the damage impulse
            p->damageimpulse = {};
        }
    }
    // Impulse logic (not actual impulse idk what that is)
    {
        p->vel = p->vel + p->damageimpulse;
        p->damageimpulse = p->damageimpulse - p->damageimpulse * physicsDTs;
    }

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
