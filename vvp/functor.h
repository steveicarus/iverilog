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
#ident "$Id: functor.h,v 1.32 2001/10/09 16:57:47 steve Exp $"
#endif

# include  "pointers.h"

/*
 *
 * The major mode is selected by the mode parameter.
 *
 * MODE 0: TABLE MODE FUNCTORS
 *
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
 * The function of the functor is defined by the table/event
 * union. Normally, the truth table is the behavior and the functor
 * output value is picked from the lookup table that the table pointer
 * points to.
 *
 * MODE 1: EDGE EVENT FUNCTORS
 *
 * These functors take inputs like mode 0 functors, but the input is
 * compared with the preveous input for that bit, and the results of
 * that comparison are used to detect edges. The functor may be
 * programmed to detect posedge, negedge, or any edge events. These
 * functors can have %wait instructions waiting on them.
 *
 * MODE 2: NAMED EVENT FUNCTORS
 *
 * These functors do not bother to check for edges. Any event on the
 * input causes an event to be detected. Like mode-1 functors, these
 * can have %wait instructions waiting on them. Mode-2 functors do not
 * have structural inputs, however. They take their inputs from %set
 * instructions.
 *
 * Mode-2 events can also be used to combine other mode-1 and mode-2
 * functors by setting their outputs to put to the mode-2
 * functor. Since the mode-2 functor does not take input, any number
 * of mode-1 and mode-2 functors may point in.
 *
 * MODE 42: LIFE, THE UNIVERSE AND EVERYTHING ELSE
 *
 * These functors are and escape for all other behaviors. This mode
 * supports arbitrary complex behavior by replacing the truth table
 * with a struct vvp_fobj_s pointer. This abstract class has virtual
 * methods for receiving and retrieving values. See the vvp_fobj_s
 * definition below.
 *
 * DRIVE STRENGTHS:
 *
 * The normal functor (modes 0, 1 and 2) is not aware of strengths. It
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

struct functor_s {
	/* This is the truth table for the device */
      union {
	    vvp_truth_t table;
	    vvp_event_t event;
            struct vvp_fobj_s *obj;
      };

	/* This is the output for the device. */
      vvp_ipoint_t out;
	/* These are the input ports. */
      vvp_ipoint_t port[4];
	/* Input with strengths, for strength aware functors. */
      unsigned char istr[4];
	/* Input values without strengths. */
      unsigned ival       : 8;
	/* Output value (low bits) and drive1 and drive0 strength. */
      unsigned oval       : 2;
      unsigned odrive0    : 3;
      unsigned odrive1    : 3;
	/* Strength form of the output value. */
      unsigned ostr       : 8;
#if defined(WITH_DEBUG)
	/* True if this functor triggers a breakpoint. */
      unsigned breakpoint : 1;
#endif

	/* functor mode:  0 == table ;  1 == event ; 2 == named event */
      unsigned mode       : 2;
      union {
 	    unsigned char old_ival; // mode 3
      };
};

typedef struct functor_s *functor_t;

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
 * This a an `obj' structute for mode-42 functors.  
 * Each instance implements the get and set methods in a type specific
 * way, so that this represents a completely general functor.
 *
 * ::set(...)
 *
 * This method is called when any of the 4 inputs of the functor
 * receives a bit value. This method is called even if the set value
 * is the same as the existing value.
 *
 * ::get(...)
 *
 * This method is called to pull the "value" of the functor. Normally,
 * there is not much of a trick to this, but some types might need to
 * do complex things here, like look up a memory index. Anyhow, this
 * method must be idempotent, because I'm only going to tell you that
 * it happens when it happens.
 */

#define M42 3

struct vvp_fobj_s {
      virtual unsigned get(vvp_ipoint_t i, functor_t f);
      virtual void set(vvp_ipoint_t i, functor_t f, bool push) =0;
};


/*
 * If functor mode is 1, the event member is valid and the vvp_event_s
 * points to the extended event information.
 */
extern const unsigned char vvp_edge_posedge[16];
extern const unsigned char vvp_edge_negedge[16];
extern const unsigned char vvp_edge_anyedge[16];

struct vvp_event_s {
      vthread_t threads;
      unsigned char ival;
      const unsigned char*vvp_edge_tab;
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
 * This function is used by the compile time to initialize the value
 * of an input, and by the run time to manipulate the bits of the
 * input in a uniform manner.
 *
 * The val parameter is the 2bit representation of the input value,
 * and the str is a strength aware version.
 */
extern void functor_put_input(functor_t fp, unsigned pp,
			      unsigned val, unsigned str);

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
extern void functor_set(vvp_ipoint_t point, unsigned val,
			unsigned str, bool push);

/*
 * Read the value of the functor. In fact, only the *value* is read --
 * the strength of that value is stripped off.
 */
extern unsigned functor_get(vvp_ipoint_t ptr);

/*
 * When a propagation event happens, this function is called with the
 * address of the affected functor. It propagates the output to all
 * the inputs it is connected to, creating new propagation event on
 * the way.
 */
extern void functor_propagate(vvp_ipoint_t ptr);
/*
 * Given an ipoint_t pointer, return a C pointer to the functor. This
 * is like a pointer dereference. The point parameter must have been
 * returned from a previous call to functor_allocate.
 */
extern functor_t functor_index(vvp_ipoint_t point);

/*
 * This is a convenience function that returns the current output
 * value of the functor.
 */
inline unsigned functor_oval(vvp_ipoint_t fptr)
{
      functor_t fp = functor_index(fptr);
      return fp->oval & 3;
}

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
 * M42 functor type for callback events
 */

struct vvp_cb_fobj_s: public vvp_fobj_s {
      virtual void set(vvp_ipoint_t i, functor_t f, bool push);
      struct __vpiCallback *cb_handle;
      unsigned permanent : 1;
};

struct vvp_cb_fobj_s *vvp_fvector_make_callback(vvp_fvector_t vec,
						const unsigned char *edge = 0);

extern const unsigned char ft_AND[];
extern const unsigned char ft_BUF[];
extern const unsigned char ft_BUFIF0[];
extern const unsigned char ft_BUFIF1[];
extern const unsigned char ft_PMOS[];
extern const unsigned char ft_NMOS[];
extern const unsigned char ft_MUXZ[];
extern const unsigned char ft_EEQ[];
extern const unsigned char ft_NAND[];
extern const unsigned char ft_NOR[];
extern const unsigned char ft_NOT[];
extern const unsigned char ft_OR[];
extern const unsigned char ft_XNOR[];
extern const unsigned char ft_XOR[];
extern const unsigned char ft_var[];

/*
 * $Log: functor.h,v $
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
