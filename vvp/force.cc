/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: force.cc,v 1.10 2004/12/15 17:17:42 steve Exp $"
#endif

# include  "codes.h"
# include  "force.h"

# include  <stdio.h>
# include  <assert.h>

/*
 * This functor method turns off the output of the functor by setting
 * an inhibit flag. While this flag is set, the functor is not
 * supposed to set its output or propagate values.
 */
bool functor_s::disable(vvp_ipoint_t ptr)
{
      bool r = inhibit;
      inhibit = 1;
      return r;
}

bool functor_s::enable(vvp_ipoint_t ptr)
{
      bool r = inhibit;
      inhibit = 0;
      if (r) {
	    if (get_str() != get_ostr()) {
		  propagate(true);
	    } else {
		  assert(get() == get_oval());
	    }
      }
      return r;
}

void force_functor_s::set(vvp_ipoint_t i, bool push,
			  unsigned val, unsigned str)
{
      if (ipoint_port(i) == 0) {
	    oval = val;
	    ostr = str;
	    if (active && out) {
		  functor_t tgt = functor_index(out);
		  if (str != tgt->get_str())
			tgt->propagate(val, str, push);
		  else
			assert(val == tgt->get());
	    }
      }
}

static bool release_force(vvp_ipoint_t itgt, functor_t tgt)
{
	// Given the ipointer to the target functor, look for a force
	// functor that is saving this output value in its input
	// 3. That is the functor that is forcing me.
      vvp_ipoint_t *ref = &tgt->out;
      while (*ref) {
	    functor_t fofu =  functor_index(*ref);
	    unsigned pp = ipoint_port(*ref);

	    if (pp == 3  &&  fofu->out == itgt) {
		  force_functor_s *ff = dynamic_cast<force_functor_s *>(fofu);

		  if (ff && ff->active) {
			ff->active = 0;
			*ref = fofu->port[pp];
			fofu->port[pp] = 0;
			return true;
		  }
	    }

	    ref = &fofu->port[pp];
      }

      return false;
}


/*
 * The %release instruction causes any force functors driving the
 * target functor to be released.
 */
bool of_RELEASE(vthread_t thr, vvp_code_t cp)
{
#if 0
      vvp_ipoint_t itgt = cp->iptr;
      functor_t tgt = functor_index(itgt);

      if (release_force(itgt, tgt))
	    tgt->enable(itgt);
#else
      fprintf(stderr, "XXXX forgot how to implement %%release\n");
#endif
      return true;
}


/*
**  Variable functors receive %set or %assign-ed values at port[0].
**  Continuous assignments are connected to port[1].
**
**  port[3] points back to the functor that drives the continuous
**  assignment.  This also serves as a flag if assignment is active.
**
**  A constant is assigned if the expr ipoint is < 4, i.e. a port of
**  the NULL functor.  port[3] is set to 3.
*/

void var_functor_s::set(vvp_ipoint_t ptr, bool push, unsigned val, unsigned)
{
      unsigned pp = ipoint_port(ptr);

      if (assigned() && pp==1  ||  !assigned() && pp==0) {
	    put_oval(val, push);
      }
}

bool var_functor_s::deassign(vvp_ipoint_t itgt)
{
      if (!assigned())
	    return false;

      vvp_ipoint_t ipt = port[3];

      if (ipt == ipoint_make(0, 3)) {
	    port[3] = 0;
	    return true;
      }

      functor_t fp = functor_index(ipt);
      vvp_ipoint_t *ref = &fp->out;

      itgt = ipoint_make(itgt, 1);

      while (*ref) {
	    if (*ref == itgt) {
		  *ref = port[1];
		  port[1] = 0;
		  return true;
	    }

	    functor_t fu =  functor_index(*ref);
	    unsigned pp = ipoint_port(*ref);
	    ref = &fu->port[pp];
      }

      return false;
}


/*
 * $Log: force.cc,v $
 * Revision 1.10  2004/12/15 17:17:42  steve
 *  Add the force/v instruction.
 *
 * Revision 1.9  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.8  2004/10/04 01:10:59  steve
 *  Clean up spurious trailing white space.
 */
