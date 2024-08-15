#include "raylib.h"
#include <math.h>
#include <sys/_types/_null.h>

Rectangle solids[512];

typedef struct Actor {
    Rectangle rect;
    Vector2 vel;
} Actor;

Actor Player = {
    .rect = {.x = 20.f, .y = 20.f, .width = 20.f, .height = 50.f},
    .vel = {0},
};

Vector2 _Position = {0, 0};

float xRemainder = 0;
float yRemainder = 0;

int Sign(int n)
{
    // wonky-ass ternary operator
    // equivalent to if (n > 0) return 1; else return -1
    return (n > 0) ? 1 : -1;
}

Vector2 vec2add(Vector2 a, Vector2 b)
{
    return (Vector2){a.x + b.x, a.y + b.y};
}

bool collideAt(Rectangle *solids[], Vector2 point)
{
    return true;
}

void MovePlayerX(float amount)
{
    xRemainder += amount; // how much to move
    int move = roundf(xRemainder); // the amount to move in pixels
    if (move != 0) // if we are moving any amount of pixels
    {
        xRemainder -= move; // take away however many pixels we are moving from how many we need to move
        int sign = Sign(move);
        while (move != 0)
        {
            if (!collideAt(solids, vec2add(_Position, (Vector2){sign, 0})))
            {
                // there is nothing stopping us from moving in that direction
                _Position.x += sign;
                move -= sign;
            }
            else // hit something / something in the way
            {
                DrawText("Something in the way", 0, 0, 50, BLACK);
                // HandleCollision();
            }
        }
    }
}
