#ifndef ENEMY_H
#define ENEMY_H

typedef struct Enemy {
    Vector2 pos;
    Vector2 vel;
    Vector2 size;
    Color color;
} Enemy;

void HandleEnemySpawningFromMouse();
void SpawnEnemy(float posx, float posy);

void EnemiesPhysicsUpdate();

void DrawEnemies();

#endif
