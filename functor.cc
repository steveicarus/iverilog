/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: functor.cc,v 1.2 1999/07/18 05:52:46 steve Exp $"
#endif

# include  "functor.h"
# include  "netlist.h"

functor_t::~functor_t()
{
}

void functor_t::signal(class Design*, class NetNet*)
{
}

void functor_t::process(class Design*, class NetProcTop*)
{
}

void Design::functor(functor_t*fun)
{
	// apply to signals
      if (signals_) {
	    NetNet*cur = signals_->sig_next_;
	    do {
		  fun->signal(this, cur);
		  cur = cur->sig_next_;
	    } while (cur != signals_->sig_next_);
      }

	// apply to processes
      procs_idx_ = procs_;
      while (procs_idx_) {
	    NetProcTop*idx = procs_idx_;
	    procs_idx_ = idx->next_;
	    fun->process(this, idx);
      }
}

/*
 * $Log: functor.cc,v $
 * Revision 1.2  1999/07/18 05:52:46  steve
 *  xnfsyn generates DFF objects for XNF output, and
 *  properly rewrites the Design netlist in the process.
 *
 * Revision 1.1  1999/07/17 22:01:13  steve
 *  Add the functor interface for functor transforms.
 *
 */

