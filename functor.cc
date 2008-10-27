/*
 * Copyright (c) 1999-2008 Stephen Williams (steve@icarus.com)
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

void functor_t::lpm_abs(class Design*, class NetAbs*)
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

void functor_t::lpm_literal(class Design*, class NetLiteral*)
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

void functor_t::lpm_pow(class Design*, class NetPow*)
{
}

void functor_t::sign_extend(class Design*, class NetSignExtend*)
{
}

void functor_t::lpm_ureduce(class Design*, class NetUReduce*)
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

      signals_map_iter_t cur = signals_map_.begin();
      while (cur != signals_map_.end()) {
	    signals_map_iter_t tmp = cur;
	    cur ++;
	    fun->signal(des, tmp->second);
      }
}

void Design::functor(functor_t*fun)
{
	// Scan the scopes
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin();
	   scope != root_scopes_.end(); scope++)
	    (*scope)->run_functor(this, fun);

	// apply to processes
      procs_idx_ = procs_;
      while (procs_idx_) {
	    NetProcTop*idx = procs_idx_;
	    procs_idx_ = idx->next_;
	    fun->process(this, idx);
      }

	// apply to nodes
      if (nodes_) {
	    assert(nodes_functor_cur_ == 0);
	    assert(nodes_functor_nxt_ == 0);

	      /* Scan the circular list of nodes, starting with the
		 front of the list.

		 This loop interacts with the Design::del_node method
		 so that the functor is free to delete any nodes it
		 choose. The destructors of the NetNode objects call
		 the del_node method, which checks with the
		 nodes_functor_* members, to keep the iterator
		 operating safely. */
	    nodes_functor_cur_ = nodes_;
	    do {
		  nodes_functor_nxt_ = nodes_functor_cur_->node_next_;
		  nodes_functor_cur_->functor_node(this, fun);

		  if (nodes_functor_nxt_ == 0)
			break;

		  nodes_functor_cur_ = nodes_functor_nxt_;
	    } while (nodes_ && (nodes_functor_cur_ != nodes_));
	    nodes_functor_cur_ = 0;
	    nodes_functor_nxt_ = 0;

      }
}


void NetNode::functor_node(Design*, functor_t*)
{
}

void NetAbs::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_abs(des, this);
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

void NetLiteral::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_literal(des, this);
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

void NetPow::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_pow(des, this);
}

void NetSignExtend::functor_node(Design*des, functor_t*fun)
{
      fun->sign_extend(des, this);
}

void NetUReduce::functor_node(Design*des, functor_t*fun)
{
      fun->lpm_ureduce(des, this);
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

int proc_match_t::block(NetBlock*)
{
      return 0;
}

int NetBlock::match_proc(proc_match_t*that)
{
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
