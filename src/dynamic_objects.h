#ifndef DYNAMIC_OBJECTS_H
#define DYNAMIC_OBJECTS_H

typedef struct MovingPlatform
{
    Vector2 minA;
    Vector2 maxA;
    Vector2 posA;
    Vector2 minB;
    Vector2 maxB;
    Vector2 posB;
    Vector2 size;
    float speed;
    // stored:
    // posA
    // posB
    // speed
    // size
} MovingPlatform;

void DrawMovingPlatforms();
void LoadMovingPlatforms(char tilemap_name[]);
void EditMovingPlatforms();
void WriteMovingPlatformChanges();

#endif
