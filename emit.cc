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
#ifdef HAVE_CVS_IDENT
#ident "$Id: emit.cc,v 1.76 2004/05/31 23:34:37 steve Exp $"
#endif

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

bool NetAddSub::emit_node(struct target_t*tgt) const
{
      tgt->lpm_add_sub(this);
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

bool NetUserFunc::emit_node(struct target_t*tgt) const
{
      return tgt->net_function(this);
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

      for (NetVariable*cur = vars_ ;  cur ;  cur = cur->snext_)
	    tgt->variable(cur);

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
	  default:  /* BEGIN_END and FORK_JOIN, do nothing */
	    break;
      }

      return flag;
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
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin(); 
	   scope != root_scopes_.end(); scope++)
	    (*scope)->emit_scope(tgt);


	// emit nodes
      if (nodes_) {
	    NetNode*cur = nodes_->node_next_;
	    do {
		  rc = rc && cur->emit_node(tgt);
		  cur = cur->node_next_;
	    } while (cur != nodes_->node_next_);
      }


	// emit task and function definitions
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin(); 
	   scope != root_scopes_.end(); scope++)
	    rc &= (*scope)->emit_defs(tgt);


	// emit the processes
      for (const NetProcTop*idx = procs_ ;  idx ;  idx = idx->next_)
	    rc = rc && idx->emit(tgt);

      if (tgt->end_design(this) != 0)
	    rc = false;

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

void NetEMemory::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_memory(this);
}

void NetEParam::expr_scan(struct expr_scan_t*tgt) const
{
      cerr << get_line() << ":internal error: unexpected NetEParam."
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

void NetEBitSel::expr_scan(struct expr_scan_t*tgt) const
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

void NetEVariable::expr_scan(struct expr_scan_t*tgt) const
{
      tgt->expr_variable(this);
}

bool emit(const Design*des, const char*type)
{
      for (unsigned idx = 0 ;  target_table[idx] ;  idx += 1) {
	    const struct target*tgt = target_table[idx];
	    if (strcmp(tgt->name, type) == 0)
		  return des->emit(tgt->meth);

      }

      cerr << "error: Code generator type " << type
	   << " not found." << endl;
      return false;
}


/*
 * $Log: emit.cc,v $
 * Revision 1.76  2004/05/31 23:34:37  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.75  2003/09/13 01:30:07  steve
 *  Missing case warnings.
 *
 * Revision 1.74  2003/05/30 02:55:32  steve
 *  Support parameters in real expressions and
 *  as real expressions, and fix multiply and
 *  divide with real results.
 *
 * Revision 1.73  2003/04/22 04:48:29  steve
 *  Support event names as expressions elements.
 *
 * Revision 1.72  2003/03/10 23:40:53  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.71  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.70  2002/11/03 20:36:10  steve
 *  Error message for mising code generator type.
 *
 * Revision 1.69  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.68  2002/06/05 03:44:25  steve
 *  Add support for memory words in l-value of
 *  non-blocking assignments, and remove the special
 *  NetAssignMem_ and NetAssignMemNB classes.
 *
 * Revision 1.67  2002/06/04 05:38:44  steve
 *  Add support for memory words in l-value of
 *  blocking assignments, and remove the special
 *  NetAssignMem class.
 *
 * Revision 1.66  2002/03/09 02:10:22  steve
 *  Add the NetUserFunc netlist node.
 *
 * Revision 1.65  2002/01/28 00:52:41  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.64  2002/01/19 19:02:08  steve
 *  Pass back target errors processing conditionals.
 *
 * Revision 1.63  2001/10/19 21:53:24  steve
 *  Support multiple root modules (Philip Blundell)
 *
 * Revision 1.62  2001/08/25 23:50:02  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.61  2001/07/27 04:51:44  steve
 *  Handle part select expressions as variants of
 *  NetESignal/IVL_EX_SIGNAL objects, instead of
 *  creating new and useless temporary signals.
 *
 * Revision 1.60  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.59  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.58  2001/04/06 02:28:02  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.57  2001/04/02 02:28:12  steve
 *  Generate code for task calls.
 *
 * Revision 1.56  2001/03/27 03:31:06  steve
 *  Support error code from target_t::end_design method.
 *
 * Revision 1.55  2000/11/04 01:54:01  steve
 *  Modifications in support of gcc 2.96
 *
 * Revision 1.54  2000/09/26 01:35:42  steve
 *  Remove the obsolete NetEIdent class.
 *
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
 */

