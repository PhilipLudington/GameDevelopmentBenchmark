# Network Prediction Desync Causes Player Teleportation

## Problem Description

QuakeWorld players are experiencing severe rubber-banding and teleportation artifacts. When network conditions become unstable (packet loss, jitter, or lag spikes), players observe:

1. Their character suddenly teleporting to a different position
2. Movement feeling "jerky" even with stable ping
3. Shots not registering because the client's view doesn't match the server state
4. In severe cases, players getting stuck in walls after correction

This is a classic client-side prediction issue. Quake pioneered client-side prediction in FPS games, but the implementation has a subtle bug in how it reconciles client state with server updates.

## Background: Quake's Network Model

Quake uses client-side prediction to hide network latency:

1. **Server authoritative**: The server runs the true game simulation
2. **Client prediction**: The client simulates movement locally for responsiveness
3. **Server correction**: When server updates arrive, the client corrects its state
4. **Interpolation**: Entity positions are interpolated between server updates

Key data structures:

```c
typedef struct {
    int     sequence;       // last executed command sequence
    usercmd_t cmd;         // movement command
    vec3_t  origin;        // position after this command
    vec3_t  velocity;      // velocity after this command
} predicted_frame_t;

// Client keeps a buffer of predicted frames
predicted_frame_t predicted_frames[64];
```

## The Bug

The bug is in the prediction reconciliation code. When the server sends an update:

1. Client compares server position to predicted position for that sequence number
2. If they differ, client should re-simulate from that point
3. **BUG**: The sequence number comparison has an off-by-one error
4. **BUG**: The position comparison uses incorrect tolerance based on velocity

This causes:
- Corrections to be applied to the wrong frame
- High-velocity movement to trigger false corrections
- Corrections to overshoot, causing oscillation

## Files

The buggy implementation spans:
- `game/cl_input.c` - Client input processing and prediction
- `game/cl_parse.c` - Server update parsing and reconciliation

Focus on:
- `CL_PredictMove()` - Client movement prediction
- `CL_ParsePlayerState()` - Server state update handling

## What to Look For

1. **Sequence number handling**: The server sends the last acknowledged sequence number. Check how this maps to the prediction buffer.

2. **Position comparison**: The code compares `server_origin` to `predicted_origin`. The tolerance calculation may be wrong.

3. **Re-simulation**: When a correction is needed, the client should replay all commands from the corrected frame forward. Check the loop bounds.

4. **Wraparound**: Sequence numbers are 32-bit and can wrap. Check for proper handling.

## Expected Behavior

After your fix:
- Smooth movement under moderate packet loss (< 5%)
- No visible teleportation during brief lag spikes
- Graceful degradation under heavy packet loss
- Correct hit registration due to consistent client/server state

## Testing

```bash
# Compile the test harness
make test_prediction

# Run prediction tests with simulated network conditions
./test_prediction
```

The tests simulate:
- Normal conditions (verify no unnecessary corrections)
- 2% packet loss (should be invisible to player)
- 10% packet loss (minor corrections, no teleportation)
- High jitter (varying latency)
- Sequence number wraparound

## Hints

1. The sequence number stored in `predicted_frames` uses a different epoch than the server's acknowledgment number
2. The velocity-based tolerance should use the magnitude, not individual components
3. When re-simulating, the starting state must come from the server update, not the current predicted state
4. Consider what happens when `incoming_sequence - outgoing_acknowledged` exceeds the prediction buffer size
