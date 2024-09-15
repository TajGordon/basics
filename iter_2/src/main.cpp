#include "guplib.hpp"
#include "raylib.h"
#include "guplib.cpp"
#include <ctime>
#include <malloc/_malloc.h>





int main(void)
{
    InitWindow(windowWidth, windowHeight, "drinkingbatteryacid.jfif");
    SetTargetFPS(120);

    if (ReadTopScoreFromFile() == 1)
    {
        printf("\033[1;31mError: File pointer is NULL.\033[0m\n");
    }

    LoadTileTextures();
    LoadBulletTextures();
    LoadBatteryTexture();
    LoadEnemyTextures();

    bool exitWindow = false;
    while (!exitWindow)
    {
        switch (gamestate)
        {
            case running:
            {
                float dt = GetFrameTime();
                physicsAccumulator += dt;

                // Physics update
                double time = GetTime();
                if (physicsAccumulator > physicsDTs)
                {
                    physicsAccumulator -= physicsDTs;
                    /*-------------------------*/
                    TickDisplayMessagesTimers();
                    EnemyPhysicsProcess(&p, solids, solidCount, time);
                    BulletPhysicsProcess();
                    PlayerPhysicsProcess(&p, solids, solidCount, time);
                }
                // Render update
                {
                    CameraFollowPlayer(&camera, &p);
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
                        RenderShip(camera, &p);
                        DrawBatteries(&p, camera, time);
                        RenderDoors(camera, &p);
                        RenderDisplayMessages();
                        RenderEnemies(camera);
                        DrawBullets();
                        DrawPlayer(p);
                    }
                    EndMode2D();

                    // Debug info
                    // DrawText(TextFormat("Players x velocity: %f", p.vel.x), 20, 20, 40, RED);
                    // DrawText(TextFormat("Players pos: %f %f ", p.pos.x, p.pos.y), 20, 100, 40, RED);
                    // DrawText(TextFormat("aliveenemycount: %d", aliveEnemyCount), 20, 60, 40, RED);
                    // DrawText(TextFormat("enemycount: %d", enemyCount), 20, 180, 40, RED);
                    // DrawText(TextFormat("Player.health: %d", p.health), 20, 140, 40, RED);
                    // DrawText(TextFormat("Player.score: %d", p.score), 20, 260, 40, RED);
                    // DrawText(TextFormat("Frame MS: %f", dt * 1000), 20, 220, 50, GREEN);
                    DrawText(TextFormat("FPS: %f", 1/dt), 20, 300, 50, GREEN);
                }
                EndDrawing();
                exitWindow = WindowShouldClose();
                break;
            }
            case gameover:
            {
                if (IsKeyPressed(KEY_X))
                {
                    gamestate = gameloadingscreen;
                    newtopscoreflag = 0;
                }
                if (IsKeyPressed(KEY_C))
                { exitWindow = true; }
                BeginDrawing();
                {
                    ClearBackground(BLACK);
                    const char* titletext = "GAME OVER\0";
                    DrawText(titletext, (windowWidth / 2) - MeasureText(titletext, TITLEFONTSIZE * 2)/2, windowHeight/2 - TITLEFONTSIZE * 2, TITLEFONTSIZE * 2, MAROON);
                    const char* subtitletext = TextFormat("SCORE: %d", p.score);
                    const char* subsubtitletext = "Press 'X' to return to the main menu\0";
                    DrawText(subtitletext, (windowWidth / 2) - MeasureText(subtitletext, SUBTITLEFONTSIZE)/2, windowHeight/2 , SUBTITLEFONTSIZE, WHITE);
                    subtitletext = TextFormat("NATTERIES COLLECTED: %d", 0);// p->batteriesCollected);
                    // DrawText(subtitletext, (windowWidth / 2) - MeasureText(subtitletext, SUBTITLEFONTSIZE)/2, windowHeight/2 + SUBTITLEFONTSIZE, SUBTITLEFONTSIZE, WHITE);
                    DrawText(subsubtitletext, (windowWidth / 2) - MeasureText(subsubtitletext, SUBSUBTITLEFONTSIZE)/2, windowHeight/2 + SUBTITLEFONTSIZE * 4, SUBSUBTITLEFONTSIZE, DARKGRAY);
                    const char* newsubtitletext = "NEW TOP SCORE!!!\n";
                    if (newtopscoreflag)
                    {
                        DrawText(newsubtitletext, 0, 0, 30, GOLD);
                    }
                }
                EndDrawing();
                exitWindow = WindowShouldClose();
                break;
            }
            case gameloadingscreen:
            {
                exitWindow = WindowShouldClose();
                if (IsKeyPressed(KEY_O))
                {
                    LoadGame();
                    gamestate = running;
                }
                if (IsKeyPressed(KEY_C))
                {
                    exitWindow = true;
                }
                BeginDrawing();
                {
                    ClearBackground(BLACK);
                    const char* titletext = "DURACELL ADVENTURES\0";
                    DrawText(titletext, (windowWidth / 2) - MeasureText(titletext, TITLEFONTSIZE)/2, windowHeight/2 - TITLEFONTSIZE, TITLEFONTSIZE, MAROON);
                    const char* subtitletext = "Press 'O' to start\0";
                    DrawText(subtitletext, (windowWidth / 2) - MeasureText(subtitletext, SUBTITLEFONTSIZE)/2, windowHeight/2 , SUBTITLEFONTSIZE, WHITE);

                    if (maxscore > 0)
                    {
                        const char* scoretext = TextFormat("Top Score: %d", maxscore);
                        DrawText(scoretext, 0, 0, 25, GOLD);
                    }
                    if (timeswon > 0)
                    {
                        const char* scoretext = TextFormat("Times won: %d", timeswon);
                        DrawText(scoretext, windowWidth - MeasureText(scoretext, 25), 0, 25, GOLD);
                    }
                }
                EndDrawing();
                break;
            }
            case win:
            {
                if (IsKeyPressed(KEY_X))
                { gamestate = gameloadingscreen; newtopscoreflag = 0; }
                if (IsKeyPressed(KEY_C))
                { exitWindow = true; }

                BeginDrawing();
                {
                    ClearBackground(BLACK);
                    const char* titletext = "YOU WON!\0";
                    DrawText(titletext, (windowWidth / 2) - MeasureText(titletext, TITLEFONTSIZE * 2)/2, windowHeight/2 - TITLEFONTSIZE * 2, TITLEFONTSIZE * 2, MAROON);
                    const char* subtitletext = TextFormat("SCORE: %d", p.score);
                    const char* subsubtitletext = "Press 'X' to return to the main menu\0";
                    DrawText(subtitletext, (windowWidth / 2) - MeasureText(subtitletext, SUBTITLEFONTSIZE)/2, windowHeight/2 , SUBTITLEFONTSIZE, WHITE);
                    subtitletext = TextFormat("BATTERIES COLLECTED: %d", 0);// p->batteriesCollected);
                    // DrawText(subtitletext, (windowWidth / 2) - MeasureText(subtitletext, SUBTITLEFONTSIZE)/2, windowHeight/2 + SUBTITLEFONTSIZE, SUBTITLEFONTSIZE, WHITE);
                    DrawText(subsubtitletext, (windowWidth / 2) - MeasureText(subsubtitletext, SUBSUBTITLEFONTSIZE)/2, windowHeight/2 + SUBTITLEFONTSIZE * 4, SUBSUBTITLEFONTSIZE, DARKGRAY);
                    const char* newsubtitletext = "NEW TOP SCORE!!!\n";
                    if (newtopscoreflag)
                    {
                        DrawText(newsubtitletext, 0, 0, 30, GOLD);
                    }
                }
                EndDrawing();
                exitWindow = WindowShouldClose();

                break;
            }
        }

    }
    CloseWindow();


    if (WriteTopScoreToFile())
    {
        printf("\033[1;31mError: File pointer is NULL.\033[0m\n");
        return 1;
    }

    return 0;
}
