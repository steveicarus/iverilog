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
#ident "$Id: functor.cc,v 1.1 2001/03/11 00:29:38 steve Exp $"
#endif

# include  "functor.h"
# include  <assert.h>

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

const unsigned functor_index0_size = 2 <<  9;
const unsigned functor_index1_size = 2 << 11;
const unsigned functor_index2_size = 2 << 10;

struct functor_index0 {
      struct functor_s table[functor_index0_size];
};

struct functor_index1 {
      struct functor_index0* table[functor_index1_size];
};

static vvp_ipoint_t functor_count = 0;
static struct functor_index1*functor_table[functor_index2_size] = { 0 };


/*
 * This function initializes the functor address space by creating the
 * zero functor. This means creating a functor_index1 and a
 * functor_index0, and initializing the count to 1.
 */
void functor_init(void)
{
      functor_table[0] = new struct functor_index1;
      functor_table[0]->table[0] = new struct functor_index0;
      functor_count = 1;
}

/*
 * Allocate normall is just a matter of incrementing the functor_count
 * and returning a pointer to the next unallocated functor. However,
 * if we overrun a chunk or an index, we need to allocate the needed
 * bits first.
 */
vvp_ipoint_t functor_allocate(void)
{
      vvp_ipoint_t idx = functor_count;

      idx /= functor_index0_size;

      unsigned index1 = idx % functor_index1_size;
      idx /= functor_index1_size;

      assert( idx < functor_index2_size);

      if (functor_table[idx] == 0)
	    functor_table[idx] = new struct functor_index1;

      if (functor_table[idx]->table[index1] == 0)
	    functor_table[idx]->table[index1] = new struct functor_index0;

      vvp_ipoint_t res = functor_count;
      functor_count += 1;
      return res * 4;
}


functor_t functor_index(vvp_ipoint_t point)
{
      point /= 4;

      assert(point < functor_count);
      assert(point > 0);

      unsigned index0 = point % functor_index0_size;
      point /= functor_index0_size;

      unsigned index1 = point % functor_index1_size;
      point /= functor_index1_size;

      return functor_table[point]->table[index1]->table + index0;
}


void functor_dump(FILE*fd)
{
      for (unsigned idx = 1 ;  idx < functor_count ;  idx += 1) {
	    functor_t cur = functor_index(idx*4);
	    fprintf(fd, "%10p: out=%x port={%x %x %x %x}\n", idx*4,
		    cur->out, cur->port[0], cur->port[1],
		    cur->port[2], cur->port[3]);
      }
}

/*
 * $Log: functor.cc,v $
 * Revision 1.1  2001/03/11 00:29:38  steve
 *  Add the vvp engine to cvs.
 *
 */

