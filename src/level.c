#include "raylib.h"
#include "level.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TILEMAP_WIDTH   24
#define TILEMAP_HEIGHT  16

static char _EditingLevel = 1;

static int _TileMap[TILEMAP_WIDTH][TILEMAP_HEIGHT] = {0};

static char _Path[64];

char EditLevelToggle()
{
    return _EditingLevel;
}

void GenerateRandomLevel(char path[])
{
    FILE * fp = fopen(path, "w");

    if (fp == NULL)
    {
        printf("Failed to open file: %s\n", path);
        return;
    }

    fprintf(fp, "%d %d", TILEMAP_WIDTH, TILEMAP_HEIGHT);

    for (int y = 0; y < TILEMAP_HEIGHT; y++)
    {
        fprintf(fp, "\n");
        for (int x = 0; x < TILEMAP_WIDTH; x++)
        {
            int num = rand() % 4;
            fprintf(fp, "%d ", num);
        }
    }
    fclose(fp);
}

void LoadLevel(char tilemap_name[])
{
    sprintf(_Path, "tilemaps/%s.txt", tilemap_name);

    FILE * fp = fopen(_Path, "r+");
    if (fp == NULL)
    {
        printf("Failed to open file: %s\n", _Path);
        return;
    }

    int xw, yh;
    fscanf(fp, "%d %d", &xw, &yh);
    for (int y = 0; y < yh; y++)
    {
        for (int x = 0; x < xw; x++)
        {
            int value;
            fscanf(fp, "%d", &value);
            _TileMap[x][y] = value;
        }
    }
    fclose(fp);
}

void DrawLevel()
{
    int rw = GetScreenWidth()/TILEMAP_WIDTH;
    int rh = GetScreenHeight()/TILEMAP_HEIGHT;

    for (int y = 0; y < TILEMAP_HEIGHT; y++)
    {
        for (int x = 0; x < TILEMAP_WIDTH; x++)
        {
            Color color;
            switch (_TileMap[x][y])
            {
                case 0:
                    continue;
                    break;
                case 1:
                    color = MAROON;
                    break;
                case 2:
                    color = GRAY;
                    break;
                case 3:
                    color = DARKBLUE;
                    break;
                default:
                    continue;
                    break;
            }
            // int posx = x * rw;
            // int posy = y * rw;

            // int width = rw;
            // int height = rh;

            DrawRectangle(x * rw, y * rw, rw, rh, color);
        }
    }
}

void EditLevel()
{
    if (IsKeyPressed(KEY_U)) {WriteLevelChanges(); printf("\033[31;1;4mWriting file changes\n\033[0m");}
    if (IsKeyPressed(KEY_T)) {_EditingLevel = _EditingLevel ^ 1;}
    if (_EditingLevel && !IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) return;

    Vector2 mpos = GetMousePosition();

    int mx = mpos.x / (GetScreenWidth()/TILEMAP_WIDTH);
    int my = mpos.y / (GetScreenHeight()/TILEMAP_HEIGHT);
    _TileMap[mx][my] = 1;
}

void WriteLevelChanges()
{
    FILE * fp = fopen(_Path, "w");
    if (fp == NULL)
    {
        printf("\033[31;1;4m Failed to open file: %s\n \033[0m", _Path);
        return;
    }

    int xw = TILEMAP_WIDTH;
    int yh = TILEMAP_HEIGHT;
    fprintf(fp, "%d %d", xw, yh);

    for (int y = 0; y < yh; y++)
    {
        fprintf(fp, "\n");
        for (int x = 0; x < xw; x++)
        {
            fprintf(fp, "%d ", _TileMap[x][y]);
        }
    }
    fclose(fp);
}

#ifdef LEVEL_MAIN
int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 0;
    }

    char * path = (char*)malloc(sizeof(char) * 30);
    sprintf(path, "tilemaps/%s.txt", argv[1]);
    GenerateRandomLevel(path);
    printf("Success! -- Probably...\n");

    return 0;
}
#endif
