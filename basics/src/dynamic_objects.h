#ifndef DYNAMIC_OBJECTS_H
#define DYNAMIC_OBJECTS_H

typedef struct MovingPlatform
{
    Vector2 posA;
    Vector2 posB;
    Vector2 cpos;
    Vector2 size;
    int speed;
    // stored:
    // posA
    // posB
    // speed
    // size
} MovingPlatform;

void DrawMovingPlatforms();
void MoveMovingPlatforms();
void LoadMovingPlatforms(char *dynamic_objects_file_name);
void EditMovingPlatforms();
void WriteMovingPlatformChanges();

#endif
