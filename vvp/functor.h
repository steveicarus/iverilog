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
#ident "$Id: functor.h,v 1.6 2001/03/26 04:00:39 steve Exp $"
#endif

# include  "pointers.h"
# include  <stdio.h>


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
 * The function of the functor is defined by the table/event
 * union. Normally, the truth table is the behavior and the functor
 * output value is picked from the lookup table that the table pointer
 * points to.
 *
 * If the functor is an event functor, however, the event member
 * points to an extended structure where thread state is stored.
 *
 * The major mode is selected by the mode parameter.
 */

struct functor_s {
	/* This is the truth table for the device */
      union {
	    vvp_truth_t table;
	    vvp_event_t event;
      };

	/* This is the output for the device. */
      vvp_ipoint_t out;
	/* These are the input ports. */
      vvp_ipoint_t port[4];
	/* These are the input values. */
      unsigned char ival;
      unsigned char oval;
	/* functor mode:  0 == table ;  1 == event */
      unsigned char mode;
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
extern void functor_set(vvp_ipoint_t point, unsigned val);

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
 * Dump a readable version of the functor address space to the file.
 */
extern void functor_dump(FILE*fd);


extern const unsigned char ft_AND[];
extern const unsigned char ft_NOR[];
extern const unsigned char ft_NOT[];
extern const unsigned char ft_OR[];
extern const unsigned char ft_var[];

/*
 * $Log: functor.h,v $
 * Revision 1.6  2001/03/26 04:00:39  steve
 *  Add the .event statement and the %wait instruction.
 *
 * Revision 1.5  2001/03/25 19:38:23  steve
 *  Support NOR and NOT gates.
 *
 * Revision 1.4  2001/03/22 05:08:00  steve
 *  implement %load, %inv, %jum/0 and %cmp/u
 *
 * Revision 1.3  2001/03/20 06:16:24  steve
 *  Add support for variable vectors.
 *
 * Revision 1.2  2001/03/11 22:42:11  steve
 *  Functor values and propagation.
 *
 * Revision 1.1  2001/03/11 00:29:38  steve
 *  Add the vvp engine to cvs.
 *
 */
#endif
