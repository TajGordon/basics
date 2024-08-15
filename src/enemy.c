#include "raylib.h"
#include "player.h"
#include "enemy.h"
#include <raymath.h>

#define MAX_ENEMIES 512

static Enemy _Enemies[MAX_ENEMIES];

static int _EnemyCount = 0;

static float _Speed = 100.f;

void HandleEnemySpawningFromMouse()
{
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;
    Vector2 MousePosition = GetMousePosition();
    SpawnEnemy(MousePosition.x, MousePosition.y);
}

void UpdateEnemyVelocities()
{
    for (int i = 0; i < _EnemyCount; i++)
    {
        Enemy * enemy = &_Enemies[i];
        Vector2 direction = Vector2Subtract(GetPlayerPosition(), enemy->pos);
        direction = Vector2Normalize(direction);
        enemy->vel = Vector2Scale(direction, _Speed);
    }
}

void MoveEnemies()
{
    float dt = GetFrameTime();
    for (int i = 0; i < _EnemyCount; i++)
    {
        Enemy * enemy = &_Enemies[i];
        enemy->pos = Vector2Add(enemy->pos, Vector2Scale(enemy->vel, dt));
    }
}

void EnemiesPhysicsUpdate()
{
    UpdateEnemyVelocities();
    MoveEnemies();
}

void SpawnEnemy(float posx, float posy)
{
    Enemy enemy = {
        .pos = {
            .x = posx,
            .y = posy,
        },
        .vel = {
            .x = 0.f,
            .y = 0.f,
        },
        .size = {
            .x = 20.f,
            .y = 50.f,
        },
        .color = RED,
    };
    _Enemies[_EnemyCount] = enemy;
    _EnemyCount++;
}

void DrawEnemies()
{
    for (int i = 0; i < _EnemyCount; i++)
    {
        DrawRectangle(_Enemies[i].pos.x, _Enemies[i].pos.y, _Enemies[i].size.x, _Enemies[i].size.y, _Enemies[i].color);
    }
}
