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
#ifdef HAVE_CVS_IDENT
#ident "$Id: bufif.cc,v 1.9 2002/09/06 04:56:28 steve Exp $"
#endif

# include  "bufif.h"
# include  "functor.h"
# include  "schedule.h"
# include  "statistics.h"

vvp_bufif_s::vvp_bufif_s(bool en_invert, bool out_invert,
			 unsigned str0, unsigned str1)
: pol_(en_invert? 1 : 0), inv_(out_invert? 1 : 0)
{
      odrive0 = str0;
      odrive1 = str1;
      count_functors_bufif += 1;
}

void vvp_bufif_s::set(vvp_ipoint_t ptr, bool push, unsigned v, unsigned)
{
      put(ptr, v);

      unsigned in0 = ival & 0x03;
      unsigned in1 = (ival >> 2) & 0x03;

      unsigned char out0 = 0x00 | (odrive0<<0) | (odrive0<<4);
      unsigned char out1 = 0x88 | (odrive1<<0) | (odrive1<<4);
      unsigned char outX = 0x80 | (odrive0<<0) | (odrive1<<4);
      unsigned char outH = 0x80 | (0)          | (odrive1<<4);
      unsigned char outL = 0x80 | (odrive0<<0) | (0);

      unsigned val;
      unsigned str;

      switch (in1 ^ pol_) {

	  case 1:
	    switch (in0 ^ inv_) {
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

	      /* The control input is x or z, so the output is H or
		 L, depending on the (possibly inverted) input. This
		 is not the same as X, as it is a combination of the
		 drive strength of the output and HiZ. */
	  default:
	    switch (in0 ^ inv_) {
		case 0:
		  val = 2;
		  str = outL;
		  break;
		case 1:
		  val = 2;
		  str = outH;
		  break;
		default:
		  val = 2;
		  str = outX;
		  break;
	    }
	    break;
      }

      put_ostr(val, str, push);
}

/*
 * $Log: bufif.cc,v $
 * Revision 1.9  2002/09/06 04:56:28  steve
 *  Add support for %v is the display system task.
 *  Change the encoding of H and L outputs from
 *  the bufif devices so that they are logic x.
 *
 * Revision 1.8  2002/08/12 01:35:07  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.7  2002/07/05 20:08:44  steve
 *  Count different types of functors.
 *
 * Revision 1.6  2001/12/19 23:43:03  steve
 *  clarify bufif output strenghts.
 *
 * Revision 1.5  2001/12/14 06:03:17  steve
 *  Arrange bufif to support notif as well.
 *
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

