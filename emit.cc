/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: emit.cc,v 1.9 1999/05/01 02:57:53 steve Exp $"
#endif

/*
 * The emit function is called to generate the output required of the
 * target.
 */
# include  "target.h"
# include  "netlist.h"
# include  <iostream>
# include  <typeinfo>
# include  <cassert>

void NetNode::emit_node(ostream&o, struct target_t*tgt) const
{
      cerr << "EMIT: Gate type? " << typeid(*this).name() << endl;
}

void NetLogic::emit_node(ostream&o, struct target_t*tgt) const
{
      tgt->logic(o, this);
}

void NetUDP::emit_node(ostream&o, struct target_t*tgt) const
{
      tgt->udp(o, this);
}

void NetAssign::emit_node(ostream&o, struct target_t*tgt) const
{
      tgt->net_assign(o, this);
}

void NetConst::emit_node(ostream&o, struct target_t*tgt) const
{
      tgt->net_const(o, this);
}

void NetNEvent::emit_node(ostream&o, struct target_t*tgt) const
{
      tgt->net_event(o, this);
}

void NetBUFZ::emit_node(ostream&o, struct target_t*tgt) const
{
      tgt->bufz(o, this);
}

void NetProcTop::emit(ostream&o, struct target_t*tgt) const
{
      assert(statement_);
      tgt->start_process(o, this);

      statement_->emit_proc(o, tgt);

      tgt->end_process(o, this);
}

void NetProc::emit_proc(ostream&o, struct target_t*tgt) const
{
      cerr << "EMIT: Proc type? " << typeid(*this).name() << endl;
}

void NetAssign::emit_proc(ostream&o, struct target_t*tgt) const
{
      tgt->proc_assign(o, this);
}

void NetBlock::emit_proc(ostream&o, struct target_t*tgt) const
{
      tgt->proc_block(o, this);
}

void NetCase::emit_proc(ostream&o, struct target_t*tgt) const
{
      tgt->proc_case(o, this);
}

void NetCondit::emit_proc(ostream&o, struct target_t*tgt) const
{
      tgt->proc_condit(o, this);
}

void NetPDelay::emit_proc(ostream&o, struct target_t*tgt) const
{
      tgt->proc_delay(o, this);
}

void NetPDelay::emit_proc_recurse(ostream&o, struct target_t*tgt) const
{
      if (statement_) statement_->emit_proc(o, tgt);
}

void NetPEvent::emit_proc(ostream&o, struct target_t*tgt) const
{
      tgt->proc_event(o, this);
}

void NetPEvent::emit_proc_recurse(ostream&o, struct target_t*tgt) const
{
      if (statement_) statement_->emit_proc(o, tgt);
}

void NetTask::emit_proc(ostream&o, struct target_t*tgt) const
{
      tgt->proc_task(o, this);
}

void NetWhile::emit_proc(ostream&o, struct target_t*tgt) const
{
      tgt->proc_while(o, this);
}

void NetBlock::emit_recurse(ostream&o, struct target_t*tgt) const
{
      if (last_ == 0)
	    return;

      NetProc*cur = last_;
      do {
	    cur = cur->next_;
	    cur->emit_proc(o, tgt);
      } while (cur != last_);
}

void NetCondit::emit_recurse_if(ostream&o, struct target_t*tgt) const
{
      if (if_)
	    if_->emit_proc(o, tgt);
}

void NetCondit::emit_recurse_else(ostream&o, struct target_t*tgt) const
{
      if (else_)
	    else_->emit_proc(o, tgt);
}

void NetWhile::emit_proc_recurse(ostream&o, struct target_t*tgt) const
{
      proc_->emit_proc(o, tgt);
}

void Design::emit(ostream&o, struct target_t*tgt) const
{
      tgt->start_design(o, this);

	// emit signals
      {
	    NetNet*cur = signals_->sig_next_;
	    do {
		  tgt->signal(o, cur);
		  cur = cur->sig_next_;
	    } while (cur != signals_->sig_next_);
      }


	// emit memories
      {
	    map<string,NetMemory*>::const_iterator mi;
	    for (mi = memories_.begin() ;  mi != memories_.end() ; mi++) {
		  tgt->memory(o, (*mi).second);
	    }
      }


	// emit nodes
      {
	    NetNode*cur = nodes_->node_next_;
	    do {
		  cur->emit_node(o, tgt);
		  cur = cur->node_next_;
	    } while (cur != nodes_->node_next_);
      }


	// emit the processes
      for (const NetProcTop*idx = procs_ ;  idx ;  idx = idx->next_)
	    idx->emit(o, tgt);

      tgt->end_design(o, this);
}

void NetEBinary::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_binary(this);
}

void NetEConst::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_const(this);
}

void NetEIdent::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_ident(this);
}

void NetEMemory::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_memory(this);
}

void NetESignal::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_signal(this);
}

void NetESignal::emit_node(ostream&o, struct target_t*tgt) const
{
      tgt->net_esignal(o, this);
}

void NetESubSignal::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_subsignal(this);
}

void NetEUnary::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_unary(this);
}

void emit(ostream&o, const Design*des, const char*type)
{
      for (unsigned idx = 0 ;  target_table[idx] ;  idx += 1) {
	    const struct target*tgt = target_table[idx];
	    if (tgt->name == type) {
		  des->emit(o, tgt->meth);
		  return;
	    }
      }
}


/*
 * $Log: emit.cc,v $
 * Revision 1.9  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.8  1999/04/25 00:44:10  steve
 *  Core handles subsignal expressions.
 *
 * Revision 1.7  1999/04/19 01:59:36  steve
 *  Add memories to the parse and elaboration phases.
 *
 * Revision 1.6  1999/02/08 02:49:56  steve
 *  Turn the NetESignal into a NetNode so
 *  that it can connect to the netlist.
 *  Implement the case statement.
 *  Convince t-vvm to output code for
 *  the case statement.
 *
 * Revision 1.5  1999/02/01 00:26:49  steve
 *  Carry some line info to the netlist,
 *  Dump line numbers for processes.
 *  Elaborate prints errors about port vector
 *  width mismatch
 *  Emit better handles null statements.
 *
 * Revision 1.4  1998/12/01 00:42:14  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.3  1998/11/09 18:55:34  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.2  1998/11/07 17:05:05  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:28:57  steve
 *  Introduce verilog to CVS.
 *
 */

