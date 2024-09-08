#define ROUND(x) (int)(x)
#define SIGN(x) (((x)>0) ? 1 : -1)

struct IVec2
{
    int x;
    int y;
};

IVec2 operator+(IVec2 a, IVec2 b)
{
    return (IVec2){a.x + b.x, a.y + b.y};
}

struct Solid
{
    bool collideAble;
    float xRemainder;
    float yRemainder;
    int left;
    int right;
    int top;
    int bottom;
    IVec2 pos;
};

struct Actor
{
    float xRemainder;
    float yRemainder;
    int left;
    int right;
    int top;
    int bottom;
    IVec2 pos;
};

bool collideAt(Solid* solids, int solidCount, Actor* a, IVec2 pos)
{
    return false;
}

void onCollide();

void MoveX(float amount, Actor* a, Solid* solids, int solidCount)
{
    a->xRemainder += amount;
    int move = ROUND(a->xRemainder);
    if (move != 0)
    {
        a->xRemainder -= move;
        int sign = SIGN(move);
        while (move != 0)
        {
            if (!collideAt(solids, solidCount, a, a->pos + (IVec2){sign, 0}))
            {
                // There is no Solid immediately beside us
                a->pos.x += sign;
                move -= sign;
            }
            else
            {
                // Hit a Solid!
                // if (onCollide != null) // Checks if we have the onCollide function since its a delegate which is an object that acts like a function
                // we dont need the above

                onCollide(); // do collision logic
                break; // break the loop and stop moving
            }
        }
    }
}

void MoveY(float amount, Actor* a, Solid* solids, int solidCount)
{
    a->yRemainder += amount;
    int move = ROUND(a->yRemainder);
    if (move != 0)
    {
        a->yRemainder -= move;
        int sign = SIGN(move);
        while (move != 0)
        {
            if (!collideAt(solids, solidCount, a, a->pos + (IVec2){0, sign}))
            {
                // There is no Solid immediately beside us
                a->pos.y += sign;
                move -= sign;
            }
            else
            {
                // Hit a Solid!
                // if (onCollide != null) // Checks if we have the onCollide function since its a delegate which is an object that acts like a function
                // we dont need the above

                onCollide(); // do collision logic
                break; // break the loop and stop moving
            }
        }
    }
}

bool SolidOverlapping(Solid* s, Actor* a)
{
    return false;
}

Actor** GetAllRidingActors(Solid* s, int* RiderCount)
{
    return nullptr;
}

bool RidingContainsActor(Actor** riding, int riderCount, Actor* actor)
{
    return true;
}

void MoveSolid(Solid* s, Actor** actors, int actorCount, Solid* solids, int solidCount, float x, float y)
{
    s->xRemainder += x;
    s->yRemainder += x;
    int moveX = ROUND(s->xRemainder);
    int moveY = ROUND(s->yRemainder);
    if (moveX != 0 || moveY != 0)
    {
        // Loop through every Actor in the Level,
        // add it to a list if actor is riding us
        int riderCount;
        Actor** riding = GetAllRidingActors(s, &riderCount);

        s->collideAble = false;


        if (moveX != 0)
        {
            s->xRemainder -= moveX;
            s->pos.x += moveX; // we don't need to check if we are colliding at each step
            // if we are colliding too fucking bad, we get there no matter what, we are a solid
            if (moveX > 0) // moving right
            {
                for (int i = 0; i < actorCount; i++)
                {
                    // Push takes priority
                    if (SolidOverlapping(s, actors[i]))
                    {
                        MoveX(s->right - riding[i]->left, riding[i], solids, solidCount);
                    }
                    // if not pushing, and riding us
                    else if (RidingContainsActor(riding, riderCount, actors[i]))
                    {
                        // move the actor the entire way we moved
                        // because we moved a lot, we teleported not one pixel at a time, so we need to carry any riders with us
                        MoveX(moveX, actors[i], solids, solidCount);
                    }
                }
            }
            else // Moving left
            {
                for (int i = 0; i < actorCount; i++)
                {
                    // Push takes priority
                    if (SolidOverlapping(s, actors[i]))
                    {
                        MoveX(s->left - riding[i]->right, riding[i], solids, solidCount);
                    }
                    // if not pushing, and riding us
                    else if (RidingContainsActor(riding, riderCount, actors[i]))
                    {
                        // move the actor the entire way we moved
                        // because we moved a lot, we teleported not one pixel at a time, so we need to carry any riders with us
                        MoveX(moveX, actors[i], solids, solidCount);
                    }
                }
            }
        }

        if (moveY != 0)
        {
            s->yRemainder -= moveY;
            s->pos.y += moveY; // we don't need to check if we are colliding at each step
            // if we are colliding too fucking bad, we get there no matter what, we are a solid
            // we teleport there
            if (moveY > 0) // moving down
            {
                for (int i = 0; i < actorCount; i++)
                {
                    // Push takes priority
                    if (SolidOverlapping(s, actors[i]))
                    {
                        MoveY(s->top - riding[i]->bottom, riding[i], solids, solidCount);
                    }
                    // if not pushing, and riding us
                    else if (RidingContainsActor(riding, riderCount, actors[i]))
                    {
                        // move the actor the entire way we moved
                        // because we moved a lot, we teleported not one pixel at a time, so we need to carry any riders with us
                        MoveY(moveY, actors[i], solids, solidCount);
                    }
                }
            }
            else // Moving left
            {
                for (int i = 0; i < actorCount; i++)
                {
                    // Push takes priority
                    if (SolidOverlapping(s, actors[i]))
                    {
                        MoveY(s->bottom - riding[i]->top, riding[i], solids, solidCount);
                    }
                    // if not pushing, and riding us
                    else if (RidingContainsActor(riding, riderCount, actors[i]))
                    {
                        // move the actor the entire way we moved
                        // because we moved a lot, we teleported not one pixel at a time, so we need to carry any riders with us
                        MoveY(moveY, actors[i], solids, solidCount);
                    }
                }
            }
        }
        s->collideAble = true;
    }
}
