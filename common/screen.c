/*
 * Copyright (c) 2007, 2008 University of Tsukuba
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Tsukuba nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "types.h"

#define GREEN    0x2
#define MAX_COL  80		// Maximum number of columns 
#define MAX_ROW  25	    // Maximum number of rows 
#define VRAM_SIZE (MAX_COL*MAX_ROW)	// Size of screen, in short's 
#define DEF_VRAM_BASE 0xb8000	// Default base for video memory

static unsigned char curr_col = 0;
static unsigned char curr_row = 0;

// Write character at current screen location
#define PUT(c) ( ((unsigned short *) (DEF_VRAM_BASE)) \
    [(curr_row * MAX_COL) + curr_col] = (GREEN << 8) | (c))

// Place a character on next screen position
static void cons_putc(int c)
{
    switch (c) 
    {
    case '\t':
        do 
        {
            cons_putc(' ');
        } while ((curr_col % 8) != 0);
        break;
    case '\r':
        curr_col = 0;
        break;
    case '\n':
        curr_row += 1;
        if (curr_row >= MAX_ROW) 
        {
            curr_row = 1;
        }
        break;
    case '\b':
        if (curr_col > 0) 
        {
            curr_col -= 1;
            PUT(' ');
        }
        break;
    default:
        PUT(c);
        curr_col += 1;
        if (curr_col >= MAX_COL) 
        {
            curr_col = 0;
            curr_row += 1;
            if (curr_row >= MAX_ROW) 
            {
                curr_row = 1;
            }
        }
    };
}

void putchar( int c )
{
    if (c == '\n') 
        cons_putc('\r');
    cons_putc(c);
}

void clear_screen()
{
    curr_col = 0;
    curr_row = 1;
    
    int i;
    for (i = 0; i < VRAM_SIZE; i++)
        cons_putc(' ');
    
    curr_col = 0;
    curr_row = 1;
}
void clear()
{
    curr_col = 0;
    curr_row = 0;
    
    int i;
    for (i = 0; i < VRAM_SIZE; i++)
        cons_putc(' ');
    
    curr_col = 0;
    curr_row = 0;
}

