# Bug Report: Fireworks Timing Offset Error

## Summary

Festival fireworks are scheduled at the wrong time because the code subtracts the timing offset instead of adding it. This causes fireworks to appear before the festival instead of during the celebration.

## Environment

- Project: Julius (Caesar III reimplementation)
- File: `src/city/festival.c`
- Severity: Low (visual/gameplay issue, not critical)

## Bug Description

When scheduling fireworks for a festival, the code calculates when the fireworks should fire:

```c
void schedule_festival_fireworks(int festival_start_tick)
{
    int fireworks_delay = get_fireworks_delay();  // e.g., 100 ticks

    // BUG: Subtracts instead of adds - fireworks fire BEFORE festival!
    int fireworks_tick = festival_start_tick - fireworks_delay;

    event_schedule(EVENT_FIREWORKS, fireworks_tick);
}
```

If the festival starts at tick 1000 and fireworks should fire 100 ticks into the festival:
- **Buggy**: `1000 - 100 = 900` (fireworks at tick 900, before festival!)
- **Correct**: `1000 + 100 = 1100` (fireworks at tick 1100, during festival)

## Steps to Reproduce

1. Schedule a large festival with fireworks
2. Note when the festival is scheduled to begin
3. Observe that fireworks appear BEFORE the festival starts
4. The celebration looks wrong - fireworks should happen during the event

## Expected Behavior

Fireworks should fire after the festival begins, at `festival_start_tick + fireworks_delay`.

## Current Behavior

Fireworks fire before the festival at `festival_start_tick - fireworks_delay`, which can even result in negative tick values for early festivals.

## Relevant Code

Look at `src/city/festival.c`:
- `schedule_festival_fireworks()` function
- `calculate_fireworks_time()` if it exists
- Event scheduling functions

## Suggested Fix Approach

Change subtraction to addition:

```c
void schedule_festival_fireworks(int festival_start_tick)
{
    int fireworks_delay = get_fireworks_delay();

    // FIXED: Add offset to fire AFTER festival starts
    int fireworks_tick = festival_start_tick + fireworks_delay;

    event_schedule(EVENT_FIREWORKS, fireworks_tick);
}
```

## Your Task

Fix the timing calculation by changing subtraction to addition. Your fix should:

1. Calculate the correct future tick for fireworks
2. Ensure fireworks fire during (not before) the festival
3. Handle edge cases where delay might be zero
4. Not change other timing calculations

Provide your fix as a unified diff (patch).
