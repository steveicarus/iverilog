/*
 * Copyright (c) 1998-2008 Stephen Williams (steve@icarus.com)
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

/*
 * The emit function is called to generate the output required of the
 * target.
 */
# include  "target.h"
# include  "netlist.h"
# include  <typeinfo>
# include  <cassert>
# include  <cstring>

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

bool NetUDP::emit_node(struct target_t*tgt) const
{
      tgt->udp(this);
      return true;
}

bool NetAbs::emit_node(struct target_t*tgt) const
{
      tgt->lpm_abs(this);
      return true;
}

bool NetAddSub::emit_node(struct target_t*tgt) const
{
      tgt->lpm_add_sub(this);
      return true;
}

bool NetArrayDq::emit_node(struct target_t*tgt) const
{
      return tgt->lpm_array_dq(this);
}

bool NetCaseCmp::emit_node(struct target_t*tgt) const
{
      tgt->net_case_cmp(this);
      return true;
}

bool NetCastInt::emit_node(struct target_t*tgt) const
{
      return tgt->lpm_cast_int(this);
}

bool NetCastReal::emit_node(struct target_t*tgt) const
{
      return tgt->lpm_cast_real(this);
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

bool NetConcat::emit_node(struct target_t*tgt) const
{
      return tgt->concat(this);
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

bool NetLiteral::emit_node(struct target_t*tgt) const
{
      return tgt->net_literal(this);
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

bool NetPartSelect::emit_node(struct target_t*tgt) const
{
      return tgt->part_select(this);
}

bool NetPow::emit_node(struct target_t*tgt) const
{
      tgt->lpm_pow(this);
      return true;
}

bool NetReplicate::emit_node(struct target_t*tgt) const
{
      return tgt->replicate(this);
}

bool NetSignExtend::emit_node(struct target_t*tgt) const
{
      return tgt->sign_extend(this);
}

bool NetUReduce::emit_node(struct target_t*tgt) const
{
      return tgt->ureduce(this);
}

bool NetSysFunc::emit_node(struct target_t*tgt) const
{
      return tgt->net_sysfunction(this);
}

bool NetUserFunc::emit_node(struct target_t*tgt) const
{
      return tgt->net_function(this);
}

bool NetTran::emit_node(struct target_t*tgt) const
{
      return tgt->tran(this);
}

bool NetBUFZ::emit_node(struct target_t*tgt) const
{
      return tgt->bufz(this);
}

bool NetProcTop::emit(struct target_t*tgt) const
{
      return tgt->process(this);
}

bool NetAnalogTop::emit(struct target_t*tgt) const
{
      return tgt->process(this);
}

bool NetProc::emit_proc(struct target_t*tgt) const
{
      cerr << "EMIT: Proc type? " << typeid(*this).name() << endl;
      return false;
}

bool NetAlloc::emit_proc(struct target_t*tgt) const
{
      tgt->proc_alloc(this);
      return true;
}

bool NetAssign::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_assign(this);
}

bool NetAssignNB::emit_proc(struct target_t*tgt) const
{
      tgt->proc_assign_nb(this);
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
      return tgt->proc_condit(this);
}

bool NetContribution::emit_proc(struct target_t*tgt) const
{
      return tgt->proc_contribution(this);
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

bool NetFree::emit_proc(struct target_t*tgt) const
{
      tgt->proc_free(this);
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

bool NetCondit::emit_recurse_if(struct target_t*tgt) const
{
      if (if_)
	    return if_->emit_proc(tgt);
      else
	    return true;
}

bool NetCondit::emit_recurse_else(struct target_t*tgt) const
{
      if (else_)
	    return else_->emit_proc(tgt);
      else
	    return true;
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

      for (signals_map_iter_t cur = signals_map_.begin()
		 ; cur != signals_map_.end() ; cur ++) {
	    tgt->signal(cur->second);
      }

	// Run the signals again, but this time to connect the
	// delay paths. This is done as a second pass because
	// the paths reference other signals that may be later
	// in the list. We can do it here because delay paths are
	// always connected within the scope.
      for (signals_map_iter_t cur = signals_map_.begin()
		 ; cur != signals_map_.end() ; cur ++) {

	    tgt->signal_paths(cur->second);
      }

}

bool NetScope::emit_defs(struct target_t*tgt) const
{
      bool flag = true;

      switch (type_) {
	  case MODULE:
	    for (NetScope*cur = sub_ ;  cur ;  cur = cur->sib_)
		  flag &= cur->emit_defs(tgt);
	    break;

	  case FUNC:
	    flag &= tgt->func_def(this);
	    break;
	  case TASK:
	    tgt->task_def(this);
	    break;
	  default:  /* BEGIN_END and FORK_JOIN, GENERATE... */
	    for (NetScope*cur = sub_ ;  cur ;  cur = cur->sib_)
		  flag &= cur->emit_defs(tgt);
	    break;
      }

      return flag;
}

void NetWhile::emit_proc_recurse(struct target_t*tgt) const
{
      proc_->emit_proc(tgt);
}

int Design::emit(struct target_t*tgt) const
{
      int rc = 0;

      if (tgt->start_design(this) == false)
	    return -2;

	// enumerate the scopes
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin();
	   scope != root_scopes_.end(); scope++)
	    (*scope)->emit_scope(tgt);


	// emit nodes
      bool nodes_rc = true;
      if (nodes_) {
	    NetNode*cur = nodes_->node_next_;
	    do {
		  nodes_rc = nodes_rc && cur->emit_node(tgt);
		  cur = cur->node_next_;
	    } while (cur != nodes_->node_next_);
      }


      bool branches_rc = true;
      for (NetBranch*cur = branches_ ; cur ; cur = cur->next_) {
	    branches_rc = tgt->branch(cur) && branches_rc;
      }

	// emit task and function definitions
      bool tasks_rc = true;
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin();
	   scope != root_scopes_.end(); scope++)
	    tasks_rc &= (*scope)->emit_defs(tgt);


	// emit the processes
      bool proc_rc = true;
      for (const NetProcTop*idx = procs_ ;  idx ;  idx = idx->next_)
	    proc_rc &= idx->emit(tgt);
      for (const NetAnalogTop*idx = aprocs_ ;  idx ;  idx = idx->next_)
	    proc_rc &= idx->emit(tgt);

      rc = tgt->end_design(this);

      if (nodes_rc == false)
	    return -1;
      if (tasks_rc == false)
	    return -2;
      if (proc_rc == false)
	  return -3;
      if (branches_rc == false)
	    return -4;

      return rc;
}

void NetEAccess::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_access_func(this);
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

void NetEConstParam::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_param(this);
}

void NetECReal::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_creal(this);
}

void NetECRealParam::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_rparam(this);
}

void NetEParam::expr_scan(struct expr_scan_t*tgt) const
{
      cerr << get_fileline() << ":internal error: unexpected NetEParam."
	   << endl;
}

void NetEEvent::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_event(this);
}

void NetEScope::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_scope(this);
}

void NetESelect::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_select(this);
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

void NetETernary::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_ternary(this);
}

void NetEUnary::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_unary(this);
}
