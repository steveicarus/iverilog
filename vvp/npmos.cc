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
#ident "$Id: npmos.cc,v 1.4 2001/10/24 03:18:52 steve Exp $"
#endif

# include  "npmos.h"
# include  "functor.h"
# include  "schedule.h"

/* from IEEE 1384-1995 Table 7-8 */
static unsigned char rmos_table[8] = { 0, 1, 1, 2, 2, 3, 5, 5 };
/* just reduce SUPPLY to STRONG */
static unsigned char mos_table[8] = { 0, 1, 2, 3, 4, 5, 6, 6 };

static inline unsigned reduce_strength(unsigned is0, unsigned char *table)
{
      unsigned vals = is0 & 0x88;
      unsigned s1 = table[is0 & 0x7];
      unsigned s2 = table[(is0 & 0x70) >> 4];

      return vals | s1 | (s2 << 4);
}

static void mos_set(vvp_ipoint_t ptr, functor_t fp, bool push, unsigned in1_on, unsigned char *table)
{
      unsigned in0 = fp->ival & 0x03;
      unsigned in1 = (fp->ival >> 2) & 0x03;
      unsigned is0 = reduce_strength(fp->istr[0], table);
  
      unsigned char outH = 0x88 | ((is0 & 7)<<4) | (0);
      unsigned char outL = 0x00 | ((is0 & 7)<<0) | (0);
      unsigned char outX = 0x80 | (is0 & 7) | (is0 & 0x70);

      if (in1 == in1_on) {
	    // gate on; output follows input
	    fp->oval = in0;
	    fp->ostr = is0;
      } else if (in1 == 2 || in1 == 3) {
	    // gate X or Z; output is undefined
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
      } else {
	    // gate off; output is high impedance
	    fp->oval = 3;
	    fp->ostr = HiZ;
      }

      if (push)
	    functor_propagate(ptr);
      else
	    schedule_functor(ptr, 0);
}

void vvp_pmos_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      mos_set(ptr, fp, push, 0, mos_table);
}

void vvp_nmos_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      mos_set(ptr, fp, push, 1, mos_table);
}

void vvp_rpmos_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      mos_set(ptr, fp, push, 0, rmos_table);
}

void vvp_rnmos_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      mos_set(ptr, fp, push, 1, rmos_table);
}

/*
 * $Log: npmos.cc,v $
 * Revision 1.4  2001/10/24 03:18:52  steve
 *  npmos outputs have 3bit strengths, not 2.
 *
 * Revision 1.3  2001/10/18 17:30:26  steve
 *  Support rnpmos devices. (Philip Blundell)
 *
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

