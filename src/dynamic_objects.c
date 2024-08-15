#include "raylib.h"
#include "dynamic_objects.h"
#include "level.h"
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MOVING_PLATFORMS 100

static MovingPlatform _mPlatforms[MAX_MOVING_PLATFORMS] = {0};
static int _mPlatformCount = 0;

static char _isA = 1;

static char _Path[128];

static bool inputActive = false;

char AABBContainsPoint(Vector2 min, Vector2 max, Vector2 point);

void DrawMovingPlatforms()
{
    for (int i = 0; i < _mPlatformCount; i++)
    {
        MovingPlatform* p = &_mPlatforms[i];
        Vector2 minA = {p->posA.x - p->size.x / 2, p->posA.y - p->size.y / 2};
        Vector2 maxA = {p->posA.x + p->size.x / 2, p->posA.y + p->size.y / 2};

        Vector2 minB = {p->posB.x - p->size.x / 2, p->posB.y - p->size.y / 2};
        Vector2 maxB = {p->posB.x + p->size.x / 2, p->posB.y + p->size.y / 2};

        DrawRectangle(minA.x, minA.y, p->size.x, p->size.y, WHITE);
        if (p->speed != 0) {
            DrawRectangle(minB.x, minB.y, p->size.x, p->size.y, GREEN);
            DrawLine(p->posA.x, p->posA.y, p->posB.x, p->posB.y, MAGENTA);
        }
        Vector2 minC = {p->cpos.x - p->size.x / 2, p->cpos.y - p->size.y / 2};
        Vector2 maxC = {p->cpos.x + p->size.x / 2, p->cpos.y + p->size.y / 2};

        DrawRectangle(minC.x, minC.y, p->size.x, p->size.y, MAROON);
    }
}

void MoveMovingPlatforms()
{
    float deltaTime = GetFrameTime();

    for (int i = 0; i < _mPlatformCount; i++)
    {
        MovingPlatform* p = &_mPlatforms[i];
        Vector2 direction = Vector2Subtract(p->posB, p->posA);
        direction = Vector2Normalize(direction);

        if (Vector2Distance(p->cpos, p->posB) < 1.0f)
        {
            Vector2 temp = p->posA;
            p->posA = p->posB;
            p->posB = temp;
        }
        else
        {
            p->cpos = Vector2Add(p->cpos, Vector2Scale(direction, p->speed * deltaTime));
        }
    }
}

void WriteMovingPlatformChanges()
{
    FILE *file = fopen(_Path, "w");
    if (file == NULL)
    {
        printf("Could not open file %s\n", _Path);
        return;
    }

    for (int i = 0; i < _mPlatformCount; i++)
    {
        MovingPlatform* p = &_mPlatforms[i];
        fprintf(file, "\"cpos\": {\"x\": %f, \"y\": %f},\n", p->cpos.x, p->cpos.y);
        fprintf(file, "\"posA\": {\"x\": %f, \"y\": %f},\n", p->posA.x, p->posA.y);
        fprintf(file, "\"posB\": {\"x\": %f, \"y\": %f},\n", p->posB.x, p->posB.y);
        fprintf(file, "\"size\": {\"x\": %f, \"y\": %f},\n", p->size.x, p->size.y);
        fprintf(file, "\"speed\": %d\n", p->speed);
    }

    fclose(file);
}

void LoadMovingPlatforms(char *dynamic_objects_file_name)
{
    strcpy(_Path, "dynamic_objects_files/");
    strcat(_Path, dynamic_objects_file_name);
    strcat(_Path, ".json");

    FILE *file = fopen(_Path, "r");
    if (file == NULL)
    {
        printf("Could not open file %s\n", _Path);
        return;
    }

    char line[256];
    int index = 0;
    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "\"cpos\":"))
        {
            sscanf(line, "\"cpos\": {\"x\": %f, \"y\": %f},", &_mPlatforms[index].cpos.x, &_mPlatforms[index].cpos.y);
        }
        else if (strstr(line, "\"posA\":"))
        {
            sscanf(line, "\"posA\": {\"x\": %f, \"y\": %f},", &_mPlatforms[index].posA.x, &_mPlatforms[index].posA.y);
        }
        else if (strstr(line, "\"posB\":"))
        {
            sscanf(line, "\"posB\": {\"x\": %f, \"y\": %f},", &_mPlatforms[index].posB.x, &_mPlatforms[index].posB.y);
        }
        else if (strstr(line, "\"size\":"))
        {
            sscanf(line, "\"size\": {\"x\": %f, \"y\": %f},", &_mPlatforms[index].size.x, &_mPlatforms[index].size.y);
        }
        else if (strstr(line, "\"speed\":"))
        {
            sscanf(line, "\"speed\": %d", &_mPlatforms[index].speed);
            index++;
        }
    }

    _mPlatformCount = index;

    fclose(file);
}


static MovingPlatform* editingPlatform = NULL;

void EditMovingPlatforms()
{
    if (IsKeyPressed(KEY_P)) { WriteMovingPlatformChanges(); printf("\033[32;1;4mWriting moving platform changes\n\033[0m");}

    Vector2 mpos = GetMousePosition();

    if (!EditLevelToggle() && !inputActive && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    { // Creates the end points
        if (_isA)
        {
            MovingPlatform platform = {0};

            platform.size.x = 50;
            platform.size.y = 50;

            platform.posA.x = mpos.x;
            platform.posA.y = mpos.y;

            platform.cpos = platform.posA;

            _isA = 0;
            _mPlatforms[_mPlatformCount++] = platform;
        }
        else
        {
            MovingPlatform* platform = &_mPlatforms[_mPlatformCount - 1];

            platform->posB.x = mpos.x;
            platform->posB.y = mpos.y;
            platform->speed = 10;
            _isA = 1;
        }
    }

    if (!EditLevelToggle())
    { // edit the velocity
        if (!inputActive && IsKeyPressed(KEY_E))
        {
            for (int i = 0; i < _mPlatformCount; i++)
            {
                MovingPlatform* p = &_mPlatforms[i];
                Vector2 minA = {p->posA.x - p->size.x / 2, p->posA.y - p->size.y / 2};
                Vector2 maxA = {p->posA.x + p->size.x / 2, p->posA.y + p->size.y / 2};

                Vector2 minB = {p->posB.x - p->size.x / 2, p->posB.y - p->size.y / 2};
                Vector2 maxB = {p->posB.x + p->size.x / 2, p->posB.y + p->size.y / 2};
                if (AABBContainsPoint(minA, maxA, mpos) || AABBContainsPoint(minB, maxB, mpos))
                { // We are on a tile
                    editingPlatform = p;
                    inputActive = true;
                    break;
                }
            }
        }

        if (inputActive)
        {
            char text[512];
            sprintf(text, "Current Speed: %d", editingPlatform->speed);
            DrawText(text, mpos.x, mpos.y + 30, 20, ORANGE);

            static char input[512] = "\0"; // Buffer for user input
            static int letterCount = 0;

            if (IsKeyPressed(KEY_ENTER)) {
                if (letterCount > 0) {
                    editingPlatform->speed = atoi(input); // Convert input to integer and save it to p->speed
                    letterCount = 0; // Reset input length
                    input[0] = '\0'; // Clear input buffer
                }
                inputActive = false; // Disable input mode after input is finalized
                editingPlatform = NULL;
            }

            int key = GetCharPressed();

            while (key > 0) {
                if ((key >= '0') && (key <= '9') && (letterCount < 511)) {
                    input[letterCount] = (char)key;
                    letterCount++;
                    input[letterCount] = '\0'; // Add null terminator
                }
                key = GetCharPressed(); // Check next character pressed
            }

            // Draw the user input and instruction below the current speed
            DrawText("Enter a new speed and press ENTER:", mpos.x, mpos.y + 60, 20, LIGHTGRAY);
            DrawText(input, mpos.x, mpos.y + 90, 20, LIGHTGRAY);
        }
    }
}


char AABBContainsPoint(Vector2 min, Vector2 max, Vector2 point)
{
    if (point.x > min.x && point.x < max.x
    &&  point.y > min.y && point.y < max.y) return 1;
    return 0;
}
