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
#ident "$Id: npmos.cc,v 1.2 2001/10/11 18:20:51 steve Exp $"
#endif

# include  "npmos.h"
# include  "functor.h"
# include  "schedule.h"

void vvp_pmos_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      unsigned in0 = fp->ival & 0x03;
      unsigned in1 = (fp->ival >> 2) & 0x03;
      unsigned is0 = fp->istr[0];

      // Reduce SUPPLY to STRONG
      if ((is0 & 0x70) == 0x70)
	is0 &= 0xef;
      if ((is0 & 0x07) == 0x07)
	is0 &= 0xfe;

      unsigned char outH = 0x88 | ((is0 & 3)<<4) | (0);
      unsigned char outL = 0x00 | ((is0 & 3)<<0) | (0);
      unsigned char outX = 0x80 | (is0 & 3) | (is0 & 0x30);

      if (in0 == 3 || in1 == 1) {

	fp->oval = 3;
	fp->ostr = HiZ;

      } else {

	switch (in1) {

	  case 0:
	    fp->oval = in0;
	    fp->ostr = is0;
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
      }

      if (push)
	    functor_propagate(ptr);
      else
	    schedule_functor(ptr, 0);
}

void vvp_nmos_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      unsigned in0 = fp->ival & 0x03;
      unsigned in1 = (fp->ival >> 2) & 0x03;
      unsigned is0 = fp->istr[0];

      // Reduce SUPPLY to STRONG
      if ((is0 & 0x70) == 0x70)
	is0 &= 0xef;
      if ((is0 & 0x07) == 0x07)
	is0 &= 0xfe;

      unsigned char outH = 0x88 | ((is0 & 3)<<4) | (0);
      unsigned char outL = 0x00 | ((is0 & 3)<<0) | (0);
      unsigned char outX = 0x80 | (is0 & 3) | (is0 & 0x30);
      if (in0 == 3 || in1 == 0) {

	fp->oval = 3;
	fp->ostr = HiZ;

      } else {

        switch (in1) {

	  case 1:
	    fp->oval = in0;
	    fp->ostr = is0;
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
      }

      if (push)
	    functor_propagate(ptr);
      else
	    schedule_functor(ptr, 0);
}

/*
 * $Log: npmos.cc,v $
 * Revision 1.2  2001/10/11 18:20:51  steve
 *  npmos devices pass strength.
 *
 * Revision 1.1  2001/10/09 02:28:17  steve
 *  Add the PMOS and NMOS functor types.
 *
 * Revision 1.1  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 */

