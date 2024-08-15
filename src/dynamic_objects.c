#include "raylib.h"
#include "dynamic_objects.h"
#include "level.h"
#include <level.h>
#include <stdio.h>

#define MAX_MOVING_PLATFORMS 100

static MovingPlatform _mPlatforms[MAX_MOVING_PLATFORMS] = {0};
static int _mPlatformCount = 0;

static char _isA = 1;

static char _Path[64];

void DrawMovingPlatforms()
{
    for (int i = 0; i < _mPlatformCount; i++)
    {
        MovingPlatform* p = &_mPlatforms[i];
        DrawRectangle(p->minA.x, p->minA.y, p->size.x, p->size.y, WHITE);
        if (platform.speed != 0)
        {
            DrawRectangle(p->minB.x, p->minB.y, p->size.x, p->size.y, MAGENTA);
        }
    }
}

void LoadMovingPlatforms(char *tilemap_name)
{
    // assert(_mPlatformCount == 0)
    _mPlatformCount = 0;

    sprintf(_Path, "tilemaps/%s.txt", tilemap_name);

    FILE * fp = fopen(_Path, "r+");
    if (fp == NULL)
    {
        printf("Failed to open file: %s\n", _Path);
        return;
    }

    int numObjects = 0;

    fscanf(fp, "%d", &numObjects);

    for (int i = 0; i < numObjects; i++)
    {
        MovingPlatform p = 0;
        fscanf(fp, "%d %d %d %d %d %d %d", &p.posA.x, &p.posA.y, &p.posB.x, &p.posB.y, &p.speed, &p.size.x, &p.size.y);
        _mPlatforms[_mPlatformCount++] = p;
    }

    fclose(fp);
}

void EditMovingPlatforms()
{
    if (IsKeyPressed(KEY_P)) { WriteMovingPlatformChanges(); printf("\033[32;1;4mWriting moving platform changes\n\033[0m");}
    if (!EditLevelToggle() && !IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        Vector2 mpos = GetMousePosition();

        MovingPlatform platform = {0};

        if (_isA)
        {
            platform.size.x = 100;
            platform.size.y = 100;

            platform.maxA.x = mpos.x + platform.size.x;
            platform.maxA.y = mpos.y + platform.size.y;

            platform.minA.x = mpos.x;
            platform.minA.y = mpos.y;

            platform.posA.x = mpos.x + platform.size.x / 2;
            platform.posA.y = mpos.y + platform.size.y / 2;

            _isA = 0;
            _mPlatforms[_mPlatformCount++] = platform;
        }
        else
        {
            platform = _mPlatforms[_mPlatformCount];
            platform.posB.x = mpos.x + platform.size.x / 2;
            platform.posB.y = mpos.y + platform.size.y / 2;
            platform.speed = 10.f;
            _isA = 1;
            _mPlatforms[_mPlatformCount] = platform;
        }
    }
}

void WriteMovingPlatformChanges()
{

}
