#include "raylib.h"
#include "player.h"
#include "enemy.h"
#include "level.h"
#include "dynamic_objects.h"

int main(void)
{
    const int SCREEN_HEIGHT = 800;
    const int SCREEN_WIDTH = 1200;

    SetTargetFPS(60.);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Learning Raylib");

    LoadLevel("map_1");
    LoadMovingPlatforms("do_1");

    while (!WindowShouldClose())
    {
        {
            EditLevel();
            EditMovingPlatforms();
            MoveMovingPlatforms();
            HandleEnemySpawningFromMouse();
            EnemiesPhysicsUpdate();
            GetPlayerInput();
            PlayerPhysics();
        }

        BeginDrawing();
            ClearBackground(BLUE);
            DrawLevel();
            DrawMovingPlatforms();
            DrawEnemies();
            DrawPlayer();
            DrawPlayerStats();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
