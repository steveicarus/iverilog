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
#ident "$Id: resolv.cc,v 1.4 2001/05/31 04:12:43 steve Exp $"
#endif

# include  "resolv.h"
# include  "schedule.h"

/*
 * A signal value is unambiguous if the top 4 bits and the bottom 4
 * bits are identical. This means that the VSSSvsss bits of the 8bit
 * value have V==v and SSS==sss.
 */
# define UNAMBIG(v)  (((v) & 0x0f) == (((v) >> 4) & 0x0f))

# define STREN1(v) ( ((v)&0x80)? ((v)&0xf0) : (0x70 - ((v)&0xf0)) )
# define STREN0(v) ( ((v)&0x08)? ((v)&0x0f) : (0x07 - ((v)&0x0f)) )

static unsigned blend(unsigned a, unsigned b)
{
      if (a == HiZ)
	    return b;

      if (b == HiZ)
	    return a;

      unsigned res = a;

      if (UNAMBIG(a) && UNAMBIG(b)) {

	      /* If both signals are unambiguous, simply choose
		 the stronger. If they have the same strength
		 but different values, then this becomes
		 ambiguous. */

	    if (a == b) {

		    /* values are equal. do nothing. */

	    } else if ((b&0x07) > (res&0x07)) {

		    /* New value is stronger. Take it. */
		  res = b;

	    } else if ((b&0x77) == (res&0x77)) {

		    /* Strengths are the same. Make value ambiguous. */
		  res = (res&0x70) | (b&0x07) | 0x80;

	    } else {

		    /* Must be res is the stronger one. */
	    }

      } else if (UNAMBIG(res) || UNAMBIG(b)) {

	      /* If one of the signals is unambiguous, then it
		 will sweep up the weaker parts of the ambiguous
		 signal. The result may be ambiguous, or maybe not. */

	    unsigned tmp = 0;

	    if ((res&0x70) > (b&0x70))
		  tmp |= res&0xf0;
	    else
		  tmp |= b&0xf0;

	    if ((res&0x07) > (b&0x07))
		  tmp |= res&0x0f;
	    else
		  tmp |= b&0x0f;

	    res = tmp;

      } else {

	      /* If both signals are ambiguous, then the result
		 has an even wider ambiguity. */

	    unsigned tmp = 0;

	    if (STREN1(b) > STREN1(res))
		  tmp |= b&0xf0;
	    else
		  tmp |= res&0xf0;

	    if (STREN0(b) < STREN0(res))
		  tmp |= b&0x0f;
	    else
		  tmp |= res&0x0f;

	    res = tmp;
      }


	/* Cannonicalize the HiZ value. */
      if ((res&0x77) == 0)
	    res = HiZ;

      return res;
}

/*
 * Resolve the strength values of the inputs, two at a time. Pairs of
 * inputs are resolved with the blend function, and the final value is
 * reduced to a 4-value result for propagation.
 */
void vvp_resolv_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      unsigned val = fp->istr[0];

      val = blend(val, fp->istr[1]);
      val = blend(val, fp->istr[2]);
      val = blend(val, fp->istr[3]);

      unsigned oval;
      if (val == HiZ) {
	    oval = 3;

      } else switch (val & 0x88) {
	  case 0x00:
	    oval = 0;
	    break;
	  case 0x88:
	    oval = 1;
	    break;
	  default:
	    oval = 2;
	    break;
      }

      fp->ostr = val;

	/* If the output changes, then create a propagation event. */
      if (oval != fp->oval) {
	    fp->oval = oval;
	    if (push)
		  functor_propagate(ptr);
	    else
		  schedule_functor(ptr, 0);
      }
}

/*
 * $Log: resolv.cc,v $
 * Revision 1.4  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 * Revision 1.3  2001/05/30 03:02:35  steve
 *  Propagate strength-values instead of drive strengths.
 *
 * Revision 1.2  2001/05/12 20:38:06  steve
 *  A resolver that understands some simple strengths.
 *
 * Revision 1.1  2001/05/09 02:53:53  steve
 *  Implement the .resolv syntax.
 *
 */

