/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
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
 * DESCRIPTION:
 *      Rendering main loop and setup functions,
 *       utility functions (BSP, geometry, trigonometry).
 *      See tables.c, too.
 *
 *-----------------------------------------------------------------------------*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "d_net.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "r_plane.h"
#include "r_draw.h"
#include "m_bbox.h"
#include "r_sky.h"
#include "v_video.h"
#include "lprintf.h"
#include "st_stuff.h"
#include "i_main.h"
#include "i_system.h"
#include "g_game.h"
#include "tables.h"
#if defined(CONFIG_DOOM_CALC_TAN_TABLE) || defined(CONFIG_DOOM_CALC_SIN_TABLE)
#include <math.h>
#endif

#include "global_data.h"

// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW 2048

//
// R_Init
//

void R_Init (void)
{
  lprintf(LO_INFO, "R_LoadTrigTables");
  R_LoadTrigTables();
  lprintf(LO_INFO, "R_InitData");
  R_InitData();
  lprintf(LO_INFO, "R_InitPlanes");
  R_InitPlanes();
  lprintf(LO_INFO, "R_InitBuffer");
  R_InitBuffer();
}

//
// R_SetupFrame
//

void R_SetupFrame (player_t *player)
{
    viewx = player->mo->x;
    viewy = player->mo->y;
    viewz = player->viewz;
    viewangle = player->mo->angle;

    extralight = player->extralight;

    viewsin = finesine[viewangle>>ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle>>ANGLETOFINESHIFT];

    fullcolormap = &colormaps[0];

    if (player->fixedcolormap)
    {
        fixedcolormap = fullcolormap   // killough 3/20/98: use fullcolormap
                + player->fixedcolormap*256*sizeof(lighttable_t);
    }
    else
        fixedcolormap = 0;

    _g->validcount++;

    highDetail = _g->highDetail;
}

#ifdef CONFIG_DOOM_CALC_YSLOPE
fixed_t yslope[SCREENHEIGHT];
#endif 

#ifdef CONFIG_DOOM_CALC_DISTSCALE
fixed_t distscale[SCREENWIDTH];
#endif

#ifdef CONFIG_DOOM_CALC_XTOVIEWANGLE
angle_t xtoviewangle[SCREENWIDTH + 1];
#endif

//
// R_ExecuteSetViewSize
// Calculates lookup table values
//

void R_ExecuteSetViewSize (void)
{
  int i, x;
  static const fixed_t projectiony = ((SCREENHEIGHT * (SCREENWIDTH/2) * 320) / 200) / SCREENWIDTH * FRACUNIT;
  // thing clipping
  for (i=0 ; i<SCREENWIDTH ; i++)
    screenheightarray[i] = viewheight;

  fixed_t focallength;

#if defined(CONFIG_DOOM_CALC_TAN_TABLE) || defined(CONFIG_DOOM_CALC_SIN_TABLE)
    float	a;
    float	fv;
    int		t;
#endif 

#ifdef CONFIG_DOOM_CALC_TAN_TABLE
    // viewangle tangent table
    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
        a = (i-FINEANGLES/4+0.5)*M_PI*2/FINEANGLES;
        fv = FRACUNIT*tan (a);
        t = fv;
        finetangent[i] = t;
    }
#endif

#ifdef CONFIG_DOOM_CALC_SIN_TABLE
    for (i=0 ; i<5*FINEANGLES/4 ; i++)
    {
        // OPTIMIZE: mirror...
        a = (i+0.5)*M_PI*2/FINEANGLES;
        t = FRACUNIT*sin (a);
        finesine[i] = t;
    }
#endif


#ifdef CONFIG_DOOM_CALC_VIEWANGLETOX
  // Use tangent table to generate viewangletox:
  //  viewangletox will give the next greatest x
  //  after the view angle.
  //
  // Calc focallength
  //  so FIELDOFVIEW angles covers SCREENWIDTH.

  focallength = FixedDiv(centerxfrac, finetangent[FINEANGLES/4+FIELDOFVIEW/2]);

  for (i=0 ; i<FINEANGLES/2 ; i++)
    {
      int t;
      if (finetangent[i] > FRACUNIT*2)
        t = -1;
      else
        if (finetangent[i] < -FRACUNIT*2)
          t = SCREENWIDTH+1;
      else
        {
          t = FixedMul(finetangent[i], focallength);
          t = (centerxfrac - t + FRACUNIT-1) >> FRACBITS;
          if (t < -1)
            t = -1;
          else
            if (t > SCREENWIDTH+1)
              t = SCREENWIDTH+1;
        }
      viewangletox[i] = t;
    }
#endif

#ifdef CONFIG_DOOM_CALC_XTOVIEWANGLE
  for (x=0; x<=SCREENWIDTH; x++)
    {
      for (i=0; viewangletox[i] > x; i++)
        ;
      xtoviewangle[x] = (i<<ANGLETOFINESHIFT)-ANG90;
    }
#endif

  clipangle = xtoviewangle[0];


#ifdef CONFIG_DOOM_CALC_YSLOPE
  for (i=0 ; i<SCREENHEIGHT ; i++)
  {
    yslope[i] = 0;
  }

  // planes
  for (i=0 ; i<viewheight ; i++)
    {   // killough 5/2/98: reformatted
      fixed_t dy = D_abs(((i-viewheight/2)<<FRACBITS)+FRACUNIT/2);
      // proff 08/17/98: Changed for high-res
      yslope[i] = FixedDiv(projectiony, dy);
    }
#endif

#ifdef CONFIG_DOOM_CALC_DISTSCALE
  for (i=0 ; i<SCREENWIDTH ; i++)
    {
      fixed_t cosadj = D_abs(finecosine[xtoviewangle[i]>>ANGLETOFINESHIFT]);
      distscale[i] = FixedDiv(FRACUNIT,cosadj);
    }
#endif

}
