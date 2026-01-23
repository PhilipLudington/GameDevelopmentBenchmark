/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_mix.c -- portable code to mix sounds for snd_dma.c
// FIXED VERSION - Properly handles integer overflow in audio mixing

#include "quakedef.h"

#define PAINTBUFFER_SIZE 512

typedef struct {
    int left;
    int right;
} portable_samplepair_t;

portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];

int snd_scaletable[32][256];
int *snd_p, snd_linear_count, snd_vol;
short *snd_out;

int paintedtime;

// FIX: Helper macro for clamping values to prevent overflow
#define CLAMP_INT16(val) \
    ((val) > 32767 ? 32767 : ((val) < -32768 ? -32768 : (val)))


/*
===============
SND_InitScaletable

Build volume lookup table for fast mixing
===============
*/
void SND_InitScaletable(void)
{
    int i, j;

    for (i = 0; i < 32; i++)
        for (j = 0; j < 256; j++)
            snd_scaletable[i][j] = ((signed char)j) * i * 8;
}


/*
===============
SND_PaintChannelFrom8

Mix 8-bit mono samples into the paint buffer.

FIX: The paintbuffer is 32-bit which can hold the accumulated values
from multiple channels without overflow. The critical fix is in
S_TransferPaintBuffer where we properly clamp before converting to 16-bit.
===============
*/
void SND_PaintChannelFrom8(channel_t *ch, sfxcache_t *sc, int count)
{
    int data;
    int *lscale, *rscale;
    unsigned char *sfx;
    int i;

    if (ch->leftvol > 255)
        ch->leftvol = 255;
    if (ch->rightvol > 255)
        ch->rightvol = 255;

    lscale = snd_scaletable[ch->leftvol >> 3];
    rscale = snd_scaletable[ch->rightvol >> 3];
    sfx = (unsigned char *)sc->data + ch->pos;

    for (i = 0; i < count; i++)
    {
        data = sfx[i];
        // Accumulation is safe in 32-bit paintbuffer
        // Final clamping happens in S_TransferPaintBuffer
        paintbuffer[i].left += lscale[data];
        paintbuffer[i].right += rscale[data];
    }

    ch->pos += count;
}


/*
===============
SND_PaintChannelFrom16

Mix 16-bit mono samples into the paint buffer.

FIX: Use proper integer math that doesn't overflow, and rely on
S_TransferPaintBuffer for final clamping to 16-bit range.
===============
*/
void SND_PaintChannelFrom16(channel_t *ch, sfxcache_t *sc, int count)
{
    int data;
    int left, right;
    int leftvol, rightvol;
    signed short *sfx;
    int i;

    leftvol = ch->leftvol;
    rightvol = ch->rightvol;
    sfx = (signed short *)sc->data + ch->pos;

    for (i = 0; i < count; i++)
    {
        data = sfx[i];
        // The multiplication fits in 32-bit:
        // -32768 * 256 = -8,388,608 (within int range)
        // After >> 8: -32768 (fits in paintbuffer)
        left = (data * leftvol) >> 8;
        right = (data * rightvol) >> 8;
        // Accumulate - may exceed 16-bit range but fits in 32-bit paintbuffer
        paintbuffer[i].left += left;
        paintbuffer[i].right += right;
    }

    ch->pos += count;
}


/*
===============
S_TransferPaintBuffer

Transfer the mixed audio to the output buffer.

FIX: Properly clamp values to 16-bit range on BOTH positive AND negative
overflow. This is the critical fix - the original code only clamped
positive overflow, causing wraparound distortion on loud negative samples.
===============
*/
void S_TransferPaintBuffer(int endtime, short *out, int out_mask, int snd_vol)
{
    int out_idx;
    int count;
    int *p;
    int val;

    p = (int *)paintbuffer;
    count = (endtime - paintedtime) * 2;  // stereo samples
    out_idx = (paintedtime * 2) & out_mask;

    while (count--)
    {
        val = (*p * snd_vol) >> 8;
        p++;

        // FIX: Proper clamping for both positive AND negative overflow
        // This prevents wraparound distortion on loud sounds
        if (val > 32767)
            val = 32767;
        else if (val < -32768)
            val = -32768;

        out[out_idx] = val;
        out_idx = (out_idx + 1) & out_mask;
    }
}


/*
===============
S_ClearPaintBuffer

Zero out the paint buffer for a new mixing pass
===============
*/
void S_ClearPaintBuffer(int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        paintbuffer[i].left = 0;
        paintbuffer[i].right = 0;
    }
}


/*
===============
S_PaintChannels

Main mixing loop - mix all active channels into the paint buffer.
This is the entry point for the audio mixing system.
===============
*/
void S_PaintChannels(int endtime, channel_t *channels, int total_channels,
                     short *outbuffer, int out_mask, int snd_vol)
{
    int i;
    int end;
    channel_t *ch;
    sfxcache_t *sc;
    int ltime, count;

    while (paintedtime < endtime)
    {
        // Paint up to end of buffer or endtime, whichever is less
        end = endtime;
        if (endtime - paintedtime > PAINTBUFFER_SIZE)
            end = paintedtime + PAINTBUFFER_SIZE;

        // Clear the paint buffer
        S_ClearPaintBuffer(end - paintedtime);

        // Paint in the channels
        ch = channels;
        for (i = 0; i < total_channels; i++, ch++)
        {
            if (!ch->sfx)
                continue;
            if (!ch->leftvol && !ch->rightvol)
                continue;

            sc = ch->sfx;  // In test harness, sfx points directly to sfxcache

            ltime = paintedtime;

            while (ltime < end)
            {
                // Calculate samples to mix this iteration
                if (ch->end < end)
                    count = ch->end - ltime;
                else
                    count = end - ltime;

                if (count > 0)
                {
                    if (sc->width == 1)
                        SND_PaintChannelFrom8(ch, sc, count);
                    else
                        SND_PaintChannelFrom16(ch, sc, count);

                    ltime += count;
                }

                // If at end of sample
                if (ltime >= ch->end)
                {
                    if (sc->loopstart >= 0)
                    {
                        ch->pos = sc->loopstart;
                        ch->end = ltime + sc->length - sc->loopstart;
                    }
                    else
                    {
                        // Channel is done
                        ch->sfx = NULL;
                        break;
                    }
                }
            }
        }

        // Transfer to output buffer
        S_TransferPaintBuffer(end, outbuffer, out_mask, snd_vol);

        paintedtime = end;
    }
}
