#include "guplib.hpp"
#include "/Users/tajgordon/raylib-5.0_webassembly/include/raylib.h"
#include "guplib.cpp"
#include <ctime>

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif


void UpdateDrawFrame(void);

bool exitWindow = false;

int main(void)
{
    InitWindow(windowWidth, windowHeight, "drinkingbatteryacid.jfif");

    if (ReadTopScoreFromFile() == 1)
    {
        printf("\033[1;31mError: File pointer is NULL.\033[0m\n");
    }

    LoadTileTextures();
    LoadBulletTextures();
    LoadBatteryTexture();
    LoadEnemyTextures();

    #ifdef PLATFORM_WEB
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
    #else
    SetTargetFPS(120);
    while (!exitWindow)
    {
        UpdateDrawFrame();
    }
    #endif

    if (WriteTopScoreToFile())
    {
        printf("\033[1;31mError: File pointer is NULL.\033[0m\n");
        return 1;
    }
    CloseWindow();
    return 0;
}

void UpdateDrawFrame(void)
{
    switch (gamestate)
    {
        case reload:
        {
            LoadGame();
            gamestate = running;
        }
        case running:
        {
            float dt = GetFrameTime();
            physicsAccumulator += dt;

            if (IsKeyPressed(KEY_R))
            {
                gamestate = reload;
            }

            // Physics update
            double time = GetTime();
            if (physicsAccumulator >= physicsDTs)
            {
                physicsAccumulator -= physicsDTs;
                /*-------------------------*/
                TickDisplayMessagesTimers();
                EnemyPhysicsProcess(&p, solids, solidCount, time);
                RangedEnemiesPhysicsProcess(&p);
                BulletPhysicsProcess(&p, time);
                PlayerPhysicsProcess(&p, solids, solidCount, time);
                CalculatePlayerFOV(&p);
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
            // (p.pos.y / minHeight) = % down we are
            // when we are at minheight (the bottom), its 255 * (1 - (1)) = black
            depthfactor = (1 - (p.pos.y / minHeight) * 0.7);
            // unsigned char blueamount = 255 * depthfactor;
            // Rendering
            BeginDrawing();
            {
                // SKYBLUE
                // ClearBackground({0, 0, blueamount, 0xff});
                ClearBackground(BLACK);

                RenderGameWithLighting(&p, camera, time);

                // Debug info
                // DrawText(TextFormat("Players x velocity: %f", p.vel.x), 20, 20, 40, RED);
                // DrawText(TextFormat("Players pos: %f %f ", p.pos.x, p.pos.y), 20, 100, 40, RED);
                // DrawText(TextFormat("aliveenemycount: %d", aliveEnemyCount), 20, 60, 40, RED);
                // DrawText(TextFormat("enemycount: %d", enemyCount), 20, 180, 40, RED);
                // DrawText(TextFormat("Player.health: %d", p.health), 20, 140, 40, RED);
                // DrawText(TextFormat("Player.score: %d", p.score), 20, 260, 40, RED);
                // DrawText(TextFormat("Frame MS: %f", dt * 1000), 20, 220, 50, GREEN);
                // DrawText(TextFormat("FPS: %f", 1/dt), 20, 300, 50, GREEN);
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
                subtitletext = TextFormat("BATTERIES FOUND: %d", batteriesFound);// p->batteriesCollected);
                DrawText(subtitletext, (windowWidth / 2) - MeasureText(subtitletext, SUBTITLEFONTSIZE)/2, windowHeight/2 + SUBTITLEFONTSIZE, SUBTITLEFONTSIZE, WHITE);
                DrawText(subsubtitletext, (windowWidth / 2) - MeasureText(subsubtitletext, SUBSUBTITLEFONTSIZE)/2, windowHeight/2 + SUBTITLEFONTSIZE * 4, SUBSUBTITLEFONTSIZE, DARKGRAY);
                // batteriesfound text
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
                if (maxbatteriesfound > 0)
                {
                    const char* batterytext = TextFormat("Most batteries found: %d", maxbatteriesfound);
                    DrawText(batterytext, 0, 25, 25, GOLD);
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
        case specialitem:
        {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            {
                gamestate = running;
            }
            if (IsKeyPressed(KEY_C))
            {
                exitWindow = true;
            }
            static int specialfontsize = 1;
            float dt = GetFrameTime();
            {
                {
                    static int something = 1;
                    if (specialCamera.zoom < EPSILON) something = 1;
                    else if (specialCamera.zoom > 16) something = -1;
                    specialCamera.zoom += 10 * dt * something;
                }
                {
                    static int something = 1;
                    if (specialCamera.rotation < EPSILON) something = 1;
                    else if (specialCamera.rotation > 720) something = -1;
                    specialCamera.rotation += 50 * dt * something;
                }
                {
                    static int something = 1;
                    if (specialfontsize <= 2) something = 1;
                    else if (specialfontsize >= 100) something = -1;
                    specialfontsize += something * 5 * dt;
                }
            }
            const char* text = "YOU FOUND A SPECIAL ITEM";
            int textoffset = MeasureText(text, specialfontsize)/2;
            static double timer = 0;
            timer += 2 * dt;
            bool showmessage = (timer < 0.5);

            if (timer > 1)
            {
                timer = 0;
            }

            BeginDrawing();
            {
                ClearBackground(GOLD);
                BeginMode2D(specialCamera);
                {
                    if (showmessage)
                    {
                        DrawText(text, 0 - textoffset, 0, specialfontsize, BLACK);
                    }
                    else
                    {
                        DrawText(text, 0 - textoffset, 0, specialfontsize, WHITE);
                    }
                }
                EndMode2D();
            }
            EndDrawing();

            exitWindow = WindowShouldClose();

            break;
        }
    }
}
