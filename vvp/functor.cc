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
#ident "$Id: functor.cc,v 1.21 2001/05/30 03:02:35 steve Exp $"
#endif

# include  "functor.h"
# include  "udp.h"
# include  "schedule.h"
# include  "vthread.h"
# include  "debug.h"
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

void functor_put_input(functor_t fp, unsigned pp, unsigned val, unsigned str)
{
	/* Change the bits of the input. */
      static const unsigned char ival_mask[4] = { 0xfc, 0xf3, 0xcf, 0x3f };
      unsigned char imask = ival_mask[pp];
      fp->ival = (fp->ival & imask) | ((val & 3) << (2*pp));

	/* Save the strength aware input value. */
      fp->istr[pp] = str;
}

static void functor_set_mode0(vvp_ipoint_t ptr, functor_t fp, bool push)
{
	/* Locate the new output value in the table. */
      unsigned char out = fp->table[fp->ival >> 2];
      out >>= 2 * (fp->ival&0x03);
      out &= 0x03;

	/* If the output changes, then create a propagation event. */
      if (out != fp->oval) {
	    fp->oval = out;
	    if (push)
		  functor_propagate(ptr);
	    else
		  schedule_functor(ptr, 0);
      }
}

const unsigned char vvp_edge_posedge[16] = {
      0, 1, 1, 1, // 0 -> ...
      0, 0, 0, 0, // 1 -> ...
      0, 1, 0, 0, // x -> ...
      0, 1, 0, 0  // z -> ...
};

const unsigned char vvp_edge_negedge[16] = {
      0, 0, 0, 0, // 0 -> ...
      1, 0, 1, 1, // 1 -> ...
      1, 0, 0, 0, // x -> ...
      1, 0, 0, 0  // z -> ...
};

const unsigned char vvp_edge_anyedge[16] = {
      0, 1, 1, 1, // 0 -> ...
      1, 0, 1, 1, // 1 -> ...
      1, 1, 0, 1, // x -> ...
      1, 1, 1, 0  // z -> ...
};

/*
 * A mode1 functor is a probe of some sort that is looking for an edge
 * of some specified type on any of its inputs. If it finds the right
 * kind of edge, then it awakens all the threads that are waiting on
 * this functor *and* it schedules an assign for any output it might
 * have. The latter is to support wider event/or then a single functor
 * can support.
 */
static void functor_set_mode1(functor_t fp)
{
      vvp_event_t ep = fp->event;

	/* Only go through the effort if there is someone interested
	   in the results... */

      if (ep->threads || fp->out) {

	    for (unsigned idx = 0 ;  idx < 4 ;  idx += 1) {
		  unsigned oval = (ep->ival >> 2*idx) & 3;
		  unsigned nval = (fp->ival >> 2*idx) & 3;

		  unsigned val = (oval << 2) | nval;
		  unsigned char edge_p = ep->vvp_edge_tab[val];

		  if (edge_p) {
			vthread_t tmp = ep->threads;
			ep->threads = 0;
			vthread_schedule_list(tmp);

			if (fp->out)
			      schedule_assign(fp->out, 0, 0);
		  }
	    }
      }

	/* the new value is the new old value. */
      ep->ival = fp->ival;
}

/*
 * A mode-2 functor is a named event. In this case, any set at all is
 * enough to trigger the blocked threads.
 */
static void functor_set_mode2(functor_t fp)
{
      vvp_event_t ep = fp->event;

      if (ep->threads) {
	    vthread_t tmp = ep->threads;
	    ep->threads = 0;
	    vthread_schedule_list(tmp);
      }

      if (fp->out)
	    schedule_assign(fp->out, 0, 0);
}

/*
 * Set the addressed bit of the functor, and recalculate the
 * output. If the output changes any, then generate the necessary
 * propagation events to pass the output on.
 */
void functor_set(vvp_ipoint_t ptr, unsigned bit, unsigned str, bool push)
{
      functor_t fp = functor_index(ptr);
      unsigned pp = ipoint_port(ptr);
      assert(fp);

	/* Store the value and strengths in the input bits. */
      functor_put_input(fp, pp, bit, str);

      switch (fp->mode) {
	  case 0:
	    functor_set_mode0(ptr, fp, push);
	    break;
	  case 1:
	    functor_set_mode1(fp);
	    break;
	  case 2:
	    functor_set_mode2(fp);
	    break;
          case M42:
            if (!fp->obj) {
		  ptr = fp->out;
		  fp = functor_index(ptr);
	    }
	    assert(fp->mode == M42);
	    fp->obj->set(ptr, fp, push);
	    break;
      }

#if defined(WITH_DEBUG)
      if (fp->breakpoint)
	    breakpoint();
#endif
}

unsigned functor_get(vvp_ipoint_t ptr)
{
      functor_t fp = functor_index(ptr);
      assert(fp);
      if ((fp->mode == M42) && fp->obj)
	    return fp->obj->get(ptr, fp);
      return fp->oval;
}

unsigned vvp_fobj_s::get(vvp_ipoint_t, functor_t fp)
{
      return fp->oval;
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
      unsigned drive0 = fp->odrive0;
      unsigned drive1 = fp->odrive1;

      unsigned str;
      switch (oval) {
	  case 0:
	    str = 0x00 | (drive0<<0) | (drive0<<4);
	    break;
	  case 1:
	    str = 0x88 | (drive1<<0) | (drive1<<4);
	    break;
	  case 2:
	    str = 0x80 | (drive0<<0) | (drive1<<4);
	    break;
	  case 3:
	    str = 0x00;
	    break;
      }

      vvp_ipoint_t idx = fp->out;
      while (idx) {
	    functor_t idxp = functor_index(idx);
	    vvp_ipoint_t next = idxp->port[ipoint_port(idx)];

	    functor_set(idx, oval, str, false);
	    idx = next;
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

