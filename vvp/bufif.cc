/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT)
#ident "$Id: bufif.cc,v 1.1 2001/05/31 04:12:43 steve Exp $"
#endif

# include  "bufif.h"
# include  "functor.h"
# include  "schedule.h"

void vvp_bufif0_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      unsigned in0 = fp->ival & 0x03;
      unsigned in1 = (fp->ival >> 2) & 0x03;

      unsigned char out0 = 0x00 | (fp->odrive0<<0) | (fp->odrive0<<4);
      unsigned char out1 = 0x88 | (fp->odrive1<<0) | (fp->odrive1<<4);
      unsigned char outX = 0x80 | (fp->odrive1<<0) | (fp->odrive0<<4);
      unsigned char outH = 0x88 | (fp->odrive1<<4) | (0);
      unsigned char outL = 0x00 | (fp->odrive1<<0) | (0);

      switch (in1) {

	  case 0:
	    switch (in0) {
		case 0:
		  fp->oval = 0;
		  fp->ostr = out0;
		  break;
		case 1:
		  fp->oval = 1;
		  fp->ostr = out1;
		  break;
		default:
		  fp->oval = 2;
		  fp->ostr = outX;
		  break;
	    }
	    break;

	  case 1:
	    fp->oval = 3;
	    fp->ostr = HiZ;
	    break;

	  default:
	    fp->oval = 2;
	    switch (in0) {
		case 0:
		  fp->ostr = outL;
		  break;
		case 1:
		  fp->ostr = outH;
		  break;
		default:
		  fp->ostr = outX;
		  break;
	    }
	    break;
      }

      if (push)
	    functor_propagate(ptr);
      else
	    schedule_functor(ptr, 0);
}

void vvp_bufif1_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      unsigned in0 = fp->ival & 0x03;
      unsigned in1 = (fp->ival >> 2) & 0x03;

      unsigned char out0 = 0x00 | (fp->odrive0<<0) | (fp->odrive0<<4);
      unsigned char out1 = 0x88 | (fp->odrive1<<0) | (fp->odrive1<<4);
      unsigned char outX = 0x80 | (fp->odrive1<<0) | (fp->odrive0<<4);
      unsigned char outH = 0x88 | (fp->odrive1<<4) | (0);
      unsigned char outL = 0x00 | (fp->odrive1<<0) | (0);

      switch (in1) {

	  case 1:
	    switch (in0) {
		case 0:
		  fp->oval = 0;
		  fp->ostr = out0;
		  break;
		case 1:
		  fp->oval = 1;
		  fp->ostr = out1;
		  break;
		default:
		  fp->oval = 2;
		  fp->ostr = outX;
		  break;
	    }
	    break;

	  case 0:
	    fp->oval = 3;
	    fp->ostr = HiZ;
	    break;

	  default:
	    fp->oval = 2;
	    switch (in0) {
		case 0:
		  fp->ostr = outL;
		  break;
		case 1:
		  fp->ostr = outH;
		  break;
		default:
		  fp->ostr = outX;
		  break;
	    }
	    break;
      }

      if (push)
	    functor_propagate(ptr);
      else
	    schedule_functor(ptr, 0);
}

/*
 * $Log: bufif.cc,v $
 * Revision 1.1  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 */

