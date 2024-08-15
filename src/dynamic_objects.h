#ifndef DYNAMIC_OBJECTS_H
#define DYNAMIC_OBJECTS_H

typedef struct MovingPlatform
{
    Vector2 min;
    Vector2 max;
    Vector2 posA;
    Vector2 posB;
    float speed;
} MovingPlatform;

void DrawMovingPlatforms();
void LoadMovingPlatforms(char tilemap_name[]);
void EditMovingPlatforms();
void WriteMovingPlatformChanges();

#endif
