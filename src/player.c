#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include <stdio.h>
#include <stdlib.h>

#define GRAVITY 9.81
#define GRAVITY_SCALE_FACTOR 150.0

static Vector2 _Position = {.x = 0.f, .y = 0.f};
static Vector2 _Velocity = {.x = 0.f, .y = 0.f};
static Vector2 _Size = {.x = 25.f, .y = 50.f};

static float _Speed = 300.f;
static float _JumpForce = 600.f;

void AddGravity()
{
    _Velocity.y += GRAVITY * GRAVITY_SCALE_FACTOR * GetFrameTime();
}

void KeepOnScreen()
{
    AddGravity();

    int Width = GetScreenWidth();
    int Height = GetScreenHeight();
    if (_Position.x > Width - _Size.x) _Position.x = Width - _Size.x;
    if (_Position.y > Height - _Size.y)
    {
        _Position.y = Height - _Size.y;
        _Velocity.y = 0.f;
    }
    if (_Position.x < 0) _Position.x = 0;
    if (_Position.y < 0) _Position.y = 0;
}

void GetPlayerInput()
{
    Vector2 direction = {.x = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A)), .y = 0.f};
    _Velocity.x = direction.x * _Speed;

    if (_Velocity.y > 0 - EPSILON && _Velocity.y < 0 + EPSILON
        && IsKeyDown(KEY_SPACE))
    {
        _Velocity.y = -_JumpForce;
    }
}

void PlayerPhysics()
{
    float dt = GetFrameTime();
    // position = position + ( velocity * deltaTime )
    _Position.x = _Position.x + _Velocity.x * dt;
    _Position.y = _Position.y + _Velocity.y * dt;
    KeepOnScreen();
}

void DrawPlayer()
{
    DrawRectangle(_Position.x, _Position.y, _Size.x, _Size.y, YELLOW);
}

void DrawPlayerStats()
{
    char * velx = malloc(sizeof(char) * 32);
    char * vely = malloc(sizeof(char) * 32);

    sprintf(velx, "Velocity.x : %.6f", _Velocity.x);
    sprintf(vely, "Velocity.y : %.6f", _Velocity.y);

    DrawText(velx, 0, 0, 20, BLACK);
    DrawText(vely, 0, 20, 20, BLACK);
}

Vector2 GetPlayerPosition()
{
    return _Position;
}
