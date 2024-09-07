/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright 2024 NXP
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef HEADER_I_SYSTEM_ZEPHYR
#define HEADER_I_SYSTEM_ZEPHYR

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char byte;

void I_InitScreen_zephyr();

void I_CreateBackBuffer_zephyr();

int I_GetVideoWidth_zephyr();

int I_GetVideoHeight_zephyr();

void I_FinishUpdate_zephyr(const byte* srcBuffer, const byte* pallete, const unsigned int width, const unsigned int height);

void I_SetPallete_zephyr(const byte* pallete);

void I_ProcessKeyEvents();

int I_GetTime_zephyr(void);

void I_Error (const char *error, ...);

void I_Quit_zephyr();

unsigned short* I_GetBackBuffer();

unsigned short* I_GetFrontBuffer();

#ifdef __cplusplus
}
#endif


#endif
