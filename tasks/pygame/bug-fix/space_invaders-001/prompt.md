# Bullets Pass Through Aliens at High Speed

## Problem Description

Players have reported a frustrating bug: sometimes bullets pass right through aliens without destroying them. This happens most noticeably when:
1. The player's bullet speed has been increased (via power-ups or game progression)
2. The bullet and alien are about to occupy the same space but the bullet "skips over" the alien between frames

This is a classic "tunneling" problem in game physics where fast-moving objects can pass through thin objects because the collision check happens only at discrete time steps.

## Task

Fix the bullet-alien collision detection to use swept collision (also known as continuous collision detection) so that bullets never tunnel through aliens regardless of their speed.

## Files

The main game file is `game/main.py`. Focus on the `Game.update()` method where bullet-alien collisions are handled.

## The Core Issue

Currently, the collision detection simply checks if `bullet.rect.colliderect(alien.rect)` at the current frame. But if a bullet moves 15 pixels per frame and an alien is only 30 pixels tall, a bullet could be at y=100 in one frame and y=85 in the next, completely skipping an alien at y=95.

## Expected Behavior

After your fix:
- Bullets should ALWAYS destroy aliens they would pass through, regardless of bullet speed
- The solution should work for bullet speeds of up to 50 pixels per frame
- Performance should remain acceptable (no significant frame drops)

## Implementation Hints

You need to implement swept collision detection. Here's the approach:

1. Before moving the bullet, calculate the path it will travel this frame
2. Check if any point along that path intersects with any alien
3. If so, register the collision even though the final position might be past the alien

One efficient approach is to use parametric line-rectangle intersection:
- Represent the bullet's movement as a line segment from `(old_x, old_y)` to `(new_x, new_y)`
- For each alien, check if this line segment intersects the alien's rectangle
- You can expand the bullet's rectangle along its movement vector and check intersection

A simpler (but slightly less precise) approach:
- Store the bullet's previous position
- Create a rectangle that spans from the previous position to the current position
- Check if this expanded rectangle collides with any alien

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

The tests will verify that:
- Normal-speed bullets still work correctly
- High-speed bullets (20+ pixels/frame) hit aliens they would tunnel through
- The collision position is approximately correct
