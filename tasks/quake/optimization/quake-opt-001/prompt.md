# Optimize PVS Decompression for Large Maps

## Problem Description

Custom maps with large, complex geometry experience significant frame time spikes. Profiling reveals that the Potentially Visible Set (PVS) decompression is a major bottleneck, especially when the player moves between areas with vastly different visibility sets.

The PVS is crucial for Quake's rendering performance - it pre-computes which leaves can potentially see each other, allowing the renderer to skip most of the level. However, the decompression of this visibility data is becoming a bottleneck.

## Background: Quake's PVS System

The PVS is stored as a compressed bitmap for each leaf:
- Each bit represents whether another leaf is potentially visible
- Run-length encoding (RLE) compresses sequences of zeros
- Decompression happens every time the player enters a new leaf

```c
// Compressed PVS format:
// - Non-zero byte: raw visibility bits
// - Zero byte followed by count: skip 'count' bytes of zeros
// Example: 0xFF, 0x00, 0x03, 0x01 means:
//   8 visible, skip 24 invisible, 1 visible

byte *Mod_DecompressVis(byte *in, model_t *model)
{
    static byte decompressed[MAX_MAP_LEAFS/8];
    int c;
    byte *out;
    int row;

    row = (model->numleafs+7)>>3;
    out = decompressed;

    do {
        if (*in) {
            *out++ = *in++;
            continue;
        }

        c = in[1];
        in += 2;
        while (c) {
            *out++ = 0;
            c--;
        }
    } while (out - decompressed < row);

    return decompressed;
}
```

## The Problem

The current implementation has several performance issues:

1. **Branch-heavy loop**: The main loop has unpredictable branches
2. **Byte-by-byte processing**: No SIMD or word-level operations
3. **Static buffer**: Always decompresses to the full buffer size
4. **No caching**: Decompresses from scratch every time

## Files

The PVS decompression is in `game/model.c`. Focus on:
- `Mod_DecompressVis()` - The main decompression function
- `Mod_LeafPVS()` - Called by the renderer to get PVS for current leaf

## Performance Target

Current performance (on test map):
- Average decompression: 45μs
- Worst case (sparse visibility): 120μs

Target performance:
- Average decompression: < 20μs
- Worst case: < 50μs

## Optimization Strategies

Consider these approaches:

1. **Word-level processing**: Process 4 bytes at a time when possible
2. **Unrolled loops**: Reduce loop overhead for common cases
3. **Branch prediction hints**: Structure code to favor common paths
4. **Incremental decompression**: Only decompress what's needed
5. **Caching**: Cache recently used PVS data

## Constraints

- Must produce identical output to the original function
- Must work on 32-bit systems (no 64-bit assumptions)
- Cannot use SIMD intrinsics (portability)
- Memory usage should not increase significantly
- Must handle malformed input gracefully (no crashes)

## Testing

```bash
# Compile with optimization
make test_pvs_perf CFLAGS="-O2"

# Run performance tests
./test_pvs_perf
```

The tests measure:
- Correctness (output matches reference implementation)
- Average decompression time across test cases
- Worst-case decompression time
- Memory access patterns

## Hints

1. The zero-run case is less common than the non-zero case - optimize for the common path
2. Consider what happens when you can guarantee 4-byte alignment
3. The count byte in a zero-run is rarely > 4, optimize for small runs
4. A lookup table for common patterns might help
5. Profile before and after - sometimes "optimizations" make things worse
