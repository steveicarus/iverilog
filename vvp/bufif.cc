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
#ident "$Id: bufif.cc,v 1.4 2001/12/06 03:31:24 steve Exp $"
#endif

# include  "bufif.h"
# include  "functor.h"
# include  "schedule.h"

void vvp_bufif1_s::set(vvp_ipoint_t ptr, bool push, unsigned v, unsigned)
{
      put(ptr, v);

      unsigned in0 = ival & 0x03;
      unsigned in1 = (ival >> 2) & 0x03;

      unsigned char out0 = 0x00 | (odrive0<<0) | (odrive0<<4);
      unsigned char out1 = 0x88 | (odrive1<<0) | (odrive1<<4);
      unsigned char outX = 0x80 | (odrive1<<0) | (odrive0<<4);
      unsigned char outH = 0x88 | (0)          | (odrive1<<4);
      unsigned char outL = 0x00 | (odrive1<<0) | (0);

      unsigned val;
      unsigned str;

      switch (in1 ^ pol) {

	  case 1:
	    switch (in0) {
		case 0:
		  val = 0;
		  str = out0;
		  break;
		case 1:
		  val = 1;
		  str = out1;
		  break;
		default:
		  val = 2;
		  str = outX;
		  break;
	    }
	    break;

	  case 0:
	    val = 3;
	    str = HiZ;
	    break;

	  default:
	    val = 2;
	    switch (in0) {
		case 0:
		  str = outL;
		  break;
		case 1:
		  str = outH;
		  break;
		default:
		  str = outX;
		  break;
	    }
	    break;
      }

      put_ostr(val, str, push);
}

/*
 * $Log: bufif.cc,v $
 * Revision 1.4  2001/12/06 03:31:24  steve
 *  Support functor delays for gates and UDP devices.
 *  (Stephan Boettcher)
 *
 * Revision 1.3  2001/11/07 03:34:42  steve
 *  Use functor pointers where vvp_ipoint_t is unneeded.
 *
 * Revision 1.2  2001/10/31 04:27:46  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.1  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 */

