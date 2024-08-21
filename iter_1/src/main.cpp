#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <time.h>

/**************************************
*              Random Math            *
**************************************/
#define round(x) (int)(x)

int sign(int x)
{
    return (x > 0) ? 1 : -1;
}
Vector2 operator+(Vector2 a, Vector2 b)
{
    return (Vector2){a.x + b.x, a.y + b.y};
}
Vector2 operator-(Vector2 a, Vector2 b)
{
    return (Vector2){a.x - b.x, a.y - b.y};
}
Vector2 operator*(Vector2 a, Vector2 b)
{
    return (Vector2){a.x * b.x, a.y * b.y};
}
Vector2 operator/(Vector2 a, Vector2 b)
{
    return (Vector2){a.x / b.x, a.y / b.y};
}
Vector2 operator+(Vector2 v, float f)
{
    return (Vector2){v.x + f, v.y + f};
}
Vector2 operator-(Vector2 v, float f)
{
    return (Vector2){v.x - f, v.y - f};
}
Vector2 operator*(Vector2 v, float f)
{
    return (Vector2){v.x * f, v.y * f};
}
Vector2 operator/(Vector2 v, float f)
{
    return (Vector2){v.x / f, v.y / f};
}
/**************************************
*                                     *
**************************************/

typedef long double Time;

#define TARGET_FPS 60
#define PHYSICS_PROCESS_FPS 60

#define GRAVITY 981

#define COYOTE_MS 10
#define JUMP_BUFFER_MS 20
#define JUMP_VELOCITY -500

#define XMOVSPD 400


// debug
bool canJump;
bool jumpPressed;
bool tlastGround;
bool tlastJump;
float _lastGravityAdded = 0;
int onGondCount = 0;

struct Player
{
    Vector2 pos;
    Vector2 size;
    // Feeling f=ma,
    // might delete late X/
    Vector2 vel;
    // For celeste style
    Vector2 rem;
    // Jumping
    bool canJump;
    int jumpCount;
    bool jumpPressed;
    Time jumpLastPressed;
    bool onGround;
    Time lastOnGround;
};

 Time GetTimeMS()
{
    // clock is cpu time since program start,
    // divide by clocks_per_sec to convert to seconds
    // multiply seconds by 1000 to get miliseconds
    Time time = ((double)clock() / CLOCKS_PER_SEC) * 1000;
    return time;
}

void Jump(Player* p, Time time)
{
    p->vel.y = JUMP_VELOCITY;
    p->jumpPressed = false;
    p->jumpCount++;
    p->jumpLastPressed = time;
}

void TryJump(Player* p, Time time)
{
    p->canJump = (p->jumpPressed && p->jumpCount < 1 && time - p->lastOnGround < COYOTE_MS && time - p->jumpLastPressed < JUMP_BUFFER_MS);
    if (p->canJump)
    {
        // HOLY FUCK I LOVE CLEAN CODE
        // THIS IS SUPER FUCKING POINTLESS TO HAVE A FUNCTION THAT ABSTRACTS ONE FUCKING LINE OF CODE
        // BUT HOLY FUCK I LOVE CLEAN CODE
        // ITS SO OBVIOUS THAT THE OTHER CODE IS ABOUT DOING THE JUMP
        // FOR ME TO CHANGE IT IN THE FUTURE
        // I FUCKIGN LOVE CLEAN CODE!!!!!!!!!!!!!!!!
        Jump(p, time);
    }
}

struct AABB
{
    Vector2 min;
    Vector2 max;
};

struct Rect
{
    Vector2 pos;
    Vector2 size;
};


struct Box
{
    Rect rec;
    Color color;
};

AABB RecToAABB(Rect r)
{
    return (AABB){(Vector2){r.pos.x - (r.size.x/2), r.pos.y - (r.size.y/2)}, (Vector2){r.pos.x + (r.size.x/2), r.pos.y + (r.size.y/2)}};
}

AABB makeAABB(Vector2 v, Vector2 size)
{
    return (AABB){v, v + size};
}

int AABBsColliding(AABB a, AABB b)
{
    if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
    return true;
}

int PlayerCollidingAt(Player* p, Vector2 position, AABB* collideables, int collideableCount)
{
    AABB pab = makeAABB(position - (p->size/2), p->size);
    // TODO! This should be an actual array
    // Real;y fucking todo
    // collission checking prolly dont work
    // if this isn't initialised the players y position wont go off the screen
    // only happens to y position

    for (int i = 0; i < collideableCount; i++)
    {
        if (AABBsColliding(pab, collideables[i]))
        {
            return true;
        }
    }

    return false;
}

void MoveX(Player* p, float amount, AABB* collideables, int collideableCount, Time time)
{
    p->rem.x += amount;
    int move = round(amount);
    if (move == 0) return;
    p->rem.x -= move;
    int Sign = sign(move);
    while (move != 0)
    {
        Vector2 newPos = p->pos;
        newPos.x += (float)Sign;
        if (PlayerCollidingAt(p, newPos, collideables, collideableCount))
        {
            p->vel.x = 0;
            break;
        }
        p->pos.x += (float)Sign;
        move -= Sign;
    }
}

void MoveY(Player* p, float amount, AABB* collideables, int collideableCount, Time time)
{
    p->rem.y += amount;
    int move = round(amount);
    if (move == 0) return;
    p->rem.y -= move;
    int Sign = sign(move);
    while (move != 0)
    {
        Vector2 newPos = p->pos;
        newPos.y += (float)Sign;
        if (PlayerCollidingAt(p, newPos, collideables, collideableCount))
        {
            p->onGround = true;
            p->lastOnGround = time;
            p->vel.y = 0;
            p->jumpCount = 0;
            break;
        }
        p->pos.y += (float)Sign;
        move -= Sign;
    }
}

void PlayerMovement(Player* p, float dt, Time time)
{
    if (IsKeyPressed(KEY_SPACE))
    {
        p->jumpPressed = true;
        p->jumpLastPressed = time;

        // debug
            canJump = p->canJump;
            jumpPressed = p->jumpPressed;
            tlastGround = time - p->lastOnGround < COYOTE_MS;
            tlastJump = time - p->jumpLastPressed < JUMP_BUFFER_MS;
    }
    TryJump(p, time); // Always try to jump, to make coyote time work
    int xDir = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
    p->vel.x = XMOVSPD * xDir;
}

void MoveXY(Player* p, Vector2 amount, AABB* collideables, int collideableCount, Time time)
{
    MoveX(p, amount.x, collideables, collideableCount, time);
    MoveY(p, amount.y, collideables, collideableCount, time);
}


void ApplyGravity(Player* p, float dt)
{
    if (!p->onGround)
    {
        // printf("onGround = %d | ", p->onGround);
        p->vel.y += GRAVITY * dt;
        _lastGravityAdded = GRAVITY * dt;
    }
}
void CheckPlayerOnGround(Player* p, AABB* collideables, int collideableCount, Time time)
{
    // on touching the gronud turn it on, then everytime check if we moved down a litt,e would we collide with something? if so then we are on ground
    // when we touch the ground, set onGround = true
    // touch the ground = collide with something in the positive y direction, then teleport (or not even need to with our movement method) the player up
    // then every physics process (set to a set rate, where a variable framerate only affects drawing)
    // (there is something we can do with interpolation to make the game appear smoother at higher fps disregarding framerate, but I'm not sure what)'
    // check cakez video cuz i think he mentioned it

    // this function doesn't do the initial set'
    // if onGround = false, then we let the other function handle it
    if (p->onGround)
    {
        // if the player is colliding if we move down, then we are on ground
        // 1
        p->onGround = PlayerCollidingAt(p, p->pos + (Vector2){0, 1}, collideables, collideableCount);
        if (p->onGround) p->lastOnGround = time;
    }
    else
    {
        onGondCount++;
    }
}

void PhysicsProcess(Player* p, float dt, AABB* collideables, int collideableCount)
{
    Time time = GetTimeMS();
    CheckPlayerOnGround(p, collideables, collideableCount, time);
    ApplyGravity(p, dt);
    PlayerMovement(p, dt, time);
    MoveXY(p, p->vel * dt, collideables, collideableCount, time);
}

void DrawPlayer(Player* p)
{
    DrawRectangleV(p->pos - (p->size / 2), p->size, YELLOW);
}

void DrawBoxes(Box* boxes, int boxCount)
{
    for (int i = 0; i < boxCount; i++)
    {
        Box box = boxes[i];
        box.rec.pos = box.rec.pos - (box.rec.size / 2);
        DrawRectangleV(box.rec.pos, box.rec.size, box.color);
    }
}

void DrawDebugInfo(Player* p)
{
    // DBG
    DrawText(TextFormat("onGround = %d", p->onGround), 20, 20, 20, YELLOW);
    DrawText(TextFormat("Position = %.1f, %.6f", p->pos.x, p->pos.y), 20, 50, 20, YELLOW);
    DrawText(TextFormat("Velocity = %.1f, %.6f", p->vel.x, p->vel.y), 20, 80, 20, YELLOW);
    DrawText(TextFormat("LastGravityAdded = %.1f, onGround was %d, onGond was %d", _lastGravityAdded, p->onGround, onGondCount), 20, 110, 20, YELLOW);
    DrawText(TextFormat("onGondCount: %d", onGondCount), 20, 140, 20, YELLOW);
    DrawText(TextFormat("canJump = %d, jumpPressed = %d, lastGroundCheck = %d, lastJumpPressCheck = %d", canJump, jumpPressed, tlastGround, tlastJump), 20, 170, 20, YELLOW);
}

void UpdateCameraCenter(Camera2D *camera, Player* player, int width, int height)
{
    camera->offset = (Vector2){width/2.f, height/2.f};
        camera->target = player->pos;
}

int main(void)
{
    int windowWidth = 1280;
    int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "Some fucking title");

    float targetFrameTimeMS = 1.f / TARGET_FPS;

    Player player = {0};
    { // Init player
        player.pos = (Vector2){(float)windowWidth/2, 0};
        player.size = (Vector2){24, 32};
    }

    Camera2D camera = {0};
    camera.target = player.pos;
    camera.offset = (Vector2){ windowWidth/2.0f, windowHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Color backgroundColor = (Color){22, 22, 22, 255};

    Box boxes[20] = {0};
    int boxCount = 0;
    Box floor = {0};
    {
        floor.rec = {(Vector2){(float)windowWidth/2, (float)windowHeight}, (Vector2){(float)windowWidth, 320}};
        floor.color = RED;
        boxes[boxCount++] = floor;
    }
    {
        boxes[1].rec = {(Vector2){416, 480}, (Vector2){32, 32}};
        boxes[1].color = RED;
        boxCount++;
        boxes[2].rec = {(Vector2){480, 416}, (Vector2){32, 32}};
        boxes[2].color = RED;
        boxCount++;
        boxes[3].rec = {(Vector2){640, 352}, (Vector2){32, 32}};
        boxes[3].color = RED;
        boxCount++;
    }
    AABB* collideables = (AABB*)malloc(sizeof(AABB) * boxCount);
    int collideableCount = boxCount;
    for (int i = 0; i < boxCount; i++)
    {
        collideables[i] = RecToAABB(boxes[i].rec);
    }

    Time lastFrameTime = 0;

    float targetPhysicsMS = 1.f / PHYSICS_PROCESS_FPS;

    float frameDtAccumulator = GetTimeMS();
    float physicsDtAccumulator = frameDtAccumulator;

    while (!WindowShouldClose())
    {
        Time currentFrameTime = GetTimeMS();
        float frameDT = currentFrameTime - lastFrameTime;

        if (physicsDtAccumulator > targetPhysicsMS)
        {
            physicsDtAccumulator -= targetPhysicsMS;
            PhysicsProcess(&player, targetPhysicsMS, collideables, collideableCount);
        }

        { // Update
            UpdateCameraCenter(&camera, &player, windowWidth, windowHeight);
        }
        BeginDrawing();
        {
            ClearBackground(backgroundColor);
            BeginMode2D(camera);
            {
                DrawBoxes(boxes, boxCount);
                DrawPlayer(&player);
            }
            EndMode2D();
            DrawDebugInfo(&player);
            DrawFPS(20, windowHeight - 100);
        }
        EndDrawing();

        lastFrameTime = currentFrameTime;
    }
    CloseWindow();

    return 0;
}
