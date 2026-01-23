# Sound Mixing Overflow Causes Audio Distortion

## Problem Description

Players report severe audio distortion in areas with many simultaneous sound effects:
- Rocket explosions cause loud pops and clicks
- Multiple enemies firing creates harsh distortion
- Underwater or special sound effects cause audio to "break"
- In extreme cases, sounds become completely garbled

The issue has been traced to integer overflow in the sound mixing code (`snd_mix.c`). Quake mixes multiple audio channels into a single output buffer, and when many loud sounds play simultaneously, the intermediate values overflow.

## Background: Quake's Audio System

Quake's audio mixer:
1. Maintains a pool of sound channels (typically 8-32)
2. Each channel plays a sample at a specific volume and position
3. Channels are mixed into an intermediate buffer
4. The buffer is converted to the output format (8-bit or 16-bit)

```c
// Mixing uses 32-bit intermediates
#define PAINTBUFFER_SIZE 512
typedef struct {
    int left;
    int right;
} portable_samplepair_t;

portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];

// Each channel has volume and sample data
typedef struct {
    int leftvol;        // 0-256 left volume
    int rightvol;       // 0-256 right volume
    int pos;            // sample position
    int end;            // end of sample
    sfxcache_t *sfx;    // sample data
} channel_t;
```

## The Bug

The mixing code accumulates samples from all channels:

```c
// Simplified mixing loop (buggy)
for (i = 0; i < count; i++) {
    sample = sfx->data[ch->pos];
    paintbuffer[i].left += (sample * ch->leftvol) >> 8;
    paintbuffer[i].right += (sample * ch->rightvol) >> 8;
}
```

Problems:
1. **Accumulation overflow**: Multiple channels can overflow the 32-bit intermediate
2. **Clipping not applied**: The final conversion to 16-bit doesn't properly clamp
3. **Signed overflow**: Mixing signed samples with volume can overflow before the shift

## Files

The buggy mixing code is in `game/snd_mix.c`. Focus on:
- `SND_PaintChannelFrom8()` - Mixes 8-bit samples
- `SND_PaintChannelFrom16()` - Mixes 16-bit samples
- `S_TransferPaintBuffer()` - Converts to output format

## What to Look For

1. **Sample scaling**: The sample (-128 to 127 for 8-bit) multiplied by volume (0-256) and shifted. Check the math.

2. **Accumulation bounds**: After mixing 32 channels at max volume, what's the theoretical maximum value? Does it fit in 32 bits?

3. **Clipping**: When converting from 32-bit to 16-bit output, values must be clamped to [-32768, 32767]. Check the clipping logic.

4. **Signed arithmetic**: C's signed integer overflow is undefined behavior. Is this handled?

## Expected Behavior

After your fix:
- No distortion even with maximum simultaneous sounds
- Clean clipping (soft limiting) instead of wraparound
- No audible artifacts during intensity peaks
- Performance should remain unchanged (no floating-point)

## Testing

```bash
# Compile the test harness
make test_snd_mix

# Run audio mixing tests
./test_snd_mix
```

The tests include:
- Single channel at various volumes
- Maximum channels at maximum volume
- Boundary conditions (samples at -128/127)
- Output buffer verification against reference

## Hints

1. The fix involves proper clamping at multiple stages
2. Consider using 64-bit intermediates for the accumulation if 32-bit overflows
3. A simple clamp: `val = val > MAX ? MAX : (val < MIN ? MIN : val)`
4. The volume multiplication should happen in a wider type before shifting
5. The original Quake used assembly for performance - the C fallback is what needs fixing
