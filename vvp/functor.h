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
#if !defined(WINNT)
#ident "$Id: functor.h,v 1.38 2001/11/07 03:34:42 steve Exp $"
#endif

# include  "pointers.h"

/*
 * Create a propagation event. The fun parameter points to the functor
 * to have its output propagated, and the delay is the delay to
 * schedule the propagation.
 */
extern void schedule_functor(functor_t fun, unsigned delay);

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
 * The output value (oval) is combined with the drive specifications
 * to make a fully strength aware output, as described below.
 *
 * OUTPUT STRENGTHS:
 *
 * The strength-aware outputs are specified as an 8 bit value, that is
 * two 4 bit numbers. The value is encoded with two drive strengths (0-7)
 * and two drive values (0 or 1). Each nibble contains three bits of
 * strength and one bit of value, like so: VSSS. The high nible has
 * the strength-value closest to supply1, and the low nibble has the
 * strength-value closest to supply0.
 *
 * The functor calculates, when it operates, a 4-value output into
 * oval and a fully strength aware value into ostr. The mode-0
 * functors use the odrive0 and odrive1 fields to form the strength
 * value.
 */

/*
 * signal strengths
 */

enum strength_e {
      HiZ = 0x00,
      Su0 = 0x77, /* Su0-Su0 */
      St0 = 0x66, /* St0-St0 */
      Pu0 = 0x55, /* Pu0-Pu0 */
      Su1 = 0x77|0x88, /* Su1 - Su1 */
      St1 = 0x66|0x88, /* St1 - St1 */
      Pu1 = 0x55|0x88, /* Pu1 - Pu1 */
      StX = 0x66|0x80, /* St0 - St1 */
};


/*
 * Initialize the functors address space. This function must be called
 * exactly once before any of the other functor functions may be
 * called.
 */
extern void functor_init(void);

/*
 * This function allocates a functor and returns the vvp_ipoint_t
 * address for it. Every call to functor_allocate is guaranteed to
 * return a different vvp_ipoint_t address. The ipoint port bits are 0.
 *
 * If the wid is >1, a bunch of contiguous functors is created, and
 * the return value is the address of the first in the vector.
 */
extern vvp_ipoint_t functor_allocate(unsigned wid);

/*
 * Given an ipoint_t pointer, return a C pointer to the functor. This
 * is like a pointer dereference. The point parameter must have been
 * returned from a previous call to functor_allocate.
 */

extern functor_t **functor_list;
static const unsigned functor_chunks = 0x400;

inline static functor_t functor_index(vvp_ipoint_t point)
{
      unsigned index1 = point/4/functor_chunks;
      unsigned index2 = (point/4) % functor_chunks;

      return functor_list[index1][index2];
}

/*
 * This function defines the functor object.  After allocation an ipoint, 
 * you must call this before functor_index() is called on it.
 */
extern void functor_define(vvp_ipoint_t point, functor_t obj);


/*
**                   The functor object
*/

struct functor_s {
      functor_s();
      virtual ~functor_s();
	/* This is the output for the device. */
      vvp_ipoint_t out;
	/* These are the input ports. */
      vvp_ipoint_t port[4];

	/* Input values without strengths. */
      unsigned ival       : 8;

    private:
	/* Output value (low bits) and drive1 and drive0 strength. */
      unsigned oval       : 2;
    protected:
      unsigned odrive0    : 3;
      unsigned odrive1    : 3;
    private:
	/* Strength form of the output value. */
      unsigned ostr       : 8;

      unsigned inhibit    : 1;

    public:
#if defined(WITH_DEBUG)
        /* True if this functor triggers a breakpoint. */
      unsigned breakpoint : 1;
#endif
      virtual void set(vvp_ipoint_t ipt, bool push, 
		       unsigned val, unsigned str = 0) = 0;

      inline unsigned char get() { return oval; }
      inline unsigned char get_ostr() { return ostr; }
      void put(vvp_ipoint_t ipt, unsigned val);
      void put_oval(bool push, unsigned val);
      void put_ostr(bool push, unsigned val, unsigned str);
      bool disable(vvp_ipoint_t ptr);
      bool enable(vvp_ipoint_t ptr);
      void propagate(bool push);
      void force(unsigned val, unsigned str);
};

/*
 *  Set the ival for input port ptr to value val.
 */

inline void functor_s::put(vvp_ipoint_t ptr, unsigned val)
{
      static const unsigned char ival_mask[4] = { 0xfc, 0xf3, 0xcf, 0x3f };
      unsigned pp = ipoint_port(ptr);
      unsigned char imask = ival_mask[pp];
      ival = (ival & imask) | ((val & 3) << (2*pp));
}

inline void functor_s::propagate(bool push)
{
      vvp_ipoint_t idx = out;
      while (idx) {
	    functor_t idxp = functor_index(idx);
	    idxp->set(idx, push, oval, ostr);
	    idx = idxp->port[ipoint_port(idx)];
	    
#if defined(WITH_DEBUG)
	    if (fp->breakpoint)
		  breakpoint();
#endif
      }
}

inline void functor_s::put_oval(bool push, unsigned val)
{
      switch (val) {
	  case 0:
	    ostr = 0x00 | (odrive0<<0) | (odrive0<<4);
	    break;
	  case 1:
	    ostr = 0x88 | (odrive1<<0) | (odrive1<<4);
	    break;
	  case 2:
	    ostr = 0x80 | (odrive0<<0) | (odrive1<<4);
	    break;
	  case 3:
	    ostr = 0x00;
	    break;
      }
      if (inhibit)
	    return;
      if (val != oval) {
	    oval = val;
	    if (push)
		  propagate(true);
	    else
		  schedule_functor(this, 0);
      }
}

inline void functor_s::put_ostr(bool push, unsigned val, unsigned str)
{
      if (val != oval || str != ostr) {      
	    ostr = str;
	    if (inhibit)
		  return;
	    oval = val;
	    if (push)
		  propagate(true);
	    else
		  schedule_functor(this, 0);
      }
}

/*
 * functor_set sets the addressed input to the specified value, and
 * calculates a new output value. If there is any propagation to do,
 * propagation events are created. Propagation calls further
 * functor_set methods for the functors connected to the output.
 *
 * The val contains 2 bits two represent the 4-value bit. The str
 * version is also passed, and typically just stored in the
 * functor.
 */

/*
 * Set the addressed bit of the functor, and recalculate the
 * output. If the output changes any, then generate the necessary
 * propagation events to pass the output on.
 */
inline static 
void functor_set(vvp_ipoint_t ptr, unsigned val, unsigned str, bool push)
{
      functor_t fp = functor_index(ptr);
      fp->set(ptr, push, val, str);

#if defined(WITH_DEBUG)
      if (fp->breakpoint)
	    breakpoint();
#endif
}

/*
 * Read the value of the functor. In fact, only the *value* is read --
 * the strength of that value is stripped off.
 */
inline static 
unsigned functor_get(vvp_ipoint_t ptr)
{
      functor_t fp = functor_index(ptr);
      return fp->get();
}


/*
 * When a propagation event happens, this function is called with the
 * address of the affected functor. It propagates the output to all
 * the inputs it is connected to, creating new propagation event on
 * the way.
 */
inline static
void functor_propagate(functor_t fp, bool push=true)
{
      fp->propagate(push);
}


//          Special infrastructure functor types


// The extra_outputs_functor_s class is for devices that require 
// multiple inputs and outputs.
// ->set redirects the job to the base_, who knows what shall be done.

struct extra_outputs_functor_s: public functor_s {
      extra_outputs_functor_s(vvp_ipoint_t b = 0) : base_(b) {}
      virtual ~extra_outputs_functor_s();
      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

      unsigned base_;
};

// extra_ports_functor_s redirects to base without setting the inputs.
// But base must be awayr that i may not match this.  This is used by 
// memory ports.

struct extra_ports_functor_s : public extra_outputs_functor_s
{
      extra_ports_functor_s(vvp_ipoint_t b = 0) : extra_outputs_functor_s(b) {}
      virtual ~extra_ports_functor_s();
      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
};

// The extra_inputs_functor_s class is for devices that require 
// multiple inputs but only one output
// ->set redirects the job to ->out, that knows what shall be done.

struct extra_inputs_functor_s: public functor_s {
      extra_inputs_functor_s(vvp_ipoint_t b = 0) { out = b; }
      virtual ~extra_inputs_functor_s();
      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
};

// edge_inputs_functor_s provides an old_ival
// it's up to the set() method to use it (UDP).
// The default set() is inherited from extra_inputs_functor_s.
struct edge_inputs_functor_s: public extra_inputs_functor_s
{
      edge_inputs_functor_s() : old_ival(2) {}
      virtual ~edge_inputs_functor_s();
      unsigned char old_ival;
};

/*
 * Vectors of functors
 */

extern unsigned vvp_fvector_size(vvp_fvector_t v);
extern vvp_ipoint_t vvp_fvector_get(vvp_fvector_t v, unsigned i);
extern void vvp_fvector_set(vvp_fvector_t v, unsigned i, vvp_ipoint_t p);
extern vvp_ipoint_t *vvp_fvector_member(vvp_fvector_t v, unsigned i);
extern vvp_fvector_t vvp_fvector_new(unsigned size);
extern vvp_fvector_t vvp_fvector_continuous_new(unsigned size, vvp_ipoint_t p);


/*
 * $Log: functor.h,v $
 * Revision 1.38  2001/11/07 03:34:42  steve
 *  Use functor pointers where vvp_ipoint_t is unneeded.
 *
 * Revision 1.37  2001/11/06 03:07:22  steve
 *  Code rearrange. (Stephan Boettcher)
 *
 * Revision 1.36  2001/11/01 03:00:19  steve
 *  Add force/cassign/release/deassign support. (Stephan Boettcher)
 *
 * Revision 1.35  2001/10/31 04:27:46  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.34  2001/10/27 03:43:56  steve
 *  Propagate functor push, to make assign better.
 *
 * Revision 1.33  2001/10/12 03:00:09  steve
 *  M42 implementation of mode 2 (Stephan Boettcher)
 *
 * Revision 1.32  2001/10/09 16:57:47  steve
 *  Collect functor reference handling into a single function. (Stephan Boettcher)
 *
 * Revision 1.31  2001/10/09 02:28:17  steve
 *  Add the PMOS and NMOS functor types.
 *
 * Revision 1.30  2001/08/08 01:05:06  steve
 *  Initial implementation of vvp_fvectors.
 *  (Stephan Boettcher)
 *
 * Revision 1.29  2001/07/28 03:12:39  steve
 *  Support C<su0> and C<su1> special symbols.
 *
 * Revision 1.28  2001/07/16 17:57:51  steve
 *  Merge sig and old_ival into union to save space.
 *
 * Revision 1.27  2001/07/13 03:02:34  steve
 *  Rewire signal callback support for fast lookup. (Stephan Boettcher)
 *
 * Revision 1.26  2001/06/21 22:54:12  steve
 *  Support cbValueChange callbacks.
 *
 * Revision 1.25  2001/06/19 03:01:10  steve
 *  Add structural EEQ gates (Stephan Boettcher)
 *
 * Revision 1.24  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 * Revision 1.23  2001/05/30 03:02:35  steve
 *  Propagate strength-values instead of drive strengths.
 *
 * Revision 1.22  2001/05/12 20:38:06  steve
 *  A resolver that understands some simple strengths.
 *
 * Revision 1.21  2001/05/09 04:23:18  steve
 *  Now that the interactive debugger exists,
 *  there is no use for the output dump.
 *
 * Revision 1.20  2001/05/09 02:53:25  steve
 *  Implement the .resolv syntax.
 *
 * Revision 1.19  2001/05/08 23:32:26  steve
 *  Add to the debugger the ability to view and
 *  break on functors.
 *
 *  Add strengths to functors at compile time,
 *  and Make functors pass their strengths as they
 *  propagate their output.
 *
 * Revision 1.18  2001/05/06 03:51:37  steve
 *  Regularize the mode-42 functor handling.
 *
 * Revision 1.17  2001/04/29 23:13:34  steve
 *  Add bufif0 and bufif1 functors.
 *
 * Revision 1.16  2001/04/26 15:52:22  steve
 *  Add the mode-42 functor concept to UDPs.
 *
 * Revision 1.15  2001/04/26 05:12:02  steve
 *  Implement simple MUXZ for ?: operators.
 *
 * Revision 1.14  2001/04/24 02:23:59  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 * Revision 1.13  2001/04/21 02:04:01  steve
 *  Add NAND and XNOR functors.
 *
 * Revision 1.12  2001/04/15 16:37:48  steve
 *  add XOR support.
 *
 * Revision 1.11  2001/04/14 05:10:56  steve
 *  support the .event/or statement.
 *
 * Revision 1.10  2001/04/04 17:43:19  steve
 *  support decimal strings from signals.
 *
 * Revision 1.9  2001/04/03 03:18:34  steve
 *  support functor_set push for blocking assignment.
 *
 * Revision 1.8  2001/04/01 21:31:46  steve
 *  Add the buf functor type.
 *
 * Revision 1.7  2001/03/29 03:46:36  steve
 *  Support named events as mode 2 functors.
 *
 * Revision 1.6  2001/03/26 04:00:39  steve
 *  Add the .event statement and the %wait instruction.
 */
#endif
