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
#ident "$Id: functor.cc,v 1.35 2001/12/06 03:31:24 steve Exp $"
#endif

# include  "functor.h"
# include  "debug.h"
# include  <assert.h>
# include  <string.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

/*
 * Functors are created as the source design is read in. Each is
 * assigned an ipoint_t address starting from 1. The design is
 * expected to have a create many functors, so it makes sense to
 * allocate the functors in chunks. This structure describes a chunk
 * of functors.
 *
 * The 32bit vvp_ipoint_t allows for 2**30 functors in the
 * design. (2 bits are used to select the input of the functor.) The
 * functor address is, for the purpose of lookup up addresses, divided
 * into three parts, the index within a chunk, the index of the chunk
 * within an index1 table, and the index of the index1 within the root
 * table. There is a single root table. The index1 tables and chunk
 * tables are allocated as needed.
 */

functor_t **functor_list = 0x0;
static unsigned functor_count = 0; 
static unsigned functors_allocated = 0; 

/*
 * This function initializes the functor address space by creating the
 * zero functor. This means creating a functor_index1 and a
 * functor_index0, and initializing the count to 1.
 */
void functor_init(void)
{
	// allocate the ZERO functor.
      functor_allocate(1);
}

/*
 * Allocate normally is just a matter of incrementing the functor_count
 * and returning a pointer to the next unallocated functor. However,
 * if we overrun an allocated chunk, we need to allocate the needed
 * bits first.
 */
vvp_ipoint_t functor_allocate(unsigned wid)
{
      vvp_ipoint_t idx = functor_count*4;
      functor_count += wid;

      if (functor_count > functors_allocated*functor_chunks) {

	      // enlarge the list of chunks
	    unsigned fa = (functor_count + functor_chunks - 1)/functor_chunks;
	    functor_list = (functor_t **)
		  realloc(functor_list, fa*sizeof(functor_t*));
	    assert(functor_list);

	      // allocate the chunks of functor pointers.
	    while (fa > functors_allocated) {

		  functor_list[functors_allocated] = (functor_t *)
			malloc(functor_chunks * sizeof(functor_t));
		  assert(functor_list[functors_allocated]);

		  memset(functor_list[functors_allocated],
			 0,
			 functor_chunks * sizeof(functor_t));

		  functors_allocated += 1;
	    }
      }

      return idx;
}

void functor_define(vvp_ipoint_t point, functor_t obj)
{
      unsigned index1 = point/4/functor_chunks;
      unsigned index2 = (point/4) % functor_chunks;
      functor_list[index1][index2] = obj;
}


functor_s::functor_s()
{
      delay = 0;
      out = 0;
      port[0] = 0;
      port[1] = 0;
      port[2] = 0;
      port[3] = 0;
      ival = 0xaa;
      cval = 2;
      oval = 2;
      odrive0 = 6;
      odrive1 = 6;
      ostr = StX;
      cstr = StX;
      inhibit = 0;
#if defined(WITH_DEBUG)
      breakpoint = 0;
#endif
}

functor_s::~functor_s()
{
}

//          Special infrastructure functor types

extra_outputs_functor_s::~extra_outputs_functor_s()
{}

void extra_outputs_functor_s::set(vvp_ipoint_t i, bool push, 
				  unsigned val, unsigned)
{
      put(i, val);
      functor_t base = functor_index(base_);
      val = base->ival & 3; // yes, this is ugly
      base->set(base_, push, val);
}

extra_ports_functor_s::~extra_ports_functor_s()
{}

void extra_ports_functor_s::set(vvp_ipoint_t i, bool push, 
			       unsigned val, unsigned str)
{
      functor_t base = functor_index(base_);
      base->set(i, push, val, str);
}

extra_inputs_functor_s::~extra_inputs_functor_s()
{}

void extra_inputs_functor_s::set(vvp_ipoint_t i, bool push, 
				 unsigned val, unsigned)
{
      put(i, val);
      functor_t base = functor_index(out);
      val = base->ival & 3; // yes, this is ugly
      base->set(ipoint_make(out,0), push, val);
}

edge_inputs_functor_s::~edge_inputs_functor_s()
{}

/*
 * $Log: functor.cc,v $
 * Revision 1.35  2001/12/06 03:31:24  steve
 *  Support functor delays for gates and UDP devices.
 *  (Stephan Boettcher)
 *
 * Revision 1.34  2001/11/16 04:22:27  steve
 *  include stdlib.h for portability.
 *
 * Revision 1.33  2001/11/10 18:07:11  steve
 *  Runtime support for functor delays. (Stephan Boettcher)
 *
 * Revision 1.32  2001/11/06 03:07:22  steve
 *  Code rearrange. (Stephan Boettcher)
 *
 * Revision 1.31  2001/11/04 05:03:21  steve
 *  MacOSX 10.1 updates.
 *
 * Revision 1.30  2001/11/01 03:00:19  steve
 *  Add force/cassign/release/deassign support. (Stephan Boettcher)
 *
 * Revision 1.29  2001/10/31 04:27:46  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.28  2001/10/27 03:43:56  steve
 *  Propagate functor push, to make assign better.
 *
 * Revision 1.27  2001/10/12 03:00:09  steve
 *  M42 implementation of mode 2 (Stephan Boettcher)
 *
 * Revision 1.26  2001/08/08 01:05:06  steve
 *  Initial implementation of vvp_fvectors.
 *  (Stephan Boettcher)
 *
 * Revision 1.25  2001/07/30 03:53:01  steve
 *  Initialize initial functor tables.
 *
 * Revision 1.24  2001/07/16 18:06:01  steve
 *  Initialize allocated functors (Stephan Boettcher)
 *
 * Revision 1.23  2001/06/21 22:54:12  steve
 *  Support cbValueChange callbacks.
 *
 * Revision 1.22  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 * Revision 1.21  2001/05/30 03:02:35  steve
 *  Propagate strength-values instead of drive strengths.
 *
 * Revision 1.20  2001/05/12 20:38:06  steve
 *  A resolver that understands some simple strengths.
 *
 * Revision 1.19  2001/05/09 04:23:18  steve
 *  Now that the interactive debugger exists,
 *  there is no use for the output dump.
 *
 * Revision 1.18  2001/05/09 02:53:25  steve
 *  Implement the .resolv syntax.
 *
 * Revision 1.17  2001/05/08 23:32:26  steve
 *  Add to the debugger the ability to view and
 *  break on functors.
 *
 *  Add strengths to functors at compile time,
 *  and Make functors pass their strengths as they
 *  propagate their output.
 *
 * Revision 1.16  2001/05/06 03:51:37  steve
 *  Regularize the mode-42 functor handling.
 *
 * Revision 1.15  2001/05/03 04:54:33  steve
 *  Fix handling of a mode 1 functor that feeds into a
 *  mode 2 functor. Feed the result only if the event
 *  is triggered, and do pass to the output even if no
 *  threads are waiting.
 *
 * Revision 1.14  2001/04/26 15:52:22  steve
 *  Add the mode-42 functor concept to UDPs.
 *
 * Revision 1.13  2001/04/24 02:23:59  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 * Revision 1.12  2001/04/18 04:21:23  steve
 *  Put threads into scopes.
 *
 * Revision 1.11  2001/04/14 05:10:56  steve
 *  support the .event/or statement.
 *
 * Revision 1.10  2001/04/03 03:18:34  steve
 *  support functor_set push for blocking assignment.
 *
 * Revision 1.9  2001/03/31 19:29:23  steve
 *  Fix compilation warnings.
 */

