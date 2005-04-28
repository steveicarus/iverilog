#ifndef __functor_H
#define __functor_H
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: functor.h,v 1.57 2005/04/28 04:59:53 steve Exp $"
#endif

/* NOTE: THIS FILE IS BEOING PHASED OUT. IT'S FUNCTIONALITY IS OBSOLETE. */
# include  "pointers.h"
# include  "delay.h"

/*
 * The vvp_ipoint_t is an integral type that is 32bits. The low 2 bits
 * select the port of the referenced functor, and the remaining 30
 * index the functor itself. All together, the 32 bits can completely
 * identify any input of any functor.
 *
 * Outputs of functors are the heads of a linked list of all the
 * inputs that it is connected to. The vvp_ipoint_t in .out points to
 * the first port in the list. The .port[x] in turn points to the next
 * port, and so on. The last .port[x] contains the null vvp_ipoint_t
 * value zero (0). In this way, an output can fan out as wide as the
 * original design requires.
 *
 * Value Encoding
 *	1'b0  : 00
 *	1'b1  : 01
 *	1'bx  : 10
 *	1'bz  : 11
 *
 * DRIVE STRENGTHS:
 *
 * The normal functor is not aware of strengths. It
 * generates strength simply by virtue of having strength
 * specifications. The drive strength specification includes a drive0
 * and drive1 strength, each with 8 possible values (that can be
 * represented in 3 bits) as given in this table:
 *
 *    HiZ    = 0,
 *    SMALL  = 1,
 *    MEDIUM = 2,
 *    WEAK   = 3,
 *    LARGE  = 4,
 *    PULL   = 5,
 *    STRONG = 6,
 *    SUPPLY = 7
 *
 * The output value (cval) is combined with the drive specifications
 * to make a fully strength aware output, as described below.
 *
 * OUTPUT STRENGTHS:
 *
 * The strength-aware outputs are specified as an 8 bit value, that is
 * two 4 bit numbers. The value is encoded with two drive strengths (0-7)
 * and two drive values (0 or 1). Each nibble contains three bits of
 * strength and one bit of value, like so: VSSS. The high nibble has
 * the strength-value closest to supply1, and the low nibble has the
 * strength-value closest to supply0.
 *
 * The functor calculates, when it operates, a 4-value output into
 * oval and a fully strength aware value into ostr.  Functors with
 * fixed drive strength use the odrive0 and odrive1 fields to form the
 * strength value.
 */

/*
 * signal strengths
 */

enum strength_e {
      HiZ = 0x00,
      Su0 = 0x77, /* Su0-Su0 */
      St0 = 0x66, /* St0-St0 */
      Pu0 = 0x55, /* Pu0-Pu0 */
      We0 = 0x33, /* We0-We0 */
      Su1 = 0x77|0x88, /* Su1 - Su1 */
      St1 = 0x66|0x88, /* St1 - St1 */
      Pu1 = 0x55|0x88, /* Pu1 - Pu1 */
      We1 = 0x33|0x88, /* We1 - We1 */
      StX = 0x66|0x80, /* St0 - St1 */
};


/*
**                   The functor object
*/

struct functor_s {
      functor_s();
      virtual ~functor_s();

        /* delay object */
      vvp_delay_t delay;
	/* This is the output for the device. */
      vvp_ipoint_t out;
	/* These are the input ports. */
      vvp_ipoint_t port[4];

	/* Input values without strengths. */
      unsigned ival       : 8;

    private:
	/* Output value (low bits) and drive1 and drive0 strength. */
      unsigned cval       : 2;
    protected:
      unsigned odrive0    : 3;
      unsigned odrive1    : 3;
    private:
	/* Strength form of the output value. */
      unsigned cstr       : 8;

    protected:
      unsigned ostr       : 8;
      unsigned oval       : 2;

    private:
      unsigned inhibit    : 1;

    public:
      virtual void set(vvp_ipoint_t ipt, bool push,
		       unsigned val, unsigned str = 0) = 0;

      inline unsigned char get()      { return cval; }
      inline unsigned char get_str()  { return cstr; }
      inline unsigned char get_oval() { return oval; }
      inline unsigned char get_ostr() { return ostr; }

      void put(vvp_ipoint_t ipt, unsigned val);
      void put_oval(unsigned val,
		    bool push, bool nba_flag =false);
      void put_ostr(unsigned val, unsigned str,
		    bool push, bool nba_flag=false);
	// Schedule the functor to propagate. If the nba_flag is true,
	// then schedule this as a non-blocking
	// assignment. (sequential primitives use this feature.)
      void schedule(vvp_time64_t delay, bool nba_flag =false);

      bool disable(vvp_ipoint_t ptr);
      bool enable(vvp_ipoint_t ptr);
      void propagate(bool push);
      void propagate(unsigned val, unsigned str, bool push);
};


/*
 * $Log: functor.h,v $
 * Revision 1.57  2005/04/28 04:59:53  steve
 *  Remove dead functor code.
 *
 * Revision 1.56  2005/04/03 06:16:54  steve
 *  Remove dead fvectors class.
 */
#endif
