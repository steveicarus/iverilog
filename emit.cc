/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: emit.cc,v 1.53 2000/09/17 21:26:15 steve Exp $"
#endif

/*
 * The emit function is called to generate the output required of the
 * target.
 */
# include  "target.h"
# include  "netlist.h"
# include  <typeinfo>
# include  <cassert>

bool NetNode::emit_node(struct target_t*tgt) const
{
      cerr << "EMIT: Gate type? " << typeid(*this).name() << endl;
      return false;
}

bool NetLogic::emit_node(struct target_t*tgt) const
{
      tgt->logic(this);
      return true;
}

bool NetUDP_COMB::emit_node(struct target_t*tgt) const
{
      tgt->udp_comb(this);
      return true;
}

bool NetUDP::emit_node(struct target_t*tgt) const
{
      tgt->udp(this);
      return true;
}

bool NetAddSub::emit_node(struct target_t*tgt) const
{
      tgt->lpm_add_sub(this);
      return true;
}

bool NetAssign_::emit_node(struct target_t*tgt) const
{
      tgt->net_assign(this);
      return true;
}

bool NetCaseCmp::emit_node(struct target_t*tgt) const
{
      tgt->net_case_cmp(this);
      return true;
}

bool NetCAssign::emit_node(struct target_t*tgt) const
{
      tgt->net_cassign(this);
      return true;
}

bool NetCLShift::emit_node(struct target_t*tgt) const
{
      tgt->lpm_clshift(this);
      return true;
}

bool NetCompare::emit_node(struct target_t*tgt) const
{
      tgt->lpm_compare(this);
      return true;
}

bool NetConst::emit_node(struct target_t*tgt) const
{
      return tgt->net_const(this);
}

bool NetDivide::emit_node(struct target_t*tgt) const
{
      tgt->lpm_divide(this);
      return true;
}

bool NetFF::emit_node(struct target_t*tgt) const
{
      tgt->lpm_ff(this);
      return true;
}

bool NetForce::emit_node(struct target_t*tgt) const
{
      tgt->net_force(this);
      return true;
}

bool NetModulo::emit_node(struct target_t*tgt) const
{
      tgt->lpm_modulo(this);
      return true;
}

bool NetMult::emit_node(struct target_t*tgt) const
{
      tgt->lpm_mult(this);
      return true;
}

bool NetMux::emit_node(struct target_t*tgt) const
{
      tgt->lpm_mux(this);
      return true;
}

bool NetRamDq::emit_node(struct target_t*tgt) const
{
      tgt->lpm_ram_dq(this);
      return true;
}

bool NetBUFZ::emit_node(struct target_t*tgt) const
{
      return tgt->bufz(this);
}

bool NetProcTop::emit(struct target_t*tgt) const
{
      return tgt->process(this);
}

bool NetProc::emit_proc(struct target_t*tgt) const
{
      cerr << "EMIT: Proc type? " << typeid(*this).name() << endl;
      return false;
}

bool NetAssign::emit_proc(struct target_t*tgt) const
{
      tgt->proc_assign(this);
      return true;
}

bool NetAssignNB::emit_proc(struct target_t*tgt) const
{
      tgt->proc_assign_nb(this);
      return true;
}

bool NetAssignMem::emit_proc(struct target_t*tgt) const
{
      tgt->proc_assign_mem(this);
      return true;
}

bool NetAssignMemNB::emit_proc(struct target_t*tgt) const
{
      tgt->proc_assign_mem_nb(this);
      return true;
}

bool NetBlock::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_block(this);
}

bool NetCase::emit_proc(struct target_t*tgt) const
{
      tgt->proc_case(this);
      return true;
}

bool NetCAssign::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_cassign(this);
}

bool NetCondit::emit_proc(struct target_t*tgt) const
{
      tgt->proc_condit(this);
      return true;
}

bool NetDeassign::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_deassign(this);
}

bool NetDisable::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_disable(this);
}

bool NetForce::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_force(this);
}

bool NetForever::emit_proc(struct target_t*tgt) const
{
      tgt->proc_forever(this);
      return true;
}

bool NetPDelay::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_delay(this);
}

bool NetPDelay::emit_proc_recurse(struct target_t*tgt) const
{
      if (statement_) return statement_->emit_proc(tgt);
      return true;
}

bool NetRelease::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_release(this);
}

bool NetRepeat::emit_proc(struct target_t*tgt) const
{
      tgt->proc_repeat(this);
      return true;
}

bool NetSTask::emit_proc(struct target_t*tgt) const
{
      tgt->proc_stask(this);
      return true;
}

bool NetUTask::emit_proc(struct target_t*tgt) const
{
      tgt->proc_utask(this);
      return true;
}

bool NetWhile::emit_proc(struct target_t*tgt) const
{
      tgt->proc_while(this);
      return true;
}

void NetBlock::emit_recurse(struct target_t*tgt) const
{
      if (last_ == 0)
	    return;

      NetProc*cur = last_;
      do {
	    cur = cur->next_;
	    cur->emit_proc(tgt);
      } while (cur != last_);
}

void NetCondit::emit_recurse_if(struct target_t*tgt) const
{
      if (if_)
	    if_->emit_proc(tgt);
}

void NetCondit::emit_recurse_else(struct target_t*tgt) const
{
      if (else_)
	    else_->emit_proc(tgt);
}

bool NetEvProbe::emit_node(struct target_t*tgt) const
{
      tgt->net_probe(this);
      return true;
}

bool NetEvTrig::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_trigger(this);
}

bool NetEvWait::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_wait(this);
}

bool NetEvWait::emit_recurse(struct target_t*tgt) const
{
      if (!statement_) return true;
      return statement_->emit_proc(tgt);
}

void NetForever::emit_recurse(struct target_t*tgt) const
{
      if (statement_)
	    statement_->emit_proc(tgt);
}

void NetRepeat::emit_recurse(struct target_t*tgt) const
{
      if (statement_)
	    statement_->emit_proc(tgt);
}

void NetScope::emit_scope(struct target_t*tgt) const
{
      tgt->scope(this);

      for (NetEvent*cur = events_ ;  cur ;  cur = cur->snext_)
	    tgt->event(cur);

      for (NetScope*cur = sub_ ;  cur ;  cur = cur->sib_)
	    cur->emit_scope(tgt);

      if (signals_) {
	    NetNet*cur = signals_->sig_next_;
	    do {
		  tgt->signal(cur);
		  cur = cur->sig_next_;
	    } while (cur != signals_->sig_next_);
      }

      if (memories_) {
	    NetMemory*cur = memories_->snext_;
	    do {
		  tgt->memory(cur);
		  cur = cur->snext_;
	    } while (cur != memories_->snext_);
      }
}

void NetScope::emit_defs(struct target_t*tgt) const
{

      switch (type_) {
	  case MODULE:
	    for (NetScope*cur = sub_ ;  cur ;  cur = cur->sib_)
		  cur->emit_defs(tgt);
	    break;

	  case FUNC:
	    tgt->func_def(this->func_def());
	    break;
	  case TASK:
	    tgt->task_def(this->task_def());
	    break;
      }

}

void NetWhile::emit_proc_recurse(struct target_t*tgt) const
{
      proc_->emit_proc(tgt);
}

bool Design::emit(struct target_t*tgt) const
{
      bool rc = true;

      rc = rc && tgt->start_design(this);
      if (rc == false)
	    return false;

	// enumerate the scopes
      root_scope_->emit_scope(tgt);


	// emit nodes
      if (nodes_) {
	    NetNode*cur = nodes_->node_next_;
	    do {
		  rc = rc && cur->emit_node(tgt);
		  cur = cur->node_next_;
	    } while (cur != nodes_->node_next_);
      }


	// emit task and function definitions
      root_scope_->emit_defs(tgt);


	// emit the processes
      for (const NetProcTop*idx = procs_ ;  idx ;  idx = idx->next_)
	    rc = rc && idx->emit(tgt);

      tgt->end_design(this);
      return rc;
}

void NetEBinary::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_binary(this);
}

void NetEConcat::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_concat(this);
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

void NetEParam::expr_scan(struct expr_scan_t*tgt) const
{
      cerr << get_line() << ":internal error: unexpected NetEParam."
	   << endl;
}

void NetEScope::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_scope(this);
}

void NetESFunc::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_sfunc(this);
}

void NetEUFunc::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_ufunc(this);
}

void NetESignal::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_signal(this);
}

void NetESubSignal::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_subsignal(this);
}

void NetETernary::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_ternary(this);
}

void NetEUnary::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_unary(this);
}

bool emit(const Design*des, const char*type)
{
      for (unsigned idx = 0 ;  target_table[idx] ;  idx += 1) {
	    const struct target*tgt = target_table[idx];
	    if (tgt->name == type)
		  return des->emit(tgt->meth);

      }
}


/*
 * $Log: emit.cc,v $
 * Revision 1.53  2000/09/17 21:26:15  steve
 *  Add support for modulus (Eric Aardoom)
 *
 * Revision 1.52  2000/09/02 20:54:20  steve
 *  Rearrange NetAssign to make NetAssign_ separate.
 *
 * Revision 1.51  2000/08/14 04:39:56  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.50  2000/08/09 03:43:45  steve
 *  Move all file manipulation out of target class.
 *
 * Revision 1.49  2000/08/08 01:50:42  steve
 *  target methods need not take a file stream.
 *
 * Revision 1.48  2000/07/30 18:25:43  steve
 *  Rearrange task and function elaboration so that the
 *  NetTaskDef and NetFuncDef functions are created during
 *  signal enaboration, and carry these objects in the
 *  NetScope class instead of the extra, useless map in
 *  the Design class.
 *
 * Revision 1.47  2000/07/29 16:21:08  steve
 *  Report code generation errors through proc_delay.
 *
 * Revision 1.46  2000/07/27 05:13:44  steve
 *  Support elaboration of disable statements.
 *
 * Revision 1.45  2000/05/11 23:37:27  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.44  2000/05/04 03:37:58  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.43  2000/05/02 03:13:31  steve
 *  Move memories to the NetScope object.
 *
 * Revision 1.42  2000/05/02 00:58:12  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.41  2000/04/23 03:45:24  steve
 *  Add support for the procedural release statement.
 *
 * Revision 1.40  2000/04/22 04:20:19  steve
 *  Add support for force assignment.
 *
 * Revision 1.39  2000/04/12 04:23:58  steve
 *  Named events really should be expressed with PEIdent
 *  objects in the pform,
 *
 *  Handle named events within the mix of net events
 *  and edges. As a unified lot they get caught together.
 *  wait statements are broken into more complex statements
 *  that include a conditional.
 *
 *  Do not generate NetPEvent or NetNEvent objects in
 *  elaboration. NetEvent, NetEvWait and NetEvProbe
 *  take over those functions in the netlist.
 *
 * Revision 1.38  2000/04/10 05:26:06  steve
 *  All events now use the NetEvent class.
 *
 * Revision 1.37  2000/04/04 03:20:15  steve
 *  Simulate named event trigger and waits.
 *
 * Revision 1.36  2000/04/01 21:40:22  steve
 *  Add support for integer division.
 *
 * Revision 1.35  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 * Revision 1.34  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.33  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.32  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.31  1999/11/28 23:42:02  steve
 *  NetESignal object no longer need to be NetNode
 *  objects. Let them keep a pointer to NetNet objects.
 *
 * Revision 1.30  1999/11/27 19:07:57  steve
 *  Support the creation of scopes.
 *
 * Revision 1.29  1999/11/21 00:13:08  steve
 *  Support memories in continuous assignments.
 *
 * Revision 1.28  1999/11/14 23:43:45  steve
 *  Support combinatorial comparators.
 *
 * Revision 1.27  1999/11/14 20:24:28  steve
 *  Add support for the LPM_CLSHIFT device.
 *
 * Revision 1.26  1999/11/04 03:53:26  steve
 *  Patch to synthesize unary ~ and the ternary operator.
 *  Thanks to Larry Doolittle <LRDoolittle@lbl.gov>.
 *
 *  Add the LPM_MUX device, and integrate it with the
 *  ternary synthesis from Larry. Replace the lpm_mux
 *  generator in t-xnf.cc to use XNF EQU devices to
 *  put muxs into function units.
 *
 *  Rewrite elaborate_net for the PETernary class to
 *  also use the LPM_MUX device.
 *
 * Revision 1.25  1999/11/01 02:07:40  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.24  1999/10/10 01:59:54  steve
 *  Structural case equals device.
 *
 * Revision 1.23  1999/09/22 16:57:23  steve
 *  Catch parallel blocks in vvm emit.
 *
 * Revision 1.22  1999/09/20 02:21:10  steve
 *  Elaborate parameters in phases.
 *
 * Revision 1.21  1999/09/15 01:55:06  steve
 *  Elaborate non-blocking assignment to memories.
 *
 * Revision 1.20  1999/09/03 04:28:38  steve
 *  elaborate the binary plus operator.
 *
 * Revision 1.19  1999/08/31 22:38:29  steve
 *  Elaborate and emit to vvm procedural functions.
 *
 * Revision 1.18  1999/07/17 19:50:59  steve
 *  netlist support for ternary operator.
 *
 * Revision 1.17  1999/07/17 03:39:11  steve
 *  simplified process scan for targets.
 *
 * Revision 1.16  1999/07/07 04:20:57  steve
 *  Emit vvm for user defined tasks.
 *
 * Revision 1.15  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.14  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.13  1999/06/09 03:00:06  steve
 *  Add support for procedural concatenation expression.
 *
 * Revision 1.12  1999/06/06 20:45:38  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.11  1999/05/12 04:03:19  steve
 *  emit NetAssignMem objects in vvm target.
 *
 * Revision 1.10  1999/05/07 01:21:18  steve
 *  Handle total lack of nodes and signals.
 *
 * Revision 1.9  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.8  1999/04/25 00:44:10  steve
 *  Core handles subsignal expressions.
 */

