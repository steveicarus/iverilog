/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: functor.cc,v 1.25 2001/07/25 03:10:49 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

# include  "functor.h"
# include  "netlist.h"

functor_t::~functor_t()
{
}

void functor_t::event(class Design*, class NetEvent*)
{
}

void functor_t::signal(class Design*, class NetNet*)
{
}

void functor_t::process(class Design*, class NetProcTop*)
{
}

void functor_t::lpm_add_sub(class Design*, class NetAddSub*)
{
}

void functor_t::lpm_compare(class Design*, class NetCompare*)
{
}

void functor_t::lpm_const(class Design*, class NetConst*)
{
}

void functor_t::lpm_divide(class Design*, class NetDivide*)
{
}

void functor_t::lpm_modulo(class Design*, class NetModulo*)
{
}

void functor_t::lpm_ff(class Design*, class NetFF*)
{
}

void functor_t::lpm_logic(class Design*, class NetLogic*)
{
}

void functor_t::lpm_mult(class Design*, class NetMult*)
{
}

void functor_t::lpm_mux(class Design*, class NetMux*)
{
}


void NetScope::run_functor(Design*des, functor_t*fun)
{
      for (NetScope*cur = sub_ ;  cur ;  cur = cur->sib_) {
	    cur->run_functor(des, fun);
      }

      for (NetEvent*cur = events_ ;  cur ;  /* */) {
	    NetEvent*tmp = cur;
	    cur = cur->snext_;
	    fun->event(des, tmp);
      }

	// apply to signals. Each iteration, allow for the possibility
	// that the current signal deletes itself.
      if (signals_) {
	    unsigned count = 0;
	    NetNet*cur = signals_->sig_next_;
	    do {
		  count += 1;
		  cur = cur->sig_next_;
	    } while (cur != signals_->sig_next_);

	    cur = signals_->sig_next_;
	    for (unsigned idx = 0 ;  idx < count ;  idx += 1) {
		  NetNet*tmp = cur->sig_next_;
		  fun->signal(des, cur);
		  cur = tmp;
	    }
      }
}

void Design::functor(functor_t*fun)
{
	// Scan the scopes
      root_scope_->run_functor(this, fun);

	// apply to processes
      procs_idx_ = procs_;
      while (procs_idx_) {
	    NetProcTop*idx = procs_idx_;
	    procs_idx_ = idx->next_;
	    fun->process(this, idx);
      }

	// apply to nodes
      if (nodes_) {
	      /* Scan the circular list of nodes, starting with the
		 front of the list. (nodes_ points to the *end* of the
		 list.) The bar is the end point. At the end of the
		 do-while loop, I know that the bar has been
		 processed or (if bar == 0) no undeleted node has been
		 processed. */
	    NetNode*cur = nodes_->node_next_;
	    NetNode*bar = 0;
	    do {
		  NetNode*tmp = cur->node_next_;
		  cur->functor_node(this, fun);

		    /* Detect the case that cur has been deleted by
		       noticing if tmp->node_prev_ no longer points to
		       cur. If that's the case, clear the bar. */
		  if (tmp->node_prev_ != cur) {
			if (cur == bar)
			      bar = 0;
		  } else if (bar == 0) {
			bar = cur;
		  }
		  cur = tmp;

	    } while (nodes_ && (cur != bar));
      }
}


void NetNode::functor_node(Design*, functor_t*)
{
}

void NetAddSub::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_add_sub(des, this);
}

void NetCompare::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_compare(des, this);
}

void NetConst::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_const(des, this);
}

void NetDivide::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_divide(des, this);
}

void NetFF::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_ff(des, this);
}

void NetLogic::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_logic(des, this);
}

void NetModulo::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_modulo(des, this);
}

void NetMult::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_mult(des, this);
}

void NetMux::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_mux(des, this);
}

proc_match_t::~proc_match_t()
{
}

int NetProc::match_proc(proc_match_t*that)
{
      return 0;
}

int proc_match_t::assign(NetAssign*)
{
      return 0;
}

int NetAssign::match_proc(proc_match_t*that)
{
      return that->assign(this);
}

int proc_match_t::assign_nb(NetAssignNB*)
{
      return 0;
}

int NetAssignNB::match_proc(proc_match_t*that)
{
      return that->assign_nb(this);
}

int proc_match_t::assign_mem(NetAssignMem*)
{
      return 0;
}

int NetAssignMem::match_proc(proc_match_t*that)
{
      return that->assign_mem(this);
}

int proc_match_t::assign_mem_nb(NetAssignMemNB*)
{
      return 0;
}

int NetAssignMemNB::match_proc(proc_match_t*that)
{
      return that->assign_mem_nb(this);
}

int proc_match_t::block(NetBlock*)
{
      cerr << "default (failing) match for block" << endl;
      return 0;
}

int NetBlock::match_proc(proc_match_t*that)
{
      cerr << "NetBlock::match_proc" << endl;
      return that->block(this);
}

int proc_match_t::condit(NetCondit*)
{
      return 0;
}

int NetCondit::match_proc(proc_match_t*that)
{
      return that->condit(this);
}

int NetEvWait::match_proc(proc_match_t*that)
{
      return that->event_wait(this);
}

int proc_match_t::event_wait(NetEvWait*)
{
      return 0;
}

/*
 * $Log: functor.cc,v $
 * Revision 1.25  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.24  2000/11/19 20:48:30  steve
 *  Fix cases where signal iteration might die early.
 *
 * Revision 1.23  2000/11/18 04:53:04  steve
 *  Watch out in functor, it may delete the last signal.
 *
 * Revision 1.22  2000/09/17 21:26:15  steve
 *  Add support for modulus (Eric Aardoom)
 *
 * Revision 1.21  2000/08/01 02:48:41  steve
 *  Support <= in synthesis of DFF and ram devices.
 *
 * Revision 1.20  2000/07/16 04:56:07  steve
 *  Handle some edge cases during node scans.
 *
 * Revision 1.19  2000/07/15 05:13:43  steve
 *  Detect muxing Vz as a bufufN.
 *
 * Revision 1.18  2000/05/02 00:58:12  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.17  2000/04/20 00:28:03  steve
 *  Catch some simple identity compareoptimizations.
 *
 * Revision 1.16  2000/04/18 04:50:19  steve
 *  Clean up unneeded NetEvent objects.
 *
 * Revision 1.15  2000/04/16 23:32:18  steve
 *  Synthesis of comparator in expressions.
 *
 *  Connect the NetEvent and related classes
 *  together better.
 *
 * Revision 1.14  2000/04/12 20:02:53  steve
 *  Finally remove the NetNEvent and NetPEvent classes,
 *  Get synthesis working with the NetEvWait class,
 *  and get started supporting multiple events in a
 *  wait in vvm.
 *
 * Revision 1.13  2000/04/01 21:40:22  steve
 *  Add support for integer division.
 *
 * Revision 1.12  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.11  2000/02/13 04:35:43  steve
 *  Include some block matching from Larry.
 *
 * Revision 1.10  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.9  2000/01/02 17:57:20  steve
 *  Handle nodes running out during node scan.
 *
 * Revision 1.8  1999/12/30 04:19:12  steve
 *  Propogate constant 0 in low bits of adders.
 *
 * Revision 1.7  1999/12/17 06:18:16  steve
 *  Rewrite the cprop functor to use the functor_t interface.
 *
 * Revision 1.6  1999/12/05 02:24:08  steve
 *  Synthesize LPM_RAM_DQ for writes into memories.
 *
 * Revision 1.5  1999/12/01 06:06:16  steve
 *  Redo synth to use match_proc_t scanner.
 *
 * Revision 1.4  1999/11/18 03:52:19  steve
 *  Turn NetTmp objects into normal local NetNet objects,
 *  and add the nodangle functor to clean up the local
 *  symbols generated by elaboration and other steps.
 *
 * Revision 1.3  1999/11/01 02:07:40  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.2  1999/07/18 05:52:46  steve
 *  xnfsyn generates DFF objects for XNF output, and
 *  properly rewrites the Design netlist in the process.
 *
 * Revision 1.1  1999/07/17 22:01:13  steve
 *  Add the functor interface for functor transforms.
 *
 */

