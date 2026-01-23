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
// BUGGY VERSION - Contains integer overflow bugs in audio mixing

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

BUG: This function accumulates samples without checking for overflow.
When many channels play simultaneously at high volume, the 32-bit
intermediate values can overflow, causing audio distortion.
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
        // BUG: No overflow checking on accumulation
        // When many channels mix together, this can overflow INT_MAX
        // Maximum single channel contribution: 127 * 31 * 8 = 31496
        // With 32 channels at max: 31496 * 32 = 1,007,872 (safe)
        // But with volume scaling and multiple sounds, overflow can occur
        paintbuffer[i].left += lscale[data];
        paintbuffer[i].right += rscale[data];
    }

    ch->pos += count;
}


/*
===============
SND_PaintChannelFrom16

Mix 16-bit mono samples into the paint buffer.

BUG: Same overflow issue as SND_PaintChannelFrom8, plus
the sample multiplication can overflow before the shift.
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
        // BUG: On 32-bit systems, this multiplication can overflow
        // data ranges from -32768 to 32767
        // volume can be up to 256
        // -32768 * 256 = -8,388,608 (fits in 32-bit)
        // But after accumulation from multiple channels, overflow occurs
        left = (data * leftvol) >> 8;
        right = (data * rightvol) >> 8;
        // BUG: No overflow checking on accumulation
        paintbuffer[i].left += left;
        paintbuffer[i].right += right;
    }

    ch->pos += count;
}


/*
===============
S_TransferPaintBuffer

Transfer the mixed audio to the output buffer.

BUG: The conversion from 32-bit to 16-bit doesn't properly clamp
values, causing wraparound distortion on loud sounds. Missing
the negative overflow check.
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

        // BUG: Only checks positive overflow, misses negative!
        // This causes wraparound for negative overflow values
        if (val > 0x7fff)
            val = 0x7fff;
        // BUG: Missing this check:
        // else if (val < -0x8000)
        //     val = -0x8000;

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
