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
#if !defined(WINNT)
#ident "$Id: force.cc,v 1.2 2001/11/01 04:42:40 steve Exp $"
#endif

# include  "codes.h"
# include  "force.h"

# include  <assert.h>

inline bool functor_s::disable(vvp_ipoint_t ptr)
{
      bool r = inhibit;
      inhibit = 1;
      return r;
}

inline bool functor_s::enable(vvp_ipoint_t ptr)
{
      unsigned val;
      if (ostr == 0)
	    val = 3;
      else switch (ostr & 0x88) {
	  case 0x00: val = 0; break;
	  case 0x88: val = 1; break;
	  default:   val = 2;
      }
      if (val != oval) {
	    oval = val;
	    propagate(true);
      }
      bool r = inhibit;
      inhibit = 0;
      return r;
}

inline void functor_s::force(unsigned val, unsigned str)
{
      if (ostr != str  ||  oval != val) {
	    unsigned save = ostr;
	    oval = val;
	    ostr = str;
	    propagate(true);
	    ostr = save;
      }
}

void force_functor_s::set(vvp_ipoint_t i, bool, unsigned val, unsigned str)
{
      put(i, val);
      if (ipoint_port(i) == 0) {
	    if (active  && out) {
		  functor_t tgt = functor_index(out);
		  tgt->force(ival&3, get_ostr());
	    }
      }
}

static bool release_force(vvp_ipoint_t itgt, functor_t tgt)
{
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

	    vvp_ipoint_t itgt = fofu->out;
	    functor_t tgt = functor_index(itgt);

	    if (tgt->disable(itgt))
		  release_force(itgt, tgt);

	    fofu->port[3] = tgt->out;
	    tgt->out = ipoint_make(ifofu, 3);

	    fofu->set(ifofu, false, fofu->ival&3, fofu->get_ostr());
      }

      return true;
}

bool of_RELEASE(vthread_t thr, vvp_code_t cp)
{
      vvp_ipoint_t itgt = cp->iptr;
      functor_t tgt = functor_index(itgt);

      if (release_force(itgt, tgt))
	    tgt->enable(itgt);
      // bug:  a strength change will not be propagated.

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
	    put_oval(ptr, push, val);
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

/*
 * $Log: force.cc,v $
 * Revision 1.2  2001/11/01 04:42:40  steve
 *  Handle procedural constant functor pointers.
 *
 * Revision 1.1  2001/11/01 03:00:19  steve
 *  Add force/cassign/release/deassign support. (Stephan Boettcher)
 *
 */
