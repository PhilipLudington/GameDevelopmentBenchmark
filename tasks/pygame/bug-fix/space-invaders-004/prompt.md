# Dead Aliens Can Still Shoot

## Problem Description

Players have reported a bizarre bug: sometimes bullets appear from empty space where an alien used to be! This happens because of a race condition in the game update loop - the order of operations allows an alien to be selected as a shooter before the collision check destroys it.

This is a subtle timing bug that requires careful analysis of the update loop order.

## Task

Fix the race condition in `Game.update()` that allows destroyed aliens to fire bullets in the same frame they're destroyed.

## Files

The main game file is `game/main.py`. Focus on the `Game.update()` method and understand the order of operations.

## The Core Issue

The current update loop processes in this order:
1. Player bullets move and check collisions (destroying aliens)
2. Alien fleet updates
3. Alien fleet tries to shoot (but may select aliens that were just destroyed)

The problem is that `get_shooters()` might return aliens that were marked `alive = False` earlier in the same frame if the alive check happens before the list comprehension filters them out.

Actually, examine the code more carefully - the issue is that `try_shoot()` is called AFTER the collision checks, but within the same frame. If an alien's `alive` flag was set to `False` during collision, `get_shooters()` correctly filters it... OR DOES IT?

Look at the exact timing and list operations involved.

## Expected Behavior

After your fix:
- Aliens destroyed in a frame should never be able to shoot in that same frame
- No "ghost bullets" should appear from positions where aliens were destroyed
- The shooting logic should only consider aliens that were alive at the START of the frame

## Implementation Hints

Several approaches can fix this:
1. Process alien shooting BEFORE player bullet collisions
2. Snapshot the list of potential shooters at the start of the frame
3. Add an additional alive check right before creating the bullet

Think carefully about which approach is cleanest and least likely to introduce new bugs.

## Testing

Run the automated tests:
```bash
pytest tests/ -v
```

The tests simulate the race condition and verify:
- Aliens destroyed in a frame cannot shoot in that same frame
- No bullets originate from dead alien positions
