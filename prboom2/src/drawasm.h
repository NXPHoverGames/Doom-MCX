// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: drawasm.h,v 1.1 2000/05/04 08:01:13 proff_fs Exp $
//
//  Sound server for LxDoom, based on the sound server sources released
//   with the original linuxdoom.
//  Copyright (C) 1993-1996 by id Software
//  Copyright (C) 1999 by Colin Phipps
//  
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------
//

#ifndef _DRAWASM_H_
#define _DRAWASM_H_

#if !defined(__ELF__) && !defined(__GNUC__)
        #define  R_DrawSpan     	_R_DrawSpan 
	#define  R_DrawColumn_Normal	_R_DrawColumn_Normal
	#define	 R_DrawTLColumn_Normal	_R_DrawTLColumn_Normal
	#define	 R_DrawColumn_HighRes	_R_DrawColumn_HighRes
	#define  R_DrawTLColumn_HighRes _R_DrawTLColumn_HighRes
	#define  ds_x1		_ds_x1
        #define  ds_x2          _ds_x2
        #define  ds_xfrac       _ds_xfrac
        #define  ds_yfrac       _ds_yfrac
        #define  ds_y           _ds_y
        #define  ds_source      _ds_source
        #define  ds_xstep       _ds_xstep
        #define  ds_ystep       _ds_ystep
        #define  ds_colormap    _ds_colormap
	#define  SCREENWIDTH	_SCREENWIDTH
	#define  dc_x		_dc_x
	#define	 dc_yh		_dc_yh
        #define  dc_yl          _dc_yl
	#define  dc_source	_dc_source
	#define  dc_iscale	_dc_iscale
	#define  dc_colormap	_dc_colormap
	#define  dc_texheight  	_dc_texheight
	#define  dc_texturemid  _dc_texturemid
        #define  ylookup        _ylookup
	#define  centery	_centery
	#define  tranmap	_tranmap
	#define  viewwindowx	_viewwindowx
#endif

#endif /* _DRAWASM_H_ */

//
// $Log: drawasm.h,v $
// Revision 1.1  2000/05/04 08:01:13  proff_fs
// Initial revision
//
// Revision 1.1  2000/05/01 15:36:38  Proff
// initial revision
//