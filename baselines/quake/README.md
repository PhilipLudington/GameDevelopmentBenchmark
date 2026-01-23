# Quake 1 Baseline

This directory contains the baseline setup for Quake 1 benchmark tasks.

## Source Code

Quake 1 source code is from the official id Software GPL release:
- **Repository:** https://github.com/id-Software/Quake
- **License:** GPL v2

## Setup

Tasks reference specific files from the WinQuake directory. To set up:

```bash
# Clone the Quake source
git clone https://github.com/id-Software/Quake.git quake-source

# The task's game/ directory will contain modified versions of specific files
# The solution/ directory contains the correct implementations
```

## Building

The original Quake requires Visual C++ 6.0 and MASM. For modern builds, consider:

- **Quakespasm:** https://github.com/sezero/quakespasm (modern cross-platform port)
- **vkQuake:** https://github.com/Novum/vkQuake (Vulkan renderer)

For benchmark purposes, we use simplified test harnesses that compile only the relevant subsystems.

## Task Structure

Unlike Pygame tasks that include complete games, Quake tasks include:
- Specific source files with injected bugs/incomplete features
- Test harnesses that compile and test isolated subsystems
- Reference to the full Quake codebase for context

## Key Source Files by Subsystem

### Rendering
- `r_main.c` - Main rendering entry points
- `r_draw.c` - Primitive drawing
- `r_bsp.c` - BSP tree traversal
- `r_edge.c` - Edge-based rendering
- `r_light.c` - Lighting calculations
- `d_*.c` - Software rasterizer

### Physics/Movement
- `sv_phys.c` - Server-side physics
- `sv_move.c` - Movement helpers
- `world.c` - World collision

### Networking
- `net_main.c` - Network main
- `net_dgrm.c` - Datagram protocol
- `cl_input.c` - Client input handling
- `cl_parse.c` - Server message parsing

### Memory
- `zone.c` - Zone memory allocator

### Sound
- `snd_dma.c` - Sound DMA
- `snd_mix.c` - Sound mixing
