#include "raylib.h"
#include "guplib.cpp"
#include <ctime>
#include <malloc/_malloc.h>


int main(void)
{
    InitWindow(windowWidth, windowHeight, "A window");
    SetTargetFPS(120);

    #define MAX_SOLID_COUNT 100
    Solid* solids = (Solid*)malloc(sizeof(Solid) * MAX_SOLID_COUNT);
    int solidCount = 0;

    LoadTilemap("tilemap.txt");


    Actor p = {};
    {
        p.pos = player_spawnpoint;
        p.size = {11, 14};
        p.lastDir = {1, 0};
        p.col = (Color){0xff, 0xff, 0x00, 50};
        p.aabb.max = p.pos + (p.size/2);
        p.aabb.min = p.pos - (p.size/2);
        p.speed = 3;
        p.jumpVel = -3;
        p.maxJumps = 1;
        p.lightShootDelay = 0.2;
        p.heavyShootDelay = 0.5;

        p.tex = LoadTexture("assets/player.png");
    }

    Camera2D camera;
    {
        camera.zoom = 4;
        camera.rotation = 0;
        camera.offset = {windowWidth / 2.f, windowHeight / 2.f + 180};
        camera.target = {0};
    }

    // solids[solidCount++] = MakeSolid(640, 720, 1280, 256, GRAY, true);
    // solids[solidCount++] = MakeSolid(640, 0, 1280, 200, GRAY, false);

    double physicsAccumulator = 0;
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        physicsAccumulator += dt;

        // Physics update
        if (physicsAccumulator > physicsDTs)
        {
            physicsAccumulator -= physicsDTs;
            double time = GetTime();
            /*-----------------*/
            BulletPhysicsProcess();
            PlayerPhysicsProcess(&p, solids, solidCount, time);
        }
        // Render update
        {
            CameraFollowActor(&camera, &p);
            /* Checking for input for things like jumping always */
            /* so that we get the most responsive movement */
            if (IsKeyPressed(KEY_SPACE))
            {
                p.pressedJump = true;
                p.timeLastJumpPressed = GetTime();
            }
        }
        // Rendering
        BeginDrawing();
        {
            ClearBackground(BLUE);

            BeginMode2D(camera);
            {
                DrawTilemap(&p);
                DrawSolids(solids, solidCount);
                DrawBullets();
                DrawActor(p);
            }
            EndMode2D();

            // Debug info
            DrawText(TextFormat("Players x velocity: %f", p.vel.x), 20, 20, 40, RED);
            DrawText(TextFormat("Players pos: %f %f ", p.pos.x, p.pos.y), 20, 100, 40, RED);
            DrawText(TextFormat("BulletCount: %d", bulletCount), 20, 60, 40, RED);
            DrawText(TextFormat("Frame MS: %f", dt * 1000), 20, 220, 50, GREEN);
            DrawText(TextFormat("FPS: %f", 1/dt), 20, 280, 50, GREEN);
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
