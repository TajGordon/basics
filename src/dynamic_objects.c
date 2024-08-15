#include "raylib.h"
#include "dynamic_objects.h"
#include "level.h"
#include <level.h>
#include <stdio.h>

#define MAX_MOVING_PLATFORMS 100

static MovingPlatform _mPlatforms[MAX_MOVING_PLATFORMS] = {0};
static int _mPlatformCount = 0;

static char _Path[64];

void DrawMovingPlatforms()
{
    for (int i = 0; i < _mPlatformCount; i++)
    {
        MovingPlatform* p = &_mPlatforms[i];
        int height = p->max.y - p->min.y;
        int width = p->max.x - p->min.x;
        DrawRectangle(p->min.x, p->min.y, width, height, WHITE);
    }
}

void LoadMovingPlatforms(char *tilemap_name)
{

}

void EditMovingPlatforms()
{
    if (IsKeyPressed(KEY_P)) { WriteMovingPlatformChanges(); printf("\033[32;1;4mWriting moving platform changes\n\033[0m");}
    if (!EditLevelToggle() && !IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        Vector2 mpos = GetMousePosition();

        MovingPlatform platform = {0};


        _mPlatforms[_mPlatformCount] = platform;
    }
}

void WriteMovingPlatformChanges()
{

}
