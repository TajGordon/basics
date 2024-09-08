class Actor
{
    int xRemainder;
    int yRemainder;

    public virtual bool IsRiding(Solid solid);
    public virtual void Squish();

    public virtual void kill();

    public void MoveX(float amount, Action onCollide)
    {
        xRemainder += amount;
        int move = Round(xRemainder);
        if (move != 0)
        {
            xRemainder -= move;
            int sign = Sign(move);
            while (move != 0)
            {
                if (!collideAt(solids, Position + new Vector2(sign, 0))
                {
                    // There is no Solid immediately beside us
                    Position.X += sign;
                    move -= sign;
                }
                else
                {
                    // Hit a solid!
                    if (onCollide != null)
                        onCollide();
                    break;
                }
            }
        }
    };


    public void MoveY(float amount, Action onCollide)
    {
        yRemainder += amount;
        int move = Round(yRemainder);
        if (move != 0)
        {
            yRemainder -= move;
            int sign = Sign(move);
            while (move != 0)
            {
                if (!collideAt(solids, Position + new Vector2(sign, 0)))
                {
                    Position.X += sign;
                    move -= sign;
                }
                else
                {
                    if (onCollide != null)
                        onCollide();
                    break;
                }
            }
        }
    }
}

class Solid
{
    float xRemainder;
    float yRemainder;

    public void Move(float x, float y)
    {
        xRemainder += x;
        yRemainder += y;
        int moveX = Round(xRemainder);
        int moveY = Round(yRemainder);
        if (moveX != 0 || moveY != 0)
        {
            // Loop through every Actor in the Level, add it to
            // a list if actor.IsRiding(this) is true
            List riding = GetAllRidingActors();

            // Make this Solid non-collidable for Actors,
            Collidable = false;
            // temporarily disable collision so that any actor code doesn't get fucked
            // disable any collision system if there ig

            // we handle the collision resolution so that the actor
            // doesn't know that it's colliding or getting moved,
            // to not fuck with the charas velocity or something ig
            // actor shouldn't know its getting pushed or carried'

            if (moveX != 0)
            {
                xRemainder -= moveX;
                Position.X += moveX;
                if (moveX > 0)
                // moving right
                {
                    foreach (Actor actor in Level.AllActors)
                    {
                        if (overlapCheck(actor))
                        {
                            // if overlapping with the actor
                            // resolve collisions
                            // Push right just enough so that we aren't colliding
                            actor.MoveX(this.Right — actor.Left, actor.Squish);
                        }
                        else if (riding.Contains(actor))
                        // if not overlapping we dont need to resolve an overlapping collision
                        {
                            // Carry right
                            // it the actor is riding us
                            // then we move the actor right the amount that its riding us for
                            // if the actor is riding, we move it the full amount that we are going to move
                            // "impart our full velocity" maybe accurate
                            actor.MoveX(moveX, null);
                            // movex will still stop and not allow any collisions,
                            // because squishing takes precedence over riding,
                            // if we end up colliding even if riding we still die
                        }
                    }
                }
                else
                // moving left, othewise same logic
                {
                    foreach (Actor actor in Level.AllActors)
                    {
                        if (overlapCheck(actor))
                        {
                            // Push left
                            actor.MoveX(this.Left — actor.Right, actor.Squish);
                        }
                        else if (riding.Contains(actor))
                        {
                            // Carry left
                            actor.MoveX(moveX, null);
                        }
                    }
                }
            }

            if (moveY != 0)
            {
                // Do y-axis movement
                …
                foreach (Actor actor in Level.AllActors)
                {
                    if (overlapCheck(actor))
                    {
                        // if overlapping with the actor
                        // resolve collisions
                        // Push right just enough so that we aren't colliding
                        actor.MoveX(this.Right — actor.Left, actor.Squish);
                    }
                    else if (riding.Contains(actor))
                    // if not overlapping we dont need to resolve an overlapping collision
                    {
                        // Carry right
                        // it the actor is riding us
                        // then we move the actor right the amount that its riding us for
                        // if the actor is riding, we move it the full amount that we are going to move
                        // "impart our full velocity" maybe accurate
                        actor.MoveX(moveX, null);
                        // movex will still stop and not allow any collisions,
                        // because squishing takes precedence over riding,
                        // if we end up colliding even if riding we still die
                    }
                }
            }
            else
            // moving left, othewise same logic
            {
                foreach (Actor actor in Level.AllActors)
                {
                    if (overlapCheck(actor))
                    {
                        // Push left
                        actor.MoveX(this.Left — actor.Right, actor.Squish);
                    }
                    else if (riding.Contains(actor))
                    {
                        // Carry left
                        actor.MoveX(moveX, null);
                    }
                }
            }
            }

            // Re-enable collisions for this Solid
            // after processing the movement, why? idk
            Collidable = true;
        }
    }
}
