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

# include  "npmos.h"
# include  "schedule.h"

void vvp_pmos_s::set(vvp_ipoint_t ptr, bool push, unsigned v, unsigned s)
{
      put(ptr, v);

      unsigned pp = ipoint_port(ptr);
      if (pp == 0) {

	    /* from IEEE 1384-1995 Table 7-8 */
	    static const unsigned char rmos_table[8]
		  = { 0, 1, 1, 2, 2, 3, 5, 5 };
	    /* just reduce SUPPLY to STRONG */
	    static const unsigned char mos_table[8]
		  = { 0, 1, 2, 3, 4, 5, 6, 6 };

	    const unsigned char *table = res ? rmos_table : mos_table;

	    unsigned vals = s & 0x88;
	    unsigned s1 = table[s & 0x7];
	    unsigned s2 = table[(s & 0x70) >> 4];

	    istr =  vals | s1 | (s2 << 4);
      }

      unsigned in0 = ival & 0x03;
      unsigned in1 = (ival >> 2) & 0x03;

      unsigned char outH = 0x88 | ((istr & 7)<<4) | (0);
      unsigned char outL = 0x00 | ((istr & 7)<<0) | (0);
      unsigned char outX = 0x80 | (istr & 7) | (istr & 0x70);

      in1 ^= pol;

      unsigned char val;
      unsigned char str;

      if (in1 == 0) {
	    // gate on; output follows input
	    val = in0;
	    str = istr;
      } else if (in0 == 3) {
	    val = 3;
	    str = HiZ;
      } else if (in1 == 2 || in1 == 3) {
	    // gate X or Z; output is undefined
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
      } else {
	    // gate off; output is high impedance
	    val = 3;
	    str = HiZ;
      }

      put_ostr(val, str, push);
}
