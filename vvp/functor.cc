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
#ident "$Id: functor.cc,v 1.6 2001/03/25 00:35:35 steve Exp $"
#endif

# include  "functor.h"
# include  "schedule.h"
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
 * Allocate normally is just a matter of incrementing the functor_count
 * and returning a pointer to the next unallocated functor. However,
 * if we overrun a chunk or an index, we need to allocate the needed
 * bits first.
 */
static vvp_ipoint_t functor_allocate_(void)
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

vvp_ipoint_t functor_allocate(unsigned wid)
{
      assert(wid > 0);
      vvp_ipoint_t res = functor_allocate_();

      wid -= 1;
      while (wid > 0) {
	    functor_allocate_();
	    wid -= 1;
      }

      return res;
}

/*
 * Given a vvp_ipoint_t pointer, return a pointer to the detailed
 * functor structure. This does not use the low 2 bits, which address
 * a specific port of the functor.
 */
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

/*
 * Set the addressed bit of the functor, and recalculate the
 * output. If the output changes any, then generate the necessary
 * propagation events to pass the output on.
 */
void functor_set(vvp_ipoint_t ptr, unsigned bit)
{
      functor_t fp = functor_index(ptr);
      unsigned pp = ipoint_port(ptr);
      assert(fp);
      assert(fp->table);

	/* Change the bits of the input. */
      static const unsigned char mask_table[4] = { 0xfc, 0xf3, 0xcf, 0x3f };
      unsigned char mask = mask_table[pp];
      fp->ival = (fp->ival & mask) | (bit << (2*pp));

	/* Locate the new output value in the table. */
      unsigned char out = fp->table[fp->ival >> 2];
      out >>= 2 * (fp->ival&0x03);
      out &= 0x03;

	/* If the output changes, then create a propagation event. */
      if (out != fp->oval) {
	    fp->oval = out;
	    schedule_functor(ptr, 0);
      }
}

unsigned functor_get(vvp_ipoint_t ptr)
{
      functor_t fp = functor_index(ptr);
      assert(fp);
      return fp->oval & 3;
}

/*
 * This function is used by the scheduler to implement the propagation
 * event. The input is the pointer to the functor who's output is to
 * be propagated. Pass the output to the inputs of all the connected
 * functors.
 */
void functor_propagate(vvp_ipoint_t ptr)
{
      functor_t fp = functor_index(ptr);
      unsigned char oval = fp->oval;

	//printf("functor %lx becomes %u\n", ptr, oval);

      vvp_ipoint_t idx = fp->out;
      while (idx) {
	    functor_t idxp = functor_index(idx);
	    vvp_ipoint_t next = idxp->port[ipoint_port(idx)];
	      //printf("    set %lx to %u\n", idx, oval);
	    functor_set(idx, oval);
	    idx = next;
      }
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
 * The variable functor is special. This is the truth table for it.
 */
const unsigned char ft_var[16] = {
      0xe4, /* 0 0: 11, 10, 01, 00 */
      0xe4, /* 0 1: 11, 10, 01, 00 */
      0xe4, /* 0 x: 11, 10, 01, 00 */
      0xe4, /* 0 z: 11, 10, 01, 00 */
      0x00, /* 1 0: 00, 00, 00, 00 */
      0x55, /* 1 1: 01, 01, 01, 01 */
      0xaa, /* 1 x: 10, 10, 10, 10 */
      0xff, /* 1 z: 11, 11, 11, 11 */
      0xe4,
      0xe4,
      0xe4,
      0xe4,
      0xe4,
      0xe4,
      0xe4,
      0xe4
};



/*
 * $Log: functor.cc,v $
 * Revision 1.6  2001/03/25 00:35:35  steve
 *  Add the .net statement.
 *
 * Revision 1.5  2001/03/22 05:28:16  steve
 *  no longer need out message.
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

