#ifndef __functor_H
#define __functor_H
/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include  "pointers.h"
# include  "delay.h"
# include  <assert.h>

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
** Return the number of allocated functors
*/
extern unsigned functor_limit();

/*
 * Given an ipoint_t pointer, return a C pointer to the functor. This
 * is like a pointer dereference. The point parameter must have been
 * returned from a previous call to functor_allocate.
 */

extern functor_t **functor_list;
static const unsigned functor_chunk_size = 0x400;

inline static functor_t functor_index(vvp_ipoint_t point)
{
      unsigned index1 = point/4/functor_chunk_size;
      unsigned index2 = (point/4) % functor_chunk_size;

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
      propagate(get_oval(), get_ostr(), push);
}

inline void functor_s::put_oval(unsigned val, bool push, bool nba_flag)
{
      unsigned char str;
      switch (val) {
	  case 0:
	    str = 0x00 | (odrive0<<0) | (odrive0<<4);
	    break;
	  case 1:
	    str = 0x88 | (odrive1<<0) | (odrive1<<4);
	    break;
	  case 2:
	    str = 0x80 | (odrive0<<0) | (odrive1<<4);
	    break;
	  default:
	    str = 0x00;
	    break;
      }

      put_ostr(val, str, push, nba_flag);
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

//          Special infrastructure functor types

/*
 * A "waitable" functor is one that the %wait instruction can wait
 * on. This includes the infrastructure needed to hold threads.
 */
struct waitable_hooks_s {
      vthread_t threads;
};


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
// But base must be aware that i may not match this.  This is used by
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

inline static
unsigned char functor_get_inputs(vvp_ipoint_t ip)
{
  functor_t fp = functor_index(ip);
  assert(fp);
  return fp->ival;
}

inline static
unsigned char functor_get_input(vvp_ipoint_t ip)
{
  unsigned char bits = functor_get_inputs(ip);
  return (bits >> (2*ipoint_port(ip))) & 3;
}

#endif
