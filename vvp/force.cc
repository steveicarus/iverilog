/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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
 * The %force instruction causes the referenced force functor[s] to
 * interpose themselves in front of the precompiled target
 * functors. The operand of the %force is the label of the force
 * functor, and the width of the functor array.
 */
bool of_FORCE(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = cp->bit_idx[0];

      for (unsigned i=0; i<wid; i++) {
	    vvp_ipoint_t ifofu = ipoint_index(cp->iptr, i);
	    functor_t fofu = functor_index(ifofu);
	    force_functor_s *ff = dynamic_cast<force_functor_s *>(fofu);

	    assert(ff);

	    if (ff->active)
		  return true;

	    ff->active = 1;

	      // tgt is the functor who's output I plan to force. The
	      // compiler stored the target pointer is the out pointer
	      // of the force functor. (The out pointer is not
	      // otherwise used.)
	    vvp_ipoint_t itgt = fofu->out;
	    functor_t tgt = functor_index(itgt);

	      // This turns OFF the target functor's output
	      // driver. If it is already off, then detach the
	      // previous force before continuing.
	    if (tgt->disable(itgt))
		  release_force(itgt, tgt);

	      // Link the functor as the first functor driven by the
	      // target. This allows the functor to track the unforced
	      // drive value, and also aids in releasing the force.
	    fofu->port[3] = tgt->out;
	    tgt->out = ipoint_make(ifofu, 3);

	      // Set the value to cause the overridden functor to take
	      // on its forced value.
	    fofu->set(ifofu, false, fofu->get_oval(), fofu->get_ostr());
      }

      return true;
}

/*
 * The %release instruction causes any force functors driving the
 * target functor to be released.
 */
bool of_RELEASE(vthread_t thr, vvp_code_t cp)
{
      vvp_ipoint_t itgt = cp->iptr;
      functor_t tgt = functor_index(itgt);

      if (release_force(itgt, tgt))
	    tgt->enable(itgt);

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

      if ((assigned() && pp==1) || (!assigned() && pp==0)) {
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
 * The %cassign instruction causes a variable functor (the target) the
 * receive a behavioral continuous assignment from the functor on the
 * right (the source expression). If the source functor address is 0,
 * then the port part is a constant value to write into the target.
 */
bool of_CASSIGN(vthread_t thr, vvp_code_t cp)
{
      vvp_ipoint_t itgt = cp->iptr;
      vvp_ipoint_t iexp = cp->iptr2;

      functor_t tgt = functor_index(itgt);

      var_functor_s *var = dynamic_cast<var_functor_s *>(tgt);
      assert(var);
      var->deassign(itgt);

      // a constant expression?
      if (ipoint_make(iexp, 0) == 0) {
	    tgt->port[3] = ipoint_make(0, 3);
	    tgt->set(ipoint_make(itgt, 1), true, ipoint_port(iexp));
	    return true;
      }

      functor_t fu = functor_index(iexp);

      tgt->port[3] = iexp;
      tgt->port[1] = fu->out;
      fu->out = ipoint_make(itgt, 1);

      tgt->set(ipoint_make(itgt, 1), true, fu->get());

      return true;
}

bool of_DEASSIGN(vthread_t thr, vvp_code_t cp)
{
      vvp_ipoint_t itgt = cp->iptr;
      unsigned wid = cp->bit_idx[0];

      for (unsigned i = 0; i<wid; i++) {
	    vvp_ipoint_t ipt = ipoint_index(itgt, i);
	    functor_t tgt = functor_index(ipt);

	    var_functor_s *var = dynamic_cast<var_functor_s *>(tgt);
	    assert(var);

	    var->deassign(ipt);
      }

      return true;
}
