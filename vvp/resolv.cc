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

# include  "resolv.h"
# include  "schedule.h"
# include  "statistics.h"
# include  <assert.h>

/*
 * A signal value is unambiguous if the top 4 bits and the bottom 4
 * bits are identical. This means that the VSSSvsss bits of the 8bit
 * value have V==v and SSS==sss.
 */
# define UNAMBIG(v)  (((v) & 0x0f) == (((v) >> 4) & 0x0f))

# define STREN1(v) ( ((v)&0x80)? ((v)&0xf0) : (0x70 - ((v)&0xf0)) )
# define STREN0(v) ( ((v)&0x08)? ((v)&0x0f) : (0x07 - ((v)&0x0f)) )

# include <iostream>
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

      } else if (UNAMBIG(res)) {
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

      } else if (UNAMBIG(b)) {

	      /* If one of the signals is unambiguous, then it
		 will sweep up the weaker parts of the ambiguous
		 signal. The result may be ambiguous, or maybe not. */

	    unsigned tmp = 0;

	    if ((b&0x70) > (res&0x70))
		  tmp |= b&0xf0;
	    else
		  tmp |= res&0xf0;

	    if ((b&0x07) > (res&0x07))
		  tmp |= b&0x0f;
	    else
		  tmp |= res&0x0f;

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


	/* Canonicalize the HiZ value. */
      if ((res&0x77) == 0)
	    res = HiZ;

      return res;
}

resolv_functor_s::resolv_functor_s(unsigned char pull)
{
      count_functors_resolv += 1;
      istr[0]=istr[1]=istr[2]=istr[3]=StX;
      hiz_ = pull;
}

resolv_functor_s::~resolv_functor_s()
{
}

/*
 * Resolve the strength values of the inputs, two at a time. Pairs of
 * inputs are resolved with the blend function, and the final value is
 * reduced to a 4-value result for propagation.
 */
void resolv_functor_s::set(vvp_ipoint_t i, bool push, unsigned, unsigned str)
{
      unsigned pp = ipoint_port(i);
      istr[pp] = str;

      unsigned sval = hiz_;
      sval = blend(sval, istr[0]);
      sval = blend(sval, istr[1]);
      sval = blend(sval, istr[2]);
      sval = blend(sval, istr[3]);

      unsigned val;
      if (sval == HiZ) {
	    val = 3;

      } else switch (sval & 0x88) {
	  case 0x00:
	    val = 0;
	    break;
	  case 0x88:
	    val = 1;
	    break;
	  default:
	    val = 2;
	    break;
      }

	/* If the output changes, then create a propagation event. */

      // Do not propagate (push).  Why?  Because if, for example, a
      // clock buffer is modeled as parallel inverters, the output
      // must not show 'bx transitions when the inverters all propagate
      // at the same time.

      put_ostr(val, sval, false);
}
