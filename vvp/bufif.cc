/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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
