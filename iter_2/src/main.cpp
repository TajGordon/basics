#include "raylib.h"
#include "guplib.cpp"
#include <malloc/_malloc.h>



int main(void)
{
    const int windowWidth = 1280;
    const int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "A window");
    SetTargetFPS(120);

    #define MAX_SOLID_COUNT 100
    Solid* solids = (Solid*)malloc(sizeof(Solid) * MAX_SOLID_COUNT);
    int solidCount = 0;

    Actor p = {0};
    {
        p.pos = {400, 0};
        p.size = {20, 20};
        p.col = YELLOW;
        p.aabb.max = p.pos + (p.size/2);
        p.aabb.min = p.pos - (p.size/2);
    }

    Camera2D camera;
    {
        camera.zoom = 1;
        camera.rotation = 0;
        camera.offset = {windowWidth / 2.f, windowHeight / 2.f};
        camera.target = {0};
    }

    solids[solidCount++] = MakeSolid(640, 720, 1280, 200, GRAY, true);
    solids[solidCount++] = MakeSolid(640, 0, 1280, 200, GRAY, false);

    double physicsAccumulator = 0;
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        physicsAccumulator += dt;

        // Physics update
        if (physicsAccumulator > physicsDTs)
        {
            physicsAccumulator -= physicsDTs;
            PhysicsUpdate(&p, solids, solidCount);
        }
        // Render update
        {
            CameraFollowActor(&camera, &p);
            if (IsKeyPressed(KEY_SPACE)) p.pressedJump = true;
        }
        // Rendering
        BeginDrawing();
        {
            ClearBackground(BLUE);
            DrawText(TextFormat("Players y velocity: %f", p.vel.y), 20, 20, 20, RED);

            BeginMode2D(camera);
            {
                DrawSolids(solids, solidCount);
                DrawActor(p);
            }
            EndMode2D();
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
