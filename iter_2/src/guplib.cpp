#include "/Users/tajgordon/raylib-5.0_webassembly/include/raylib.h"
#include "/Users/tajgordon/raylib-5.0_webassembly/include/raymath.h"
#include "guplib.hpp"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include "math.h"

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif
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
Vector2 operator*(Vector2 v1, Vector2 v2)
{
    return {v1.x * v2.x, v1.y * v2.y};
}
Vector2 operator-(Vector2 v, float f)
{
    return {v.x - f, v.y - f};
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
float bulletSpeed[bulletcount] =
{
    0,
    500,
    200,
    350,
    200, // enemylightbullet
};

float depthfactor = 0;

#define MAX_BULLETS 400
Bullet bulletsTs[MAX_BULLETS] = {none};
Vector2 bulletsVs[MAX_BULLETS] = {0};
Vector2 bulletsPs[MAX_BULLETS] = {0};
int bulletCount = 0;

Texture batteryTexture;
Texture batterySlotTexture;
Texture shipTexture;
void LoadBatteryTexture()
{
    batteryTexture = LoadTexture("assets/battery.png");
    batterySlotTexture = LoadTexture("assets/batteryslot.png");

    shipTexture = LoadTexture("assets/ship.png");
}

float bulletDelays[bulletcount] =
{
    0,
    0.1, // light
    0.6, // heavy
    0.3, // normal
};
typedef enum Gamestate
{
    running,
    gameover,
    gameloadingscreen,
    win,
    specialitem,
    reload,
    gamestatecount
} Gamestate;

Vector2 batteryPositions[batterycount] = {};
bool batteryInUse[batterycount] = {};
bool batteryInDoor[batterycount] = {};
bool batteryCanBePickedUp[batterycount] = {};
int maxscore = 0;
int maxbatteriesfound = 0;
int timeswon = 0;

Gamestate gamestate = gameloadingscreen;

#define MAX_DISPLAY_MESSAGES 100
#define MAX_DISPLAYMESSAGE_SIZE 100
#define DISPLAYMESSAGEFONTSIZE 5
double displayMessagesTimers[MAX_DISPLAY_MESSAGES] = {};
Vector2 displayMessagesPositions[MAX_DISPLAY_MESSAGES] = {};
char displayMessages[MAX_DISPLAY_MESSAGES][MAX_DISPLAYMESSAGE_SIZE] = {};
int displayMessagesCount = 0;

bool TileOnScreen(Vector2 tilepos, Camera2D camera)
{
    Vector2 screenMin = GetScreenToWorld2D((Vector2){0, 0}, camera);
    Vector2 screenMax = GetScreenToWorld2D((Vector2){(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

    Vector2 tileMin = tilepos * TMSIZE;
    Vector2 tileMax = tileMin + TMSIZE;

    return !(tileMax.x < screenMin.x || tileMin.x > screenMax.x || tileMax.y < screenMin.y || tileMin.y > screenMax.y);
}
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
int maxHeight = 0; // gonna be lower than minheight
int minHeight = 0; // gonna be bigger than maxheight

bool showingMessage = false;
int showingMessageIndex = 0;

Camera2D specialCamera = {};

bool foundSpecialItem = false;
Vector2 specialItemPos = {};
Texture2D specialItemTex = {};

void RenderSpecialItem(Player* p, Camera2D camera)
{
    if (!foundSpecialItem && TileOnScreen(specialItemPos/TMSIZE, camera))
    {
        DrawTexture(specialItemTex, specialItemPos.x, specialItemPos.y, WHITE);


        if (Vector2Distance(p->pos, specialItemPos) < (INTERACTION_DISTANCE * INTERACTION_DISTANCE))
        {
            const char* text = "?!?!??!?!?!?!?!?!?!?!??!?!?!?!?!\0";
            #define specialitemtextcolor (Color){255, 203, 0, 0xff}
            #define specialitemfontsize 25
            int textoffset = MeasureText(text, specialitemfontsize)/2;
            DrawText(text, p->pos.x - textoffset, p->pos.y - TMSIZE * 2, specialitemfontsize, specialitemtextcolor);

            if (IsKeyPressed(KEY_E))
            {
                foundSpecialItem = true;
                gamestate = specialitem;
            }
        }
    }
}

/**************/
/* SHIP STUFF */
/**************/
// the amount of batteries needed to power the ship
#define BATTERIESNEEDEDFORSHIP 4
Vector2 shipPosition = {};
bool shipOpen = false;
bool shipBatterySlotHasBattery[BATTERIESNEEDEDFORSHIP] = {};
Battery shipBatterySlotBatteryType[BATTERIESNEEDEDFORSHIP] = {};
int numBatteriesInShip = 0;
bool bigbatteryinship = false;

bool batteryInShip[batterycount] = {};

int shipBatteryCount = 0;
#define SHIPBATTERYSLOTSIZE (Vector2){8, 12}


bool showingShipBatterySlotSelectionMessage = false;
bool showingShipPlaceBatteryMessage = false;
bool showingBatterySlotRemovalMessage = false;
int shipSlot = 0;

bool foundBattery[batterycount] = {};
int batteriesFound = 0;

void DrawBatteryStatus()
{
    const char* batteryNames[batterycount] = {
        "Big Jump",
        "Double Jump",
        "Rapid Fire",
        "Heavy Fire",
        "Quickie",
        "Tanky",
        "Vision Up"
    };

    if (IsKeyDown(KEY_TAB))
    {

        for (int i = 0; i < batterycount; i++)
        {
            Color statusColor = foundBattery[i] ? GREEN : RED;
            const char* statusText = foundBattery[i] ? "Found" : "Missing";
            int startY = GetScreenHeight() - 50;
            DrawText(TextFormat("%s:", batteryNames[i]), 10 + i * 180, startY, 20, statusColor);
            DrawText(TextFormat("%s", statusText), 10 + i * 180, startY + 25, 20, statusColor);
        }
    }
}

Color batteryColor[batterycount] =
{
    YELLOW,// bigjump,
    ORANGE,// doublejump,
    BLUE,// rapidfire,
    RED,// heavyfire,
    GREEN,// quickie,
    BROWN,// tanky,
    MAGENTA,
};

void PlaceBatteryIntoShipKeyhole(Player* p, Battery batterytype, int idx)
{
    numBatteriesInShip++;
    shipBatterySlotHasBattery[idx] = true;
    shipBatterySlotBatteryType[idx] = batterytype;
    batteryInShip[batterytype] = true;
    batteryInUse[batterytype] = false;

    switch (batterytype)
    {
        case bigjump:
        {
            batteryPositions[bigjump] = (Vector2){p->pos.x, p->pos.y - Round(TMSIZE / 2)};
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case doublejump:
        {
            batteryPositions[doublejump] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case rapidfire:
        {
            batteryPositions[rapidfire] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->shootDelay = bulletDelays[normalbullet];
            p->bullettype = normalbullet;
            break;
        }
        case heavyfire:
        {
            batteryPositions[heavyfire] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->shootDelay = bulletDelays[normalbullet];
            p->bullettype = normalbullet;
            break;
        }
        case quickie:
        {
            batteryPositions[quickie] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->speed = NORMALSPEED;
            break;
        }
        case tanky:
        {
            batteryPositions[tanky] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->maxHealth = NORMALMAXHEALTH;
            break;
        }
        case visionup:
        {
            batteryPositions[visionup] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->FOV = DEFAULT_FOV_RADIUS;
            break;
        }
    }
}

void PickupBattery(Player* p, Battery battery);
void PickupBatteryFromShipKeyhole(Player* p, int idx)
{
    Battery battery = shipBatterySlotBatteryType[idx];
    PickupBattery(p, battery);
    shipBatterySlotHasBattery[idx] = false;
    numBatteriesInShip--;
}

void RenderShip(Camera2D camera, Player* p)
{
    bool didSomething = false;
    if (numBatteriesInShip == BATTERIESNEEDEDFORSHIP) shipOpen = true;

    // render ship itself
    // ship position is in the middle of a block
    // DrawRectangleV(shipPosition - 32, {64, 64}, shipColor);

    if (TileOnScreen(shipPosition/TMSIZE - (Vector2){3, 0}, camera) || TileOnScreen(shipPosition/TMSIZE + (Vector2){3, 0}, camera)
     || TileOnScreen(shipPosition/TMSIZE - (Vector2){0, 3}, camera) || TileOnScreen(shipPosition/TMSIZE + (Vector2){0, 3}, camera))
    {
        DrawTexture(shipTexture, shipPosition.x - 64, shipPosition.y - 40, WHITE);

        if (shipBatterySlotHasBattery[0])
        { // Slot one
            Battery battery = shipBatterySlotBatteryType[0];
            DrawTexture(batteryTexture, shipPosition.x - TMSIZE, shipPosition.y - 1.5 * TMSIZE, batteryColor[battery]);
        }
        if (shipBatterySlotHasBattery[1])
        { // Slot two
            Battery battery = shipBatterySlotBatteryType[1];
            DrawTexture(batteryTexture, shipPosition.x, shipPosition.y - 1.5 * TMSIZE, batteryColor[battery]);
        }
        if (shipBatterySlotHasBattery[2])
        { // Slot three
            Battery battery = shipBatterySlotBatteryType[2];
            DrawTexture(batteryTexture, shipPosition.x + TMSIZE, shipPosition.y - 1.5 * TMSIZE, batteryColor[battery]);
        }
        if (shipBatterySlotHasBattery[3])
        { // Slot three
            Battery battery = shipBatterySlotBatteryType[3];
            DrawTexture(batteryTexture, shipPosition.x + TMSIZE * 2, shipPosition.y - 1.5 * TMSIZE, batteryColor[battery]);
        }
    }


    #define SHIPINTERACTIONDISTANCE 50
    if (shipOpen && Vector2DistanceSqr(p->pos, shipPosition) < (SHIPINTERACTIONDISTANCE * SHIPINTERACTIONDISTANCE))
    {
        const char* text = "Press E to board the ship\0";
        #define GAMEWINSHIPFONTSIZE 15
        int offset = MeasureText(text, GAMEWINSHIPFONTSIZE)/2;
        DrawText(text, shipPosition.x - offset, shipPosition.y - TMSIZE, GAMEWINSHIPFONTSIZE, GOLD);

        if (IsKeyPressed(KEY_E) && foundSpecialItem)
        {
            gamestate = win;
            timeswon++;
        }
        else if (IsKeyPressed(KEY_E))
        {
            AddDisplayMessage(shipPosition - (Vector2){0, TMSIZE * 3}, 3,  "I can't leave without my special item\0");
            showingShipPlaceBatteryMessage = false;
            showingShipBatterySlotSelectionMessage = false;
            showingBatterySlotRemovalMessage = false;
        }
        // could let you take them out, but nah, "electric current holds them in place"
    }
    else if (Vector2DistanceSqr(p->pos, shipPosition + (Vector2){TMSIZE, 0}) < (SHIPINTERACTIONDISTANCE * SHIPINTERACTIONDISTANCE))
    {
        if (showingShipBatterySlotSelectionMessage)
        {
            #define thisfontsize 12
            const char* text = "Which slot would you like to place a battery into? \n '1' '2' '3' '4'\0";
            int textOffset = MeasureText(text, thisfontsize)/2;
            DrawText(text, p->pos.x - textOffset, p->pos.y - TMSIZE * 3, thisfontsize, WHITE);

            if (IsKeyPressed(KEY_ONE) && !didSomething)
            {
                shipSlot = 0;
                didSomething = true;

                if (shipBatterySlotHasBattery[shipSlot])
                {
                    showingBatterySlotRemovalMessage = true;
                    showingShipBatterySlotSelectionMessage = false;
                }
                else
                {
                    showingShipBatterySlotSelectionMessage = false;
                    showingShipPlaceBatteryMessage = true;
                }
            }
            else if (IsKeyPressed(KEY_TWO) && !didSomething)
            {
                shipSlot = 1;
                didSomething = true;

                if (shipBatterySlotHasBattery[shipSlot])
                {
                    showingBatterySlotRemovalMessage = true;
                    showingShipBatterySlotSelectionMessage = false;
                }
                else
                {
                    showingShipBatterySlotSelectionMessage = false;
                    showingShipPlaceBatteryMessage = true;
                }
            }
            else if (IsKeyPressed(KEY_THREE) && !didSomething)
            {
                shipSlot = 2;
                didSomething = true;

                if (shipBatterySlotHasBattery[shipSlot])
                {
                    showingBatterySlotRemovalMessage = true;
                    showingShipBatterySlotSelectionMessage = false;
                }
                else
                {
                    showingShipBatterySlotSelectionMessage = false;
                    showingShipPlaceBatteryMessage = true;
                }
            }
            else if (IsKeyPressed(KEY_FOUR) && !didSomething)
            {
                shipSlot = 3;
                didSomething = true;

                if (shipBatterySlotHasBattery[shipSlot])
                {
                    showingBatterySlotRemovalMessage = true;
                    showingShipBatterySlotSelectionMessage = false;
                }
                else
                {
                    showingShipBatterySlotSelectionMessage = false;
                    showingShipPlaceBatteryMessage = true;
                }
            }
        }
        else if (showingShipPlaceBatteryMessage)
        {
            const char* text = "Press a key to insert a battery: \n";
            if (batteryInUse[bigjump]) text = TextFormat("%s'1' bigjump ", text);
            if (batteryInUse[doublejump]) text = TextFormat("%s'2' doublejump ", text);
            if (batteryInUse[rapidfire]) text = TextFormat("%s'3' rapidfire ", text);
            if (batteryInUse[heavyfire]) text = TextFormat("%s'4' heavyfire ", text);
            if (batteryInUse[quickie]) text = TextFormat("%s'5' quickie ", text);
            if (batteryInUse[tanky]) text = TextFormat("%s'6' tanky ", text);
            int atleastone = (batteryInUse[bigjump] || batteryInUse[doublejump] || batteryInUse[rapidfire] || batteryInUse[heavyfire] || batteryInUse[quickie] || batteryInUse[tanky]);
            if (!atleastone) text = "You need a battery to do this!";
            int textOffset = MeasureText(text, INSERTBATTERYFONTSIZE)/2;
            DrawText(text, p->pos.x - textOffset, p->pos.y - TMSIZE * 4, thisfontsize, WHITE);

            if (atleastone)
            {
                if (IsKeyPressed(KEY_ONE) && batteryInUse[bigjump])
                {
                    PlaceBatteryIntoShipKeyhole(p, bigjump, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_TWO) && batteryInUse[doublejump])
                {
                    PlaceBatteryIntoShipKeyhole(p, doublejump, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_THREE) && batteryInUse[rapidfire])
                {
                    PlaceBatteryIntoShipKeyhole(p, rapidfire, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_FOUR) && batteryInUse[heavyfire])
                {
                    PlaceBatteryIntoShipKeyhole(p, heavyfire, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_FIVE) && batteryInUse[quickie])
                {
                    PlaceBatteryIntoShipKeyhole(p, quickie, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_SIX) && batteryInUse[tanky])
                {
                    PlaceBatteryIntoShipKeyhole(p, tanky, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_SEVEN) && batteryInUse[visionup])
                {
                    PlaceBatteryIntoShipKeyhole(p, visionup, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
            }
        }
        else if (showingBatterySlotRemovalMessage)
        {
            const char* text = "This battery slot already has a battery\0";
            const char* subtitle = "Press E to pick it up\0";
            int textoffset = MeasureText(text, thisfontsize)/2;
            int subtextoffset = MeasureText(subtitle, thisfontsize)/2;
            DrawText(text, p->pos.x - textoffset, p->pos.y - TMSIZE * 3, thisfontsize, WHITE);
            DrawText(subtitle, p->pos.x - subtextoffset, p->pos.y - TMSIZE * 2, thisfontsize, WHITE);

            if (IsKeyPressed(KEY_E) && !didSomething)
            {
                PickupBatteryFromShipKeyhole(p, shipSlot);
                didSomething = true;
                showingBatterySlotRemovalMessage = false;
            }
        }
        else
        {
            const char* text = "Press E to place a battery\0";
            int textOffset = MeasureText(text, thisfontsize)/2;
            DrawText(text, p->pos.x - textOffset, p->pos.y - TMSIZE * 3, thisfontsize, WHITE);

            if (IsKeyPressed(KEY_E) && !didSomething)
            {
                showingShipBatterySlotSelectionMessage = true;
                didSomething = true;
            }
        }
    }
    else if (showingShipPlaceBatteryMessage || showingShipBatterySlotSelectionMessage || showingBatterySlotRemovalMessage)
    {
        showingShipPlaceBatteryMessage = false;
        showingShipBatterySlotSelectionMessage = false;
        showingBatterySlotRemovalMessage = false;
    }
}

/*****************/
/* Battery Stuff */
/*****************/
// jump velocity needs to be negative retard



void DropBattery(Player* p, Battery battery)
{
    if (!batteryInUse[battery]) return;
    switch (battery)
    {
        case bigjump:
        {
            batteryInUse[bigjump] = false;
            batteryPositions[bigjump] = (Vector2){p->pos.x, p->pos.y - Round(TMSIZE / 2)};
            // printf("Dropped bigjump battery\n");
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case doublejump:
        {
            batteryInUse[doublejump] = false;
            batteryPositions[doublejump] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            // printf("Dropped doublejump battery\n");
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
        case visionup:
        {
            batteryInUse[visionup] = false;
            batteryPositions[visionup] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            p->FOV = DEFAULT_FOV_RADIUS;
            break;
        }
    }
}

void PickupBattery(Player* p, Battery battery)
{
    if (!foundBattery[battery]) { foundBattery[battery] = true; batteriesFound++; }
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
        case visionup:
        {
            p->FOV = FOV_RADIUS_WITH_BATTERY;
            batteryInUse[visionup] = true;
            break;
        }
    }
}


void DrawBatteries(Player* p, Camera2D camera, double time)
{
    bool batteryPickedUp = false;
    for (int i = bigjump; i < batterycount; i++)
    {
        if (!batteryInUse[i] && !batteryInDoor[i] && !batteryInShip[i])
        {
            if (TileOnScreen(batteryPositions[i] / TMSIZE, camera))
            {
                DrawTextureV(batteryTexture, batteryPositions[i] - (Vector2){5, 3}, batteryColor[i]);
                // DrawRectangleV(batteryPositions[i], {5.f, 8.f}, LIGHTGRAY);
            }
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
                    case visionup:
                    {
                        batteryname = "visionup";
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
int doorCount = 0;

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
        while (tilemap[(int)doorStartPos[i].x][(int)doorStartPos[i].y - 1 - numSpaces] == empty || tilemap[(int)doorStartPos[i].x][(int)doorStartPos[i].y - 1 - numSpaces] == sky)
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
            // printf("Dropped bigjump battery\n");
            p->jumpVel = NORMALJUMPVEL;
            p->maxJumps = 1;
            break;
        }
        case doublejump:
        {
            batteryInUse[doublejump] = false;
            batteryInDoor[doublejump] = true;
            batteryPositions[doublejump] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
            // printf("Dropped doublejump battery\n");
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
        case visionup:
        {
            batteryInUse[visionup] = false;
            batteryInDoor[visionup] = true;
            batteryPositions[visionup] = (Vector2){p->pos.x, p->pos.y - BATTERYDROPOFFSET};
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
            // DrawRectangleV(doorKeyPos[i] * TMSIZE, {8, 12}, LIGHTGRAY);
            DrawTexture(batterySlotTexture, doorKeyPos[i].x * TMSIZE, doorKeyPos[i].y * TMSIZE, WHITE);
            if (doorKeyholeHasBattery[i])
            { DrawTextureV(batteryTexture, doorKeyPos[i] * TMSIZE, batteryColor[doorKeyholebattery[i]]); }

            if (Vector2DistanceSqr(doorKeyPos[i] * TMSIZE, p->pos) < (INTERACTION_DISTANCE * INTERACTION_DISTANCE) && !didSomething)
            {
                if (!doorKeyholeHasBattery[i]) // if no battery already in
                {
                    if (showingMessage)
                    {
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

Rectangle GetPlayerRect(Player p)
{
    return {p.pos.x - p.size.x/2, p.pos.y - p.size.y/2, p.size.x, p.size.y};
}

/***********/
/* Enemies */
/***********/
#define MAX_RANGED_ENEMIES 200
Vector2 rangedEnemyPositions[MAX_RANGED_ENEMIES] = {};
bool rangedEnemyAlive[MAX_RANGED_ENEMIES] = {};
double rangedEnemyShootingCooldowns[MAX_RANGED_ENEMIES] = {};
#define RANGEDENEMYSHOOTINGCOOLDOWNSECONDS 2
Texture rangedEnemyTexture = {};
int rangedEnemyCount = 0;

void ShootEnemyBullet(Bullet bullettype, Vector2 fromPosition, Vector2 direction)
{
    bulletsTs[bulletCount] = bullettype;
    bulletsPs[bulletCount] = fromPosition;
    bulletsVs[bulletCount] = direction * bulletSpeed[bullettype];
    bulletCount++;
}

void RenderRangedEnemies(Camera2D camera)
{
    for (int i = 0; i < rangedEnemyCount; i++)
    {
        if (rangedEnemyAlive[i] && TileOnScreen(rangedEnemyPositions[i]/TMSIZE, camera))
        {
            DrawTexture(rangedEnemyTexture, rangedEnemyPositions[i].x, rangedEnemyPositions[i].y, WHITE);
        }
    }
}

void RangedEnemiesPhysicsProcess(Player* p)
{
    for (int i = 0; i < rangedEnemyCount; i++)
    {
        if (rangedEnemyAlive[i])
        {
            bool booleanvalue = Vector2DistanceSqr(p->pos, rangedEnemyPositions[i]) < (200 * 200);
            if (rangedEnemyShootingCooldowns[i] < EPSILON && booleanvalue)
            {
                ShootEnemyBullet(enemylightbullet, rangedEnemyPositions[i], Vector2Normalize(p->pos - rangedEnemyPositions[i]));
                rangedEnemyShootingCooldowns[i] = RANGEDENEMYSHOOTINGCOOLDOWNSECONDS;
            }
            else if (booleanvalue)
            {
                rangedEnemyShootingCooldowns[i] -= physicsDTs;
            }
        }
    }
}


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
    1,
    0,
};

float enemyMaxHealth[enemytypecount] =
{
    10,
    40,
    50,
};

Vector2 enemySize[enemytypecount] =
{
    {10, 16}, // light
    {10, 32}, // heavy
    {20, 20},
};

Color enemyColor[enemytypecount] =
{
    MAROON,
    GREEN,
    BLACK
};

int enemyScore[enemytypecount] =
{
    100,
    300,
    100,
};

Vector2 enemyOffsets[enemytypecount] =
{
    {8, 8},
    {8, 16},
};

#define ENEMYSPAWNCOOLDOWNSECONDS 300

double enemySpawnTimers[MAX_ENEMIES] = {};

// as tiles
Vector2 enemySpawnPoints[MAX_ENEMIES] = {};

// as position
Vector2 enemyPositions[MAX_ENEMIES] = {};
Vector2 enemyVelocities[MAX_ENEMIES] = {};
int enemyDirections[MAX_ENEMIES] = {};
int enemyHealth[MAX_ENEMIES] = {};
bool enemyDead[MAX_ENEMIES] = {};
Enemy enemyTypes[MAX_ENEMIES] = {};
int enemyCount = 0;

Texture enemyTextureLeft[enemytypecount] = { };
Texture enemyTextureRight[enemytypecount] = { };

void LoadEnemyTextures()
{
    enemyTextureLeft[regular] = LoadTexture("assets/lightenemy_left.png");
    enemyTextureRight[regular] = LoadTexture("assets/lightenemy_right.png");
    enemyTextureLeft[heavy] = LoadTexture("assets/heavyenemy_left.png");
    enemyTextureRight[heavy] = LoadTexture("assets/heavyenemy_right.png");
    rangedEnemyTexture = LoadTexture("assets/rangedenemy.png");
    specialItemTex = LoadTexture("assets/specialitem.png");
}

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

    // the commented code IS FUCKED AND KILLS FPS
    // kills fps? this should be enableable by the player
    // Check if colliding with the tilemap
    // {
    //     // // prevent enemy from walking on spikes
    //     // int minx = (enemyRect.x / TMSIZE) - 1;
    //     // int miny = (enemyRect.y / TMSIZE) - 1;
    //     // int maxx = (enemyRect.x + enemyRect.width / TMSIZE) + 1;
    //     // int maxy = (enemyRect.y + enemyRect.height / TMSIZE) + 1;

    //     // for (int y = miny; y <= maxy; y++)
    //     // {
    //     //     for (int x = minx; x <= maxx; x++)
    //     //     {
    //     //         if (x >= 0 && x < WORLD_SIZE_X && y >= 0 && y < WORLD_SIZE_Y)
    //     //         {
    //     //             if (tilemap[x][y] != empty)
    //     //             {
    //     //                 Rectangle tileRect = {(float)x * TMSIZE, (float)y * TMSIZE, TMSIZE, TMSIZE};
    //     //                 if (CheckCollisionRecs(enemyRect, tileRect))
    //     //                 {
    //     //                     return true;
    //     //                 }
    //     //             }
    //     //         }
    //     //     }
    //     // }

    // }

    // // Check if colliding with any solid in the scene, currently loops over every solid object
    // for (int i = 0; i < solidCount; i++)
    // {
    //     Solid* s = &solids[i];
    //     if (s->collideable)
    //     {
    //         Rectangle solidRect = {s->pos.x - s->size.x / 2, s->pos.y - s->size.y / 2, s->size.x, s->size.y};
    //         if (CheckCollisionRecs(enemyRect, solidRect))
    //         {
    //             return true;
    //         }
    //     }
    // }

    // // Check if colliding with any door
    // for (int j = 0; j < doorCount; j++)
    // {
    //     if (!doorOpen[j])
    //     {
    //         Rectangle doorRect = {doorStartPos[j].x * TMSIZE, doorEndPos[j].y * TMSIZE, DOORTHICKNESS, (doorStartPos[j].y - doorEndPos[j].y) * TMSIZE + TMSIZE};
    //         if (CheckCollisionRecs(enemyRect, doorRect))
    //         {
    //             return true;
    //         }
    //     }
    // }
    // Check if colliding with the tilemap
    int minx = (enemyRect.x / TMSIZE) - 1;
    int miny = (enemyRect.y / TMSIZE) - 1;
    int maxx = ((enemyRect.x + enemyRect.width) / TMSIZE) + 1;
    int maxy = ((enemyRect.y + enemyRect.height) / TMSIZE) + 1;

    for (int y = miny; y <= maxy; y++)
    {
        for (int x = minx; x <= maxx; x++)
        {
            if (x >= 0 && x < WORLD_SIZE_X && y >= 0 && y < WORLD_SIZE_Y)
            {
                if (tilemap[x][y] != empty && tilemap[x][y] != sky && tilemap[x][y] != spike)
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


void EnemyPhysicsProcess(Player* p, Solid* solids, int solidCount, double time)
{
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
                p->score += enemyScore[enemyTypes[i]];
                AddDisplayMessage(enemyPositions[i] - (Vector2){0, TMSIZE}, 1, TextFormat("+%d", enemyScore[enemyTypes[i]]));
                aliveEnemyCount--;
                continue;
            }

            // Apply gravity if enemy is in the air
            if (!EnemyCollidingAt(i, enemyPositions[i] + (Vector2){0, 1}, solids, solidCount))
            {
                enemyVelocities[i].y += GRAVITY * physicsDTs;
            }
            else
            {
                enemyVelocities[i].y = 0;
            }

            // Update enemy position

            if (EnemyCollidingAt(i, enemyPositions[i] + enemyVelocities[i], solids, solidCount))
            {
                enemyVelocities[i].x *= -1; // flip around
            }
            else
            { enemyPositions[i] = enemyPositions[i] + enemyVelocities[i]; }

            // Push players out of the way

            // Check if colliding with the player, if so, push the player
            AABB enemyAABB = {.max = enemyPositions[i] + (enemySize[enemyTypes[i]] / 2), .min = enemyPositions[i] - (enemySize[enemyTypes[i]] / 2)};
            AABB playerAABB = {.max = p->pos + (p->size / 2), .min = p->pos - (p->size / 2)};
            if (AABBsColliding(enemyAABB, playerAABB))
            {
                enemyDirections[i] *= -1;
                Vector2 pushDir = Vector2Normalize(p->pos - enemyPositions[i]);
                p->damageimpulse = pushDir * 5;// (Vector2){ENEMYHORIZONTALKNOCKBACKFORCE, 1.5}; // Adjust the push strength as needed
                p->health -= enemyHealth[i]; // Adjust the damage as needed
                p->lasthittaken = time;
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



                enemyVelocities[i] = {(float)((rand() % 2) ? 1 : -1) * ((rand() % 3) + 1), 0};
                enemyDead[i] = false;
                aliveEnemyCount++;
            }
        }
    }
}

void RenderEnemies(Camera2D camera)
{
    for (int i = 0; i < enemyCount; i++)
    {
        if (!enemyDead[i] && TileOnScreen(enemyPositions[i]/TMSIZE, camera))
        {
            if (enemyDirections[i] < 1)
            { DrawTexture(enemyTextureLeft[enemyTypes[i]], enemyPositions[i].x - enemyOffsets[enemyTypes[i]].x, enemyPositions[i].y - enemyOffsets[enemyTypes[i]].y, WHITE); }
            else { DrawTexture(enemyTextureRight[enemyTypes[i]], enemyPositions[i].x - enemyOffsets[enemyTypes[i]].x, enemyPositions[i].y - enemyOffsets[enemyTypes[i]].y, WHITE); }
            // debug info
            // DrawRectangleV(enemyPositions[i] - enemySize[enemyTypes[i]] / 2, enemySize[enemyTypes[i]], enemyColor[enemyTypes[i]] * 0.7);
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
    tileTex[cloud] = LoadTexture("assets/cloud.png");
    tileTex[platform] = LoadTexture("assets/platform.png");
    tileTex[empty] = LoadTexture("assets/empty.png");
    tileTex[sky] = LoadTexture("assets/background.png");
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
            if (y < 44) t = sky;
            char c;
            fscanf(fp, " %c", &c);
            switch (c)
            {
                case 's':
                {
                    player_spawnpoint = {(float)x * TMSIZE, (float)y * TMSIZE};
                    break;
                }
                case 'S':
                {
                    shipPosition = (Vector2){(float)x, (float)y} * TMSIZE + (int)TMSIZE/2;
                    Vector2 shipBatterySlotPositions[BATTERIESNEEDEDFORSHIP] = {};
                    break;
                }
                case '%':
                {
                    // boss
                    break;
                }
                case ',':
                {
                    t = sky;
                    break;
                }
                case 'M':
                {
                    maxHeight = y * TMSIZE;
                    break;
                }
                case 'm':
                {
                    minHeight = y * TMSIZE;
                    break;
                }
                case '.':
                {
                    break;
                }
                case 'r':
                {
                    rangedEnemyAlive[rangedEnemyCount] = true;
                    rangedEnemyShootingCooldowns[rangedEnemyCount] = 0;
                    rangedEnemyPositions[rangedEnemyCount] = {(float)x * TMSIZE, (float)y * TMSIZE};
                    rangedEnemyCount++;
                    break;
                }
                case 'P':
                {
                    t = platform;
                    break;
                }
                case 'C':
                {
                    t = cloud;
                    break;
                }
                case '#':
                {
                    t = stone;
                    break;
                }
                case '*':
                {
                    specialItemPos = {(float)x * TMSIZE, (float)y * TMSIZE};
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
                case '6':
                {
                    batteryPositions[6] = {(float)x * TMSIZE, (float)y * TMSIZE};
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
    2, // enemy light bulet
};

float bulletDamage[bulletcount] =
{
    0,
    5,
    12,
    5,
    5,
};

void SetBulletDamages()
{
    // float normalbulletdps = 3.5; idfk man
    // float lightbulletdps = 10;
    // float heavybulletdps = 10;
    // // bulletDamage[normalbullet] = normalbulletdps * bulletDelays[normalbullet];
    // bulletDamage[normalbullet] = 2; // fuck the other shit
    // bulletDamage[lightbullet] = (int)(lightbulletdps * bulletDelays[lightbullet]);
    // // bulletDamage[heavybullet] = (int)(heavybulletdps * bulletDelays[heavybullet]);
    // printf("normalbullet damage = %f\n", bulletDamage[normalbullet]);
    // printf("lightbullet damage = %f\n", bulletDamage[lightbullet]);
    // printf("heavybullet damage = %f\n", bulletDamage[heavybullet]);
}



Texture bulletTexture[bulletcount] =
{ };

void LoadBulletTextures()
{
    bulletTexture[lightbullet] = LoadTexture("assets/lightbullet.png");
    bulletTexture[heavybullet] = LoadTexture("assets/heavybullet.png");
    bulletTexture[normalbullet] = LoadTexture("assets/regularbullet.png");
    bulletTexture[enemylightbullet] = LoadTexture("assets/enemylightbullet.png");
}

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
        DrawTexture(bulletTexture[bulletsTs[i]], bulletsPs[i].x - bulletRadius[bulletsTs[i]], bulletsPs[i].y - bulletRadius[bulletsTs[i]], WHITE);
    }
}

void BulletPhysicsProcess(Player* p, double time)
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
                        if (tilemap[x][y] != empty && tilemap[x][y] != sky && tilemap[x][y] != cloud && tilemap[x][y] != platform)
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
                                    case spike:
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


            if (bulletsTs[i] != enemylightbullet)
            {
                // enemy collisions
                for (int j = 0; j < enemyCount; j++)
                {
                    if (!enemyDead[j])
                    {
                        Rectangle enemyRect = {enemyPositions[j].x - enemySize[enemyTypes[j]].x / 2, enemyPositions[j].y - enemySize[enemyTypes[j]].y / 2, enemySize[enemyTypes[j]].x, enemySize[enemyTypes[j]].y};
                        if (CheckCollisionCircleRec(bulletsPs[i], bulletRadius[bulletsTs[i]], enemyRect))
                        {
                            enemyHealth[j] -= bulletDamage[bulletsTs[i]];
                            KillBullet(i);
                            break;
                        }
                    }
                }

                // ranged enemy collisions
                for (int j = 0; j < rangedEnemyCount; j++)
                {
                    Rectangle rangedEnemyRect = {rangedEnemyPositions[j].x, rangedEnemyPositions[j].y, 16, 16};
                    if (CheckCollisionCircleRec(bulletsPs[i], bulletRadius[bulletsTs[i]], rangedEnemyRect))
                    {
                        rangedEnemyAlive[j] = false;
                        KillBullet(i);
                    }
                }
            }

            if (bulletsTs[i] == enemylightbullet && CheckCollisionCircleRec(bulletsPs[i], bulletRadius[bulletsTs[i]], GetPlayerRect(*p)))
            {
                // we (the enemy bullet) are colliding with the player
                p->damageimpulse = Vector2Normalize(bulletsVs[i]) * 5;
                p->lastBulletHitTaken = time;
                p->lasthittaken = time;
                KillBullet(i);
            }
        }
    }
}

int ReadTopScoreFromFile()
{
    FILE* fp = fopen("topscores.txt", "r+");
    if (fp == NULL)
    {
        return 1;
    }
    fscanf(fp, "%d", &maxscore);
    fclose(fp);

    fp = fopen("timeswon.txt", "r+");
    if (fp == NULL)
    {
        return 1;
    }
    fscanf(fp, "%d", &timeswon);
    fclose(fp);

    fp = fopen("batteriesfound.txt", "r+");
        if (fp == NULL)
        {
            return 1;
        }
        fscanf(fp, "%d", &maxbatteriesfound);
        fclose(fp);

    return 0;
}

int WriteTopScoreToFile()
{
    FILE* fp = fopen("topscores.txt", "w+");
    if (fp == NULL)
    {
        return 1;
    }
    fprintf(fp, "%d", maxscore);
    fclose(fp);

    fp = fopen("timeswon.txt", "w+");
    if (fp == NULL)
    {
        return 1;
    }
    fprintf(fp, "%d", timeswon);
    fclose(fp);

    fp = fopen("batteriesfound.txt", "w+");
    if (fp == NULL)
    {
        return 1;
    }
    fprintf(fp, "%d", maxbatteriesfound);
    fclose(fp);
    return 0;
}

/***************/
/* Player Stuff */
/***************/
bool newtopscoreflag = 0;

void PlayerDie(Player* p)
{
    gamestate = gameover;
    if (p->score > maxscore)
    {
        newtopscoreflag = 1;
        maxscore = p->score;
    }
    if (batteriesFound > maxbatteriesfound) maxbatteriesfound = batteriesFound;
}

void DrawPlayer(Player p)
{
    if (p.lastDir.x > 0)
    { DrawTexture(p.right_tex, p.pos.x - p.size.x/2 - 2, p.pos.y - p.size.y/2, p.tint); }
    else if (p.lastDir.x < 0)
    { DrawTexture(p.left_tex, p.pos.x - p.size.x/2 - 3.5, p.pos.y - p.size.y/2, p.tint); }

    // DrawRectangleV(p.pos - (p.size / 2), p.size, p.col);
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
                    if (tilemap[x][y] == platform)
                    {
                        AABB taabb = {.min = (Vector2){(float)x * TMSIZE, (float)y * TMSIZE}, .max = (Vector2){(float)x * TMSIZE + TMSIZE, (float)y * TMSIZE + 3}};
                        if (AABBsColliding(aabb, taabb))
                        {
                            return true;
                        }
                    }
                    else if (tilemap[x][y] != empty && tilemap[x][y] != sky)
                    {
                        AABB taabb = {.min = (Vector2){(float)x * TMSIZE, (float)y * TMSIZE}, .max = (Vector2){(float)x * TMSIZE + TMSIZE, (float)y * TMSIZE + TMSIZE}};
                        if (AABBsColliding(aabb, taabb))
                        {
                            if (tilemap[x][y] == spike)
                            { a->standingOnSpike = true; }
                            return true;
                        }
                    }
                }
            }
        }
    }

    // Check if colliding with any solid in the scene, currently loosp over every solid
    // i dont have any fucking solids
    // for (int i = 0; i < solidCount; i++)
    // {
    //     Solid* s = &solids[i];
    //     if (s->collideable && AABBsColliding(aabb, s->aabb))
    //     {
    //         return true;
    //     }
    // }

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

#define COYOTE_S 0.1
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
        p->persistentVel.x = DOUBLEJUMPSIDEWAYSBONUSVELMULTIPLIER;
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
            PlayerDie(p);
        }
        else // alive
        {

        }
    }

    p->standingOnSpike = false;
    // Still on Ground check
    {
        // if we move the player down EPSILON, would they be colliding? if so, they're on ground
        // if we move down a bit, and aren't colliding, then we aren't grounded
        // if we move down a bit, are are oclliding, we're on the ground
        // instead of epsilon we need to do 1, since we have pixel movement
        if (PlayerCollidingAt(p, {p->pos.x, p->pos.y + 1}, solids, solidCount))
        {
            p->grounded = true;
            p->persistentVel = {0};
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
    if (p->persistentVel.x != 0) p->vel.x *= p->persistentVel.x;

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
        {
            if (p->bulletHitNotAccountedFor && time - p->lastBulletHitTaken > DAMAGETINTDURATION)
            {
                p->health -= bulletDamage[p->lastBulletHitType];
                p->bulletHitNotAccountedFor = false;
            }
        }
    }
    // Impulse logic (not actual impulse idk what that is)
    {
        p->vel.x = p->vel.x + p->damageimpulse.x;
        if (p->damageimpulse.y != 0) p->vel.y = p->damageimpulse.y;
        p->damageimpulse = p->damageimpulse - p->damageimpulse * physicsDTs;
        if (p->damageimpulse.x < 0 + EPSILON) p->damageimpulse.x = 0;
        if (p->damageimpulse.y < 0 + EPSILON) p->damageimpulse.y = 0;
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

Player p = {};
Camera2D camera;
double physicsAccumulator = 0;
Solid* solids = (Solid*)malloc(sizeof(Solid) * MAX_SOLID_COUNT);
int solidCount = 0;

/*****************************************************************************************************************************/
/* Basic lighting code (change later maybe to add fog, and have bullets clear parts of the fog like in 20 minutes till dawn) */
/*****************************************************************************************************************************/
 enum VisibilityType {
    NOT_VISIBLE,
    VISIBLE,
    BEHIND_WALL
};

VisibilityType visibilityGrid[WORLD_SIZE_X][WORLD_SIZE_Y];
Tile wallTypeGrid[WORLD_SIZE_X][WORLD_SIZE_Y];

void CalculatePlayerFOV(Player* player)
{
    // Clear previous visibility
    for (int y = 0; y < WORLD_SIZE_Y; y++)
        for (int x = 0; x < WORLD_SIZE_X; x++)
        {
            visibilityGrid[x][y] = NOT_VISIBLE;
            wallTypeGrid[x][y] = empty;
        }

    for (int i = 0; i < RAY_COUNT; i++)
    {
        float angle = (float)i * 2 * PI / RAY_COUNT;
        float dx = cos(angle);
        float dy = sin(angle);

        float x = player->pos.x / TMSIZE;
        float y = player->pos.y / TMSIZE;

        bool hitWall = false;
        Tile wallType = empty;

        for (float j = 0; j < FOV_MAX_RADIUS; j += 0.5f)
        {
            int gridX = (int)x;
            int gridY = (int)y;

            if (gridX >= 0 && gridX < WORLD_SIZE_X && gridY >= 0 && gridY < WORLD_SIZE_Y)
            {
                if (!hitWall)
                {
                    if (tilemap[gridX][gridY] == empty || tilemap[gridX][gridY] == sky || tilemap[gridX][gridY] == platform || tilemap[gridX][gridY] == spike)
                    {
                        if (j < p.FOV)
                        {
                            visibilityGrid[gridX][gridY] = VISIBLE;
                        }
                    }
                    else
                    {
                        hitWall = true;
                        wallType = tilemap[gridX][gridY];
                        if (j < p.FOV) visibilityGrid[gridX][gridY] = VISIBLE;  // The wall itself is visible
                        wallTypeGrid[gridX][gridY] = wallType;
                    }
                }
                else
                {
                    visibilityGrid[gridX][gridY] = BEHIND_WALL;
                    wallTypeGrid[gridX][gridY] = wallType;
                }
            }

            x += dx;
            y += dy;
        }
    }
}

void RenderEnemiesWithLighting(Camera2D camera)
{
    for (int i = 0; i < enemyCount; i++)
    {
        if (!enemyDead[i] && TileOnScreen(enemyPositions[i]/TMSIZE, camera))
        {
            int gridX = enemyPositions[i].x / TMSIZE;
            int gridY = enemyPositions[i].y / TMSIZE;
            Color tint = WHITE;

            if (visibilityGrid[gridX][gridY] == NOT_VISIBLE)
            {
                tint = (Color){(unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), 255};
            }
            else if (visibilityGrid[gridX][gridY] == BEHIND_WALL)
            {
                continue; // Don't render enemies behind walls
            }

            if (enemyDirections[i] < 1)
            {
                DrawTexture(enemyTextureLeft[enemyTypes[i]],
                            enemyPositions[i].x - enemyOffsets[enemyTypes[i]].x,
                            enemyPositions[i].y - enemyOffsets[enemyTypes[i]].y,
                            tint);
            }
            else
            {
                DrawTexture(enemyTextureRight[enemyTypes[i]],
                            enemyPositions[i].x - enemyOffsets[enemyTypes[i]].x,
                            enemyPositions[i].y - enemyOffsets[enemyTypes[i]].y,
                            tint);
            }
        }
    }
    for (int i = 0; i < rangedEnemyCount; i++)
    {
        if (rangedEnemyAlive[i] && TileOnScreen(rangedEnemyPositions[i] / TMSIZE, camera))
        {
            int gridX = rangedEnemyPositions[i].x / TMSIZE;
            int gridY = rangedEnemyPositions[i].y / TMSIZE;
            Color tint = WHITE;

            if (visibilityGrid[gridX][gridY] == NOT_VISIBLE)
            {
                tint = (Color){(unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), 255};
            }
            else if (visibilityGrid[gridX][gridY] == BEHIND_WALL)
            {
                continue; // Don't render enemies behind walls
            }

            DrawTexture(rangedEnemyTexture,
                        rangedEnemyPositions[i].x,
                        rangedEnemyPositions[i].y,
                        tint);
        }
    }
}

void RenderDoorsWithLighting(Camera2D camera, Player* p)
{
    bool didSomething = false;
    for (int i = 0; i < doorCount; i++)
    {
        doorOpen[i] = doorKeyholeHasBattery[i];
        int gridX = doorStartPos[i].x;
        int gridY = doorStartPos[i].y;

        if (visibilityGrid[gridX][gridY] != NOT_VISIBLE && TileOnScreen(doorStartPos[i], camera)) {
            Color doorColor = doorOpen[i] ? GREEN : RED;
            if (visibilityGrid[gridX][gridY] == BEHIND_WALL) {
                doorColor = (Color){(unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), 255};
            }
            DrawLineEx(doorStartPos[i] * TMSIZE + (Vector2){DOORTHICKNESS/2., TMSIZE},
                       doorEndPos[i] * TMSIZE + (Vector2){DOORTHICKNESS/2., 0},
                       DOORTHICKNESS, doorColor);
        }

        // Render keyhole and interaction logic
        // previous implementation, but check visibility before rendering
        if (TileOnScreen(doorKeyPos[i], camera))
        {
            int keyX = doorKeyPos[i].x;
            int keyY = doorKeyPos[i].y;

            if (visibilityGrid[keyX][keyY] != NOT_VISIBLE)
            {
                Color keyholeTint = (visibilityGrid[keyX][keyY] == BEHIND_WALL) ?
                    (Color){(unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), 255} : WHITE;

                DrawTexture(batterySlotTexture, doorKeyPos[i].x * TMSIZE, doorKeyPos[i].y * TMSIZE, keyholeTint);
                if (doorKeyholeHasBattery[i])
                {
                    DrawTextureV(batteryTexture, doorKeyPos[i] * TMSIZE, ColorAlpha(batteryColor[doorKeyholebattery[i]], keyholeTint.a / 255.0f));
                }

                if (Vector2DistanceSqr(doorKeyPos[i] * TMSIZE, p->pos) < (INTERACTION_DISTANCE * INTERACTION_DISTANCE) && !didSomething)
                {
                    // Interaction logic (previous implementation)
                    if (!doorKeyholeHasBattery[i])
                    {
                        if (showingMessage)
                        {
                            const char* text = "Press a key to insert a battery: ";
                            if (batteryInUse[bigjump]) text = TextFormat("%s'1' bigjump ", text);
                            if (batteryInUse[doublejump]) text = TextFormat("%s'2' doublejump ", text);
                            if (batteryInUse[rapidfire]) text = TextFormat("%s'3' rapidfire ", text);
                            if (batteryInUse[heavyfire]) text = TextFormat("%s'4' heavyfire ", text);
                            if (batteryInUse[quickie]) text = TextFormat("%s'5' quickie ", text);
                            if (batteryInUse[tanky]) text = TextFormat("%s'6' tanky ", text);
                            if (batteryInUse[visionup]) text = TextFormat("%s'7' visionup ", text);
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
                                else if (IsKeyPressed(KEY_SEVEN) && batteryInUse[visionup]) {
                                    PlaceBatteryIntoKeyhole(p, visionup, i);
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
                    else
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
}

void RenderBatteriesWithLighting(Player* p, Camera2D camera, double time)
{
    for (int i = bigjump; i < batterycount; i++)
    {
        if (!batteryInUse[i] && !batteryInDoor[i] && !batteryInShip[i])
        {
            int gridX = batteryPositions[i].x / TMSIZE;
            int gridY = batteryPositions[i].y / TMSIZE;

            if (visibilityGrid[gridX][gridY] != NOT_VISIBLE && TileOnScreen(batteryPositions[i] / TMSIZE, camera))
            {
                Color tint = (visibilityGrid[gridX][gridY] == BEHIND_WALL) ?
                    (Color){(unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), 255} : WHITE;
                DrawTextureV(batteryTexture, batteryPositions[i] - (Vector2){5, 3}, ColorAlpha(batteryColor[i], tint.a / 255.0f));
            }

            // Interaction logic
            // previous implementation
            if (Vector2DistanceSqr(batteryPositions[i], p->pos) < (INTERACTION_DISTANCE * INTERACTION_DISTANCE))
            {
                batteryCanBePickedUp[i] = true;
                const char* batteryname;
                switch (i)
                {
                    case bigjump:
                        batteryname = "bigjump";
                        break;
                    case doublejump:
                        batteryname = "doublejump";
                        break;
                    case rapidfire:
                        batteryname = "rapidfire";
                        break;
                    case heavyfire:
                        batteryname = "heavyfire";
                        break;
                    case quickie:
                        batteryname = "quickie";
                        break;
                    case tanky:
                        batteryname = "tanky";
                        break;
                    case visionup:
                        batteryname = "visionup";
                        break;
                }
                const char* string = TextFormat("Press 'E' to pickup %s battery\0", batteryname);

                int stringsize = MeasureText(string, 10);
                DrawText(string, batteryPositions[i].x - stringsize/2.f, batteryPositions[i].y - TMSIZE, 10, WHITE);

                if (IsKeyPressed(KEY_E))
                {
                    PickupBattery(p, (Battery)i);
                }
            }
            else
            {
                batteryCanBePickedUp[i] = false;
            }
        }
    }
}

void RenderShipWithLighting(Camera2D camera, Player* p)
{
    int gridX = shipPosition.x / TMSIZE;
    int gridY = shipPosition.y / TMSIZE;

    if (visibilityGrid[gridX][gridY] != NOT_VISIBLE)
    {
        Color tint = (visibilityGrid[gridX][gridY] == BEHIND_WALL) ?
            BLANK : WHITE;
        // (Color){(unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), 255} : WHITE;
        DrawTexture(shipTexture, shipPosition.x - 64, shipPosition.y - 40, tint);

        // Render battery slots
        // previous implementation, but use tint
        for (int i = 0; i < BATTERIESNEEDEDFORSHIP; i++) {
            if (shipBatterySlotHasBattery[i]) {
                Battery battery = shipBatterySlotBatteryType[i];
                Vector2 slotPosition = {shipPosition.x + (i - 1.5f) * TMSIZE, shipPosition.y - 1.5f * TMSIZE};
                DrawTexture(batteryTexture, slotPosition.x, slotPosition.y, ColorAlpha(batteryColor[battery], tint.a / 255.0f));
            }
        }
    }
    else if (Vector2DistanceSqr(p->pos, shipPosition) < (256 * 256))
    {
        Color tint = (Color){(unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), (unsigned char)(depthfactor * 255), 255};
        DrawTexture(shipTexture, shipPosition.x - 64, shipPosition.y - 40, tint);

        // Render battery slots
        // previous implementation, but use tint
        for (int i = 0; i < BATTERIESNEEDEDFORSHIP; i++) {
            if (shipBatterySlotHasBattery[i]) {
                Battery battery = shipBatterySlotBatteryType[i];
                Vector2 slotPosition = {shipPosition.x + (i - 1.5f) * TMSIZE, shipPosition.y - 1.5f * TMSIZE};
                DrawTexture(batteryTexture, slotPosition.x, slotPosition.y, ColorAlpha(batteryColor[battery], tint.a / 255.0f));
            }
        }
    }

    // Interaction logic
    // previous implementation
    bool didSomething = false;
    if (shipOpen && Vector2DistanceSqr(p->pos, shipPosition) < (SHIPINTERACTIONDISTANCE * SHIPINTERACTIONDISTANCE))
    {
        const char* text = "Press E to board the ship\0";
        int offset = MeasureText(text, GAMEWINSHIPFONTSIZE)/2;
        DrawText(text, shipPosition.x - offset, shipPosition.y - TMSIZE, GAMEWINSHIPFONTSIZE, GOLD);

        if (IsKeyPressed(KEY_E) && foundSpecialItem)
        {
            gamestate = win;
            timeswon++;
        }
        else if (IsKeyPressed(KEY_E))
        {
            AddDisplayMessage(shipPosition - (Vector2){0, TMSIZE * 3}, 3,  "I can't leave without my special item\0");
            showingShipPlaceBatteryMessage = false;
            showingShipBatterySlotSelectionMessage = false;
            showingBatterySlotRemovalMessage = false;
        }
    }
    else if (Vector2DistanceSqr(p->pos, shipPosition + (Vector2){TMSIZE, 0}) < (SHIPINTERACTIONDISTANCE * SHIPINTERACTIONDISTANCE))
    {
        if (showingShipBatterySlotSelectionMessage)
        {
            const char* text = "Which slot would you like to place a battery into? \n '1' '2' '3' '4'\0";
            int textOffset = MeasureText(text, thisfontsize)/2;
            DrawText(text, p->pos.x - textOffset, p->pos.y - TMSIZE * 3, thisfontsize, WHITE);

            if (IsKeyPressed(KEY_ONE) && !didSomething)
            {
                shipSlot = 0;
                didSomething = true;

                if (shipBatterySlotHasBattery[shipSlot])
                {
                    showingBatterySlotRemovalMessage = true;
                    showingShipBatterySlotSelectionMessage = false;
                }
                else
                {
                    showingShipBatterySlotSelectionMessage = false;
                    showingShipPlaceBatteryMessage = true;
                }
            }
            else if (IsKeyPressed(KEY_TWO) && !didSomething)
            {
                shipSlot = 1;
                didSomething = true;

                if (shipBatterySlotHasBattery[shipSlot])
                {
                    showingBatterySlotRemovalMessage = true;
                    showingShipBatterySlotSelectionMessage = false;
                }
                else
                {
                    showingShipBatterySlotSelectionMessage = false;
                    showingShipPlaceBatteryMessage = true;
                }
            }
            else if (IsKeyPressed(KEY_THREE) && !didSomething)
            {
                shipSlot = 2;
                didSomething = true;

                if (shipBatterySlotHasBattery[shipSlot])
                {
                    showingBatterySlotRemovalMessage = true;
                    showingShipBatterySlotSelectionMessage = false;
                }
                else
                {
                    showingShipBatterySlotSelectionMessage = false;
                    showingShipPlaceBatteryMessage = true;
                }
            }
            else if (IsKeyPressed(KEY_FOUR) && !didSomething)
            {
                shipSlot = 3;
                didSomething = true;

                if (shipBatterySlotHasBattery[shipSlot])
                {
                    showingBatterySlotRemovalMessage = true;
                    showingShipBatterySlotSelectionMessage = false;
                }
                else
                {
                    showingShipBatterySlotSelectionMessage = false;
                    showingShipPlaceBatteryMessage = true;
                }
            }
        }
        else if (showingShipPlaceBatteryMessage)
        {
            const char* text = "Press a key to insert a battery: \n";
            if (batteryInUse[bigjump]) text = TextFormat("%s'1' bigjump ", text);
            if (batteryInUse[doublejump]) text = TextFormat("%s'2' doublejump ", text);
            if (batteryInUse[rapidfire]) text = TextFormat("%s'3' rapidfire ", text);
            if (batteryInUse[heavyfire]) text = TextFormat("%s'4' heavyfire ", text);
            if (batteryInUse[quickie]) text = TextFormat("%s'5' quickie ", text);
            if (batteryInUse[tanky]) text = TextFormat("%s'6' tanky ", text);
            int atleastone = (batteryInUse[bigjump] || batteryInUse[doublejump] || batteryInUse[rapidfire] || batteryInUse[heavyfire] || batteryInUse[quickie] || batteryInUse[tanky]);
            if (!atleastone) text = "You need a battery to do this!";
            int textOffset = MeasureText(text, INSERTBATTERYFONTSIZE)/2;
            DrawText(text, p->pos.x - textOffset, p->pos.y - TMSIZE * 4, thisfontsize, WHITE);

            if (atleastone)
            {
                if (IsKeyPressed(KEY_ONE) && batteryInUse[bigjump])
                {
                    PlaceBatteryIntoShipKeyhole(p, bigjump, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_TWO) && batteryInUse[doublejump])
                {
                    PlaceBatteryIntoShipKeyhole(p, doublejump, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_THREE) && batteryInUse[rapidfire])
                {
                    PlaceBatteryIntoShipKeyhole(p, rapidfire, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_FOUR) && batteryInUse[heavyfire])
                {
                    PlaceBatteryIntoShipKeyhole(p, heavyfire, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_FIVE) && batteryInUse[quickie])
                {
                    PlaceBatteryIntoShipKeyhole(p, quickie, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
                else if (IsKeyPressed(KEY_SIX) && batteryInUse[tanky])
                {
                    PlaceBatteryIntoShipKeyhole(p, tanky, shipSlot);
                    showingShipPlaceBatteryMessage = false;
                    didSomething = true;
                    shipSlot = 0;
                }
            }
        }
        else if (showingBatterySlotRemovalMessage)
        {
            const char* text = "This battery slot already has a battery\0";
            const char* subtitle = "Press E to pick it up\0";
            int textoffset = MeasureText(text, thisfontsize)/2;
            int subtextoffset = MeasureText(subtitle, thisfontsize)/2;
            DrawText(text, p->pos.x - textoffset, p->pos.y - TMSIZE * 3, thisfontsize, WHITE);
            DrawText(subtitle, p->pos.x - subtextoffset, p->pos.y - TMSIZE * 2, thisfontsize, WHITE);

            if (IsKeyPressed(KEY_E) && !didSomething)
            {
                PickupBatteryFromShipKeyhole(p, shipSlot);
                didSomething = true;
                showingBatterySlotRemovalMessage = false;
            }
        }
        else
        {
            const char* text = "Press E to place a battery\0";
            int textOffset = MeasureText(text, thisfontsize)/2;
            DrawText(text, p->pos.x - textOffset, p->pos.y - TMSIZE * 3, thisfontsize, WHITE);

            if (IsKeyPressed(KEY_E) && !didSomething)
            {
                showingShipBatterySlotSelectionMessage = true;
                didSomething = true;
            }
        }
    }
    else if (showingShipPlaceBatteryMessage || showingShipBatterySlotSelectionMessage || showingBatterySlotRemovalMessage)
    {
        showingShipPlaceBatteryMessage = false;
        showingShipBatterySlotSelectionMessage = false;
        showingBatterySlotRemovalMessage = false;
    }
}

void DrawTilemapWithLighting(Player* player)
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
                Tile tileToRender;
                Color tileColor;

                switch(visibilityGrid[x][y])
                {
                    case VISIBLE:
                        tileToRender = tilemap[x][y];
                        tileColor = WHITE;
                        break;
                    case BEHIND_WALL:
                        tileToRender = wallTypeGrid[x][y];
                        tileColor = (Color){(unsigned char)(depthfactor * 255),
                                            (unsigned char)(depthfactor * 255),
                                            (unsigned char)(depthfactor * 255), 255};
                        break;
                    case NOT_VISIBLE:
                    default:
                        tileToRender = tilemap[x][y];
                        tileColor = (Color){(unsigned char)(depthfactor * 255),
                                            (unsigned char)(depthfactor * 255),
                                            (unsigned char)(depthfactor * 255), 255};
                        break;
                }

                if (tileToRender == platform || tileToRender == spike)
                {
                    Color tempColor = tileColor;
                    tileColor = (Color){(unsigned char)(depthfactor * 255),
                                        (unsigned char)(depthfactor * 255),
                                        (unsigned char)(depthfactor * 255), 255};
                    if (y < 44) DrawTexture(tileTex[sky], x * TMSIZE, y * TMSIZE, tileColor);
                    else { DrawTexture(tileTex[empty], x * TMSIZE, y * TMSIZE, tileColor); }
                    DrawTexture(tileTex[tileToRender], x * TMSIZE, y * TMSIZE, tempColor);
                }
                else if (tileToRender != empty && tileToRender != sky)
                {
                    DrawTexture(tileTex[tileToRender], x * TMSIZE, y * TMSIZE, tileColor);
                }
                else
                {
                    tileColor = (Color){(unsigned char)(depthfactor * 255),
                                        (unsigned char)(depthfactor * 255),
                                        (unsigned char)(depthfactor * 255), 255};
                    DrawTexture(tileTex[tileToRender], x * TMSIZE, y * TMSIZE, tileColor);
                }
            }
        }
    }
}

void DrawBulletsWithLighting()
{
    for (int i = 0; i < bulletCount; i++)
    {
        int gridX = bulletsPs[i].x / TMSIZE;
        int gridY = bulletsPs[i].y / TMSIZE;
        if (gridX >= 0 && gridX < WORLD_SIZE_X && gridY >= 0 && gridY < WORLD_SIZE_Y)
        {
            Color tint = (visibilityGrid[gridX][gridY] == NOT_VISIBLE) ? (Color){(unsigned char)(depthfactor * 100), (unsigned char)(depthfactor * 100), (unsigned char)(depthfactor * 100), 255} : WHITE;
            DrawTexture(bulletTexture[bulletsTs[i]],
                        bulletsPs[i].x - bulletRadius[bulletsTs[i]],
                        bulletsPs[i].y - bulletRadius[bulletsTs[i]],
                        tint);
        }
    }
}

void RenderGameWithLighting(Player* player, Camera2D camera, double time)
{
    BeginMode2D(camera);

    // Draw tilemap with lighting
    DrawTilemapWithLighting(player);

    // Draw other game elements
    RenderDoorsWithLighting(camera, player);
    RenderBatteriesWithLighting(player, camera, time);
    RenderShipWithLighting(camera, player);
    RenderEnemiesWithLighting(camera);

    // Draw bullets
    DrawBulletsWithLighting();

    // Draw player
    DrawPlayer(*player);

    // Draw any other elements that should always be visible
    RenderDisplayMessages();
    RenderSpecialItem(player, camera);

    EndMode2D();
}

/****************/
/* Game loading */
/****************/
void LoadGame()
{
    {
        printf("can i see this? \n");
        for (int i = 0; i < BATTERIESNEEDEDFORSHIP; i++) {
            shipBatterySlotHasBattery[i] = false;
            shipBatterySlotBatteryType[i] = bigjump;
        }
        for (int i = 0; i < batterycount; i++) {
            batteryInUse[i] = false;
            batteryInDoor[i] = false;
            batteryInShip[i] = false;
            batteryCanBePickedUp[i] = false;
            batteryPositions[i] = {0};
        }
        p = {};
        camera = {};
        physicsAccumulator = 0;
        for (int i = 0; i < MAX_SOLID_COUNT; i++) {
            solids[i] = {};
        }
        solidCount = 0;

        displayMessagesCount = 0;
        showingMessageIndex = 0;
        showingMessage = false;
        for (int i = 0; i < batterycount; i++)
        {
            batteryInUse[i] = false;
            batteryInDoor[i] = false;
            batteryCanBePickedUp[i] = false;
            batteryPositions[i] = {};
            foundBattery[i] = false;
        }
        batteriesFound = 0;
        for (int i = 0; i < MAX_DISPLAY_MESSAGES; i++)
        {
            displayMessagesTimers[i] = 0;
            displayMessagesPositions[i] = {};
            memset(displayMessages[i], 0, MAX_DISPLAYMESSAGE_SIZE);
        }
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            enemySpawnTimers[i] = 0;
            enemySpawnPoints[i] = {};
            enemyPositions[i] = {};
            enemyDirections[i] = 0;
            enemyHealth[i] = 0;
            enemyDead[i] = true;
            enemyTypes[i] = regular;
        }
        for (int i = 0; i < MAXDOORS; i++)
        {
            doorStartPos[i] = {};
            doorEndPos[i] = {};
            doorOpen[i] = false;
            doorKeyPos[i] = {};
            doorKeyholeHasBattery[i] = false;
            doorKeyholebattery[i] = bigjump;
        }
        doorCount = 0;
        doorloadcount = 0;
        doorkeycount = 0;
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            bulletsTs[i] = none;
            bulletsVs[i] = {};
            bulletsPs[i] = {};
        }
        bulletCount = 0;
        shipBatteryCount = 0;
        showingShipBatterySlotSelectionMessage = false;
        showingShipPlaceBatteryMessage = false;
        shipSlot = 0;
        specialItemPos = {0};
        foundSpecialItem = false;
        rangedEnemyCount = 0;
        for (int i = 0; i < MAX_RANGED_ENEMIES; i++) {
            rangedEnemyPositions[i] = {};
            rangedEnemyAlive[i] = false;
        }
        shipOpen = false;
        showingBatterySlotRemovalMessage = false;
        enemyCount = 0;
    }

    printf("do we make it before the loadtilemap and doors?? \n");
    LoadTilemap("assets/map.txt");
    LoadDoors();
    printf("How about after?? \n");

    SetEnemyStatsProperBecauseInitValueNotWorking();

    { // loading screen stuff
        // Texture loadingbackground = LoadTexture("loadingbackground.png");
    }

    SetBulletDamages();
    // for (int i = 0; i < enemyCount; i++)
    // {
        // printf("Enemy %d: Direction = %d\n", i, enemyDirections[i]);
    // }

    {
        p.pos = player_spawnpoint;
        p.FOV = DEFAULT_FOV_RADIUS;
        p.size = {9, 14};
        p.lastDir = {1, 0};
        p.col = (Color){0xff, 0xff, 0x00, 50};
        p.aabb.max = p.pos + (p.size/2);
        p.aabb.min = p.pos - (p.size/2);
        p.speed = NORMALSPEED;
        p.jumpVel = NORMALJUMPVEL;
        p.persistentVel = {};
        p.doubleJumpVel = DOUBLEJUMPVEL;
        p.maxJumps = 1;
        p.bullettype = normalbullet; // none = normal
        p.shootDelay = bulletDelays[p.bullettype];
        p.maxHealth = NORMALMAXHEALTH;
        p.health = p.maxHealth;
        p.tint = WHITE;

        p.right_tex = LoadTexture("assets/player_right.png");
        p.left_tex = LoadTexture("assets/player_left.png");
    }

    {
        camera.zoom = 4;
        camera.rotation = 0;
        camera.offset = {windowWidth / 2.f, windowHeight / 2.f + 90};
        camera.target = {0};
    }

    {
        specialCamera.zoom = 1;
        specialCamera.rotation = 0;
        specialCamera.offset = {windowWidth / 2.f, windowHeight / 2.f};
        camera.target = {};
    }
    printf("if we make it here its not this functions fault. \n");
    // solids[solidCount++] = MakeSolid(640, 720, 1280, 256, GRAY, true);
    // solids[solidCount++] = MakeSolid(640, 0, 1280, 200, GRAY, false);

}
