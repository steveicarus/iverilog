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
#ident "$Id: functor.cc,v 1.39 2002/07/05 02:50:58 steve Exp $"
#endif

# include  "functor.h"
# include  "statistics.h"
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
static unsigned functor_chunk_count = 0;

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

unsigned functor_limit()
{
      return functor_count;
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
      count_functors += wid;

      if (functor_count > functor_chunk_count*functor_chunk_size) {

	      // enlarge the list of chunks
	    unsigned fa = (functor_count + functor_chunk_size - 1)
		  / functor_chunk_size;

	    functor_list = (functor_t **)
		  realloc(functor_list, fa*sizeof(functor_t*));
	    assert(functor_list);

	      // allocate the chunks of functor pointers.
	    while (fa > functor_chunk_count) {

		  functor_list[functor_chunk_count] = (functor_t *)
			malloc(functor_chunk_size * sizeof(functor_t));
		  assert(functor_list[functor_chunk_count]);

		  memset(functor_list[functor_chunk_count], 0,
			 functor_chunk_size * sizeof(functor_t));

		  functor_chunk_count += 1;
	    }
      }

      return idx;
}

void functor_define(vvp_ipoint_t point, functor_t obj)
{
      unsigned index1 = point/4/functor_chunk_size;
      unsigned index2 = (point/4) % functor_chunk_size;
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
      break_flag = 0;
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

#ifdef WITH_DEBUG
# include  <stdio.h>
static const char bitval_tab[4] = { '0', '1', 'x', 'z' };

void functor_s::debug_print(vvp_ipoint_t fnc)
{
      printf("0x%x: out pointer  =", fnc);
      vvp_ipoint_t cur = out;
      while (cur) {
	    printf(" 0x%x", cur);
	    functor_t tmp = functor_index(cur);
	    cur = tmp->port[cur&3];
      }
      printf("\n");
      printf("0x%x: input values = %c %c %c %c\n", fnc,
	     bitval_tab[ival&3],
	     bitval_tab[(ival>>2)&3],
	     bitval_tab[(ival>>4)&3],
	     bitval_tab[(ival>>6)&3]);
      printf("0x%x: out value    = %c (%02x)\n", fnc,
	     bitval_tab[get_oval()], get_ostr());
}
#endif

/*
 * $Log: functor.cc,v $
 * Revision 1.39  2002/07/05 02:50:58  steve
 *  Remove the vpi object symbol table after compile.
 *
 * Revision 1.38  2002/01/06 17:50:50  steve
 *  Support scope for functors. (Stephan Boettcher)
 *
 * Revision 1.37  2001/12/18 05:32:11  steve
 *  Improved functor debug dumps.
 *
 * Revision 1.36  2001/12/14 01:59:28  steve
 *  Better variable names for functor chunks.
 *
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
 */

