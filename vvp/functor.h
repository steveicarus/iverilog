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
#ident "$Id: functor.h,v 1.15 2001/04/26 05:12:02 steve Exp $"
#endif

# include  "pointers.h"
# include  <stdio.h>


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
 * These fuctors do not bother to check for edges. Any event on the
 * input causes an event to be detected. Like mode-1 functors, these
 * can have %wait instructions waiting on them. Mode-2 functors do not
 * have structural inputs, however. They take their inputs from %set
 * instructions.
 *
 * Mode-2 events can also be used to combine other mode-1 and mode-2
 * functors by setting their outputs to put to the mode-2
 * functor. Since the mode-2 functor does not take input, any number
 * of mode-1 and mode-2 functors may point in.
 */

struct functor_s {
	/* This is the truth table for the device */
      union {
	    vvp_truth_t table;
	    vvp_event_t event;
	    struct vvp_udp_s *udp; // mode 3
      };

	/* This is the output for the device. */
      vvp_ipoint_t out;
	/* These are the input ports. */
      vvp_ipoint_t port[4];
	/* These are the input values. */
      unsigned char ival;
      unsigned char oval;
	/* functor mode:  0 == table ;  1 == event ; 2 == named event */
	/*                3 == udp                                    */
      unsigned char mode;
      union {
 	    unsigned char old_ival; // mode 3
      };
};

typedef struct functor_s *functor_t;

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
 * functor_set sets the addressed input to the specified value, and
 * calculates a new output value. If there is any propagation to do,
 * propagation events are created.
 */
extern void functor_set(vvp_ipoint_t point, unsigned val, bool push=false);

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
 * Dump a readable version of the functor address space to the file.
 */
extern void functor_dump(FILE*fd);


extern const unsigned char ft_AND[];
extern const unsigned char ft_BUF[];
extern const unsigned char ft_MUXZ[];
extern const unsigned char ft_NAND[];
extern const unsigned char ft_NOR[];
extern const unsigned char ft_NOT[];
extern const unsigned char ft_OR[];
extern const unsigned char ft_XNOR[];
extern const unsigned char ft_XOR[];
extern const unsigned char ft_var[];

/*
 * $Log: functor.h,v $
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
