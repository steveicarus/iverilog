/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.will need a Picture Elements Binary Software
 *    License.
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
#ident "$Id: t-dll-proc.cc,v 1.65 2004/10/04 01:10:55 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

# include  "target.h"
# include  "ivl_target.h"
# include  "compiler.h"
# include  "t-dll.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>


bool dll_target::process(const NetProcTop*net)
{
      ivl_process_t obj = (struct ivl_process_s*)
	    calloc(1, sizeof(struct ivl_process_s));

      switch (net->type()) {
	  case NetProcTop::KINITIAL:
	    obj->type_ = IVL_PR_INITIAL;
	    break;
	  case NetProcTop::KALWAYS:
	    obj->type_ = IVL_PR_ALWAYS;
	    break;
	  default:
	    assert(0);
      }

	/* Save the scope of the process. */
      obj->scope_ = lookup_scope_(net->scope());

      obj->nattr = net->attr_cnt();
      obj->attr = fill_in_attributes(net);

	/* This little bit causes the process to be completely
	   generated so that it can be passed to the DLL. The
	   stmt_cur_ member us used to hold a pointer to the current
	   statement in progress, and the emit_proc() method fills in
	   that object.

	   We know a few things about the current statement: we are
	   not in the middle of one, and when we are done, we have our
	   statement back. The asserts check these conditions. */

      assert(stmt_cur_ == 0);
      stmt_cur_ = (struct ivl_statement_s*)calloc(1, sizeof*stmt_cur_);
      assert(stmt_cur_);
      net->statement()->emit_proc(this);

      assert(stmt_cur_);
      obj->stmt_ = stmt_cur_;
      stmt_cur_ = 0;

	/* Save the process in the design. */
      obj->next_ = des_.threads_;
      des_.threads_ = obj;

      return true;
}

void dll_target::task_def(const NetScope*net)
{
      ivl_scope_t scope = lookup_scope_(net);
      const NetTaskDef*def = net->task_def();

      assert(stmt_cur_ == 0);
      stmt_cur_ = (struct ivl_statement_s*)calloc(1, sizeof*stmt_cur_);
      assert(stmt_cur_);
      def->proc()->emit_proc(this);

      assert(stmt_cur_);
      scope->def = stmt_cur_;
      stmt_cur_ = 0;

}

bool dll_target::func_def(const NetScope*net)
{
      ivl_scope_t scope = lookup_scope_(net);
      const NetFuncDef*def = net->func_def();

      assert(stmt_cur_ == 0);
      stmt_cur_ = (struct ivl_statement_s*)calloc(1, sizeof*stmt_cur_);
      assert(stmt_cur_);
      def->proc()->emit_proc(this);

      assert(stmt_cur_);
      scope->def = stmt_cur_;
      stmt_cur_ = 0;

      scope->ports = def->port_count() + 1;
      if (scope->ports > 0) {
	    scope->port = new ivl_signal_t[scope->ports];
	    for (unsigned idx = 1 ;  idx < scope->ports ;  idx += 1)
		  scope->port[idx] = find_signal(des_, def->port(idx-1));
      }

	/* FIXME: the ivl_target API expects port-0 to be the output
	   port. This assumes that the return value is a signal, which
	   is *not* correct. Someday, I'm going to have to change
	   this, but that will break code generators that use this
	   result. */
      if (const NetNet*ret_sig = def->return_sig()) {
	    scope->port[0] = find_signal(des_, ret_sig);
	    return true;
      }

      if (const NetVariable*ret_var = def->return_var()) {
	    cerr << ret_var->get_line() << ": internal error: "
		 << "Function " << net->name() << " has an unsupported "
		 << "return type." << endl;
	    return false;
      }

      cerr << "?:0" << ": internal error: "
	   << "Function " << net->name() << " has a return type"
	   << " that I do not understand." << endl;

      return false;
}

/*
 */
void dll_target::proc_assign(const NetAssign*net)
{
      unsigned cnt;

      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_ASSIGN;

      stmt_cur_->u_.assign_.lvals_ = cnt = net->l_val_count();
      stmt_cur_->u_.assign_.lval_ = new struct ivl_lval_s[cnt];
      stmt_cur_->u_.assign_.delay = 0;

	/* The assignment may have multiple concatenated
	   l-values. Scan them and accumulate an ivl_lval_t list. */
      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
	    struct ivl_lval_s*cur = stmt_cur_->u_.assign_.lval_ + idx;
	    const NetAssign_*asn = net->l_val(idx);

	    cur->width_ = asn->lwidth();
	    cur->loff_  = asn->get_loff();
	    if (asn->sig()) {
		  cur->type_  = IVL_LVAL_REG;
		  cur->n.sig  = find_signal(des_, asn->sig());

		  cur->idx = 0;
		  if (asn->bmux()) {
			assert(expr_ == 0);
			asn->bmux()->expr_scan(this);

			if (cur->n.sig->lsb_index != 0)
			      sub_off_from_expr_(asn->sig()->lsb());
			if (cur->n.sig->lsb_dist != 1)
			      mul_expr_by_const_(cur->n.sig->lsb_dist);

			cur->type_ = IVL_LVAL_MUX;
			cur->idx = expr_;
			expr_ = 0;
		  }

	    } else if (asn->var()) {
		  cur->type_ = IVL_LVAL_VAR;
		  cur->idx = 0;
		  cur->n.var = find_variable(des_, asn->var());

	    } else {
		  assert(asn->mem());
		  cur->type_ = IVL_LVAL_MEM;
		  cur->n.mem = find_memory(des_, asn->mem());
		  assert(cur->n.mem);
		  cur->width_ = ivl_memory_width(cur->n.mem);

		  assert(expr_ == 0);
		  asn->bmux()->expr_scan(this);
		  cur->idx = expr_;
		  expr_ = 0;
	    }
      }

      assert(expr_ == 0);
      net->rval()->expr_scan(this);
      stmt_cur_->u_.assign_.rval_ = expr_;
      expr_ = 0;

      const NetExpr*del = net->get_delay();
      if (del) {
	    del->expr_scan(this);
	    stmt_cur_->u_.assign_.delay = expr_;
	    expr_ = 0;
      }
}


void dll_target::proc_assign_nb(const NetAssignNB*net)
{
      unsigned cnt = net->l_val_count();

      const NetExpr* delay_exp = net->get_delay();
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_ASSIGN_NB;

      stmt_cur_->u_.assign_.lvals_ = cnt;
      stmt_cur_->u_.assign_.lval_  = new struct ivl_lval_s[cnt];
      stmt_cur_->u_.assign_.delay  = 0;

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
	    struct ivl_lval_s*cur = stmt_cur_->u_.assign_.lval_ + idx;
	    const NetAssign_*asn = net->l_val(idx);

	    cur->width_ = asn->lwidth();
	    cur->loff_  = asn->get_loff();
	    if (asn->sig()) {
		  cur->type_ = IVL_LVAL_REG;
		  cur->n.sig = find_signal(des_, asn->sig());

		  cur->idx = 0;
		  if (asn->bmux()) {
			assert(expr_ == 0);
			asn->bmux()->expr_scan(this);

			if (cur->n.sig->lsb_index != 0)
			      sub_off_from_expr_(asn->sig()->lsb());
			if (cur->n.sig->lsb_dist != 1)
			      mul_expr_by_const_(cur->n.sig->lsb_dist);

			cur->type_ = IVL_LVAL_MUX;
			cur->idx = expr_;
			expr_ = 0;
		  }
	    } else if (asn->mem()) {
		  assert(asn->mem());
		  cur->type_ = IVL_LVAL_MEM;
		  cur->n.mem = find_memory(des_, asn->mem());
		  assert(cur->n.mem);
		  cur->width_ = ivl_memory_width(cur->n.mem);

		  assert(expr_ == 0);
		  asn->bmux()->expr_scan(this);
		  cur->idx = expr_;
		  expr_ = 0;
	    } else {
		  assert(asn->var());
		  cur->type_ = IVL_LVAL_VAR;
		  cur->idx = 0;
		  cur->n.var = find_variable(des_, asn->var());

	    }
      }

      assert(expr_ == 0);
      net->rval()->expr_scan(this);
      stmt_cur_->u_.assign_.rval_ = expr_;
      expr_ = 0;

      if (const NetEConst*delay_num = dynamic_cast<const NetEConst*>(delay_exp)) {
	    verinum val = delay_num->value();
	    ivl_expr_t de = new struct ivl_expr_s;
	    de->type_ = IVL_EX_ULONG;
	    de->width_  = 8 * sizeof(unsigned long);
	    de->signed_ = 0;
	    de->u_.ulong_.value = val.as_ulong();
	    stmt_cur_->u_.assign_.delay = de;

      } else if (delay_exp != 0) {
	    delay_exp->expr_scan(this);
	    stmt_cur_->u_.assign_.delay = expr_;
	    expr_ = 0;
      }
}

bool dll_target::proc_block(const NetBlock*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

	/* First, count the statements in the block. */
      unsigned count = 0;
      for (const NetProc*cur = net->proc_first()
		 ;  cur ;  cur = net->proc_next(cur))
	    count += 1;

	/* If the block has no statements, then turn it into a no-op */
      if (count == 0) {
	    stmt_cur_->type_ = IVL_ST_NOOP;
	    return true;
      }

	/* If there is exactly one statement, there is no need for the
	   block wrapper, generate the contained statement instead. */
      if ((count == 1) && (net->subscope() == 0)) {
	    return net->proc_first()->emit_proc(this);
      }


	/* Handle the general case. The block has some statements in
	   it, so fill in the block fields of the existing statement,
	   and generate the contents for the statement array. */

      stmt_cur_->type_ = (net->type() == NetBlock::SEQU)
	    ? IVL_ST_BLOCK
	    : IVL_ST_FORK;
      stmt_cur_->u_.block_.nstmt_ = count;
      stmt_cur_->u_.block_.stmt_ = (struct ivl_statement_s*)
	    calloc(count, sizeof(struct ivl_statement_s));

      if (net->subscope())
	    stmt_cur_->u_.block_.scope = lookup_scope_(net->subscope());
      else
	    stmt_cur_->u_.block_.scope = 0;

      struct ivl_statement_s*save_cur_ = stmt_cur_;
      unsigned idx = 0;
      bool flag = true;

      for (const NetProc*cur = net->proc_first()
		 ;  cur ;  cur = net->proc_next(cur), idx += 1) {
	    assert(idx < count);
	    stmt_cur_ = save_cur_->u_.block_.stmt_ + idx;
	    bool rc = cur->emit_proc(this);
	    flag = flag && rc;
      }
      assert(idx == count);

      stmt_cur_ = save_cur_;

      return flag;
}

/*
 * A case statement is in turn an array of statements with gate
 * expressions. This builds arrays of the right size and builds the
 * ivl_expr_t and ivl_statement_s arrays for the substatements.
 */
void dll_target::proc_case(const NetCase*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      switch (net->type()) {
	  case NetCase::EQ:
	    stmt_cur_->type_ = IVL_ST_CASE;
	    break;
	  case NetCase::EQX:
	    stmt_cur_->type_ = IVL_ST_CASEX;
	    break;
	  case NetCase::EQZ:
	    stmt_cur_->type_ = IVL_ST_CASEZ;
	    break;
      }
      assert(stmt_cur_->type_ != IVL_ST_NONE);

      assert(expr_ == 0);
      assert(net->expr());
      net->expr()->expr_scan(this);
      stmt_cur_->u_.case_.cond = expr_;
      expr_ = 0;

	/* If the condition expression is a real valued expression,
	   then change the case statement to a CASER statement. */
      if (stmt_cur_->u_.case_.cond->value_ == IVL_VT_REAL)
	    stmt_cur_->type_ = IVL_ST_CASER;

      unsigned ncase = net->nitems();
      stmt_cur_->u_.case_.ncase = ncase;

      stmt_cur_->u_.case_.case_ex = new ivl_expr_t[ncase];
      stmt_cur_->u_.case_.case_st = new struct ivl_statement_s[ncase];

      ivl_statement_t save_cur = stmt_cur_;

      for (unsigned idx = 0 ;  idx < ncase ;  idx += 1) {
	    const NetExpr*ex = net->expr(idx);
	    if (ex) {
		  ex->expr_scan(this);
		  save_cur->u_.case_.case_ex[idx] = expr_;
		  expr_ = 0;
	    } else {
		  save_cur->u_.case_.case_ex[idx] = 0;
	    }

	    stmt_cur_ = save_cur->u_.case_.case_st + idx;
	    stmt_cur_->type_ = IVL_ST_NONE;
	    if (net->stat(idx) == 0) {
		  stmt_cur_->type_ = IVL_ST_NOOP;
	    } else {
		  net->stat(idx)->emit_proc(this);
	    }
      }

      stmt_cur_ = save_cur;
}

bool dll_target::proc_cassign(const NetCAssign*net)
{

      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_CASSIGN;

      stmt_cur_->u_.cassign_.lvals = 1;
      stmt_cur_->u_.cassign_.lval = (struct ivl_lval_s*)
	    calloc(1, sizeof(struct ivl_lval_s));

      const NetNet*lsig = net->lval();

      stmt_cur_->u_.cassign_.lval[0].width_ = lsig->pin_count();
      stmt_cur_->u_.cassign_.lval[0].loff_  = 0;
      stmt_cur_->u_.cassign_.lval[0].type_  = IVL_LVAL_REG;
      stmt_cur_->u_.cassign_.lval[0].idx    = 0;
      stmt_cur_->u_.cassign_.lval[0].n.sig  = find_signal(des_, lsig);

      stmt_cur_->u_.cassign_.npins = net->pin_count();
      stmt_cur_->u_.cassign_.pins = (ivl_nexus_t*)
	    calloc(stmt_cur_->u_.cassign_.npins, sizeof(ivl_nexus_t));

      ivl_nexus_t*ntmp = stmt_cur_->u_.cassign_.pins;
      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    ntmp[idx] = (ivl_nexus_t)net->pin(idx).nexus()->t_cookie();
	    assert(ntmp[idx]);
      }

      return true;
}

bool dll_target::proc_condit(const NetCondit*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_CONDIT;
      stmt_cur_->u_.condit_.stmt_ = (struct ivl_statement_s*)
	    calloc(2, sizeof(struct ivl_statement_s));

      assert(expr_ == 0);
      net->expr()->expr_scan(this);
      stmt_cur_->u_.condit_.cond_ = expr_;
      expr_ = 0;

      ivl_statement_t save_cur_ = stmt_cur_;

      stmt_cur_ = save_cur_->u_.condit_.stmt_+0;
      bool flag = net->emit_recurse_if(this);

      stmt_cur_ = save_cur_->u_.condit_.stmt_+1;
      flag = flag && net->emit_recurse_else(this);

      stmt_cur_ = save_cur_;
      return flag;
}

bool dll_target::proc_deassign(const NetDeassign*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_DEASSIGN;
      stmt_cur_->u_.cassign_.lvals = 1;
      stmt_cur_->u_.cassign_.lval = (struct ivl_lval_s*)
	    calloc(1, sizeof(struct ivl_lval_s));

      const NetNet*lsig = net->lval();

      stmt_cur_->u_.cassign_.lval[0].width_ = lsig->pin_count();
      stmt_cur_->u_.cassign_.lval[0].loff_  = 0;
      stmt_cur_->u_.cassign_.lval[0].type_  = IVL_LVAL_REG;
      stmt_cur_->u_.cassign_.lval[0].idx    = 0;
      stmt_cur_->u_.cassign_.lval[0].n.sig  = find_signal(des_, lsig);
      stmt_cur_->u_.cassign_.npins = 0;
      stmt_cur_->u_.cassign_.pins  = 0;

      return true;
}

bool dll_target::proc_delay(const NetPDelay*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      ivl_statement_t tmp = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      if (const NetExpr*expr = net->expr()) {

	    stmt_cur_->type_ = IVL_ST_DELAYX;
	    assert(expr_ == 0);
	    expr->expr_scan(this);
	    stmt_cur_->u_.delayx_.expr = expr_;
	    expr_ = 0;

	    stmt_cur_->u_.delayx_.stmt_ = tmp;

      } else {
	    stmt_cur_->type_ = IVL_ST_DELAY;
	    stmt_cur_->u_.delay_.stmt_  = tmp;
	    stmt_cur_->u_.delay_.delay_ = net->delay();
      }

      ivl_statement_t save_cur_ = stmt_cur_;
      stmt_cur_ = tmp;
      bool flag = net->emit_proc_recurse(this);

	/* If the recurse doesn't turn this new item into something,
	   then either it failed or there is no statement
	   there. Either way, draw a no-op into the statement. */
      if (stmt_cur_->type_ == IVL_ST_NONE) {
	    stmt_cur_->type_ = IVL_ST_NOOP;
      }

      stmt_cur_ = save_cur_;

      return flag;
}

bool dll_target::proc_disable(const NetDisable*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_DISABLE;
      stmt_cur_->u_.disable_.scope = lookup_scope_(net->target());
      return true;
}

bool dll_target::proc_force(const NetForce*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_FORCE;

      stmt_cur_->u_.cassign_.lvals = 1;
      stmt_cur_->u_.cassign_.lval = (struct ivl_lval_s*)
	    calloc(1, sizeof(struct ivl_lval_s));

      const NetNet*lsig = net->lval();
      assert(lsig);
      ivl_signal_t sig = find_signal(des_, lsig);
      assert(sig);

      ivl_lval_type_t ltype;
      switch (sig->type_) {
	  case IVL_SIT_REG:
	    ltype = IVL_LVAL_REG;
	    break;
	  case IVL_SIT_TRI:
	  case IVL_SIT_TRI0:
	  case IVL_SIT_TRI1:
	    ltype = IVL_LVAL_NET;
	    break;
	  default:
	    cerr << net->get_line() << ": internal error: Sorry, "
		 << "force to nets not supported by this target."
		 << endl;
	    return false;
      }

      stmt_cur_->u_.cassign_.lval[0].width_ = lsig->pin_count();
      stmt_cur_->u_.cassign_.lval[0].loff_  = 0;
      stmt_cur_->u_.cassign_.lval[0].type_  = ltype;
      stmt_cur_->u_.cassign_.lval[0].idx    = 0;
      stmt_cur_->u_.cassign_.lval[0].n.sig  = sig;

      stmt_cur_->u_.cassign_.npins = net->pin_count();
      stmt_cur_->u_.cassign_.pins = (ivl_nexus_t*)
	    calloc(stmt_cur_->u_.cassign_.npins, sizeof(ivl_nexus_t));

      ivl_nexus_t*ntmp = stmt_cur_->u_.cassign_.pins;
      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    ntmp[idx] = (ivl_nexus_t)net->pin(idx).nexus()->t_cookie();
	    assert(ntmp[idx]);
      }

      return true;
}

void dll_target::proc_forever(const NetForever*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_FOREVER;

      ivl_statement_t tmp = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      ivl_statement_t save_cur_ = stmt_cur_;
      stmt_cur_ = tmp;

      net->emit_recurse(this);

      save_cur_->u_.forever_.stmt_ = stmt_cur_;
      stmt_cur_ = save_cur_;
}

bool dll_target::proc_release(const NetRelease*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_RELEASE;

	/* If there is no signal attached to the release, then it is
	   the victim of an elided net. In that case, simply state
	   that there are no lvals, and that's all. */
      const NetNet*lsig = net->lval();
      if (lsig == 0) {
	    stmt_cur_->u_.cassign_.lvals = 0;
	    return true;
      }

      assert(lsig);
      stmt_cur_->u_.cassign_.lvals = 1;
      stmt_cur_->u_.cassign_.lval = (struct ivl_lval_s*)
	    calloc(1, sizeof(struct ivl_lval_s));


      ivl_signal_t sig = find_signal(des_, lsig);
      assert(sig);

      ivl_lval_type_t ltype;
      switch (sig->type_) {
	  case IVL_SIT_REG:
	    ltype = IVL_LVAL_REG;
	    break;
	  case IVL_SIT_TRI:
	  case IVL_SIT_TRI0:
	  case IVL_SIT_TRI1:
	    ltype = IVL_LVAL_NET;
	    break;
	  default:
	    cerr << net->get_line() << ": internal error: Sorry, "
		 << "force/release to nets not supported by this target."
		 << endl;
	    return false;
      }


      stmt_cur_->u_.cassign_.lval[0].width_ = lsig->pin_count();
      stmt_cur_->u_.cassign_.lval[0].loff_  = 0;
      stmt_cur_->u_.cassign_.lval[0].type_  = ltype;
      stmt_cur_->u_.cassign_.lval[0].idx    = 0;
      stmt_cur_->u_.cassign_.lval[0].n.sig  = sig;

      return true;
}

void dll_target::proc_repeat(const NetRepeat*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_REPEAT;

      assert(expr_ == 0);
      net->expr()->expr_scan(this);
      stmt_cur_->u_.while_.cond_ = expr_;
      expr_ = 0;

      ivl_statement_t tmp = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      ivl_statement_t save_cur_ = stmt_cur_;
      stmt_cur_ = tmp;

      net->emit_recurse(this);

      save_cur_->u_.while_.stmt_ = stmt_cur_;
      stmt_cur_ = save_cur_;
}

void dll_target::proc_stask(const NetSTask*net)
{
      unsigned nparms = net->nparms();
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_STASK;
	/* System task names are lex_strings strings. */
      stmt_cur_->u_.stask_.name_ = net->name();
      stmt_cur_->u_.stask_.nparm_= nparms;
      stmt_cur_->u_.stask_.parms_= (ivl_expr_t*)
	    calloc(nparms, sizeof(ivl_expr_t));

      for (unsigned idx = 0 ;  idx < nparms ;  idx += 1) {
	    if (net->parm(idx))
		  net->parm(idx)->expr_scan(this);
	    stmt_cur_->u_.stask_.parms_[idx] = expr_;
	    expr_ = 0;
      }

}

bool dll_target::proc_trigger(const NetEvTrig*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_TRIGGER;
      stmt_cur_->u_.wait_.nevent = 1;

	/* Locate the event by name. Save the ivl_event_t in the
	   statement so that the generator can find it easily. */
      const NetEvent*ev = net->event();
      ivl_scope_t ev_scope = lookup_scope_(ev->scope());

      for (unsigned idx = 0 ;  idx < ev_scope->nevent_ ;  idx += 1) {
	    const char*ename = ivl_event_basename(ev_scope->event_[idx]);
	    if (strcmp(ev->name(), ename) == 0) {
		  stmt_cur_->u_.wait_.event = ev_scope->event_[idx];
		  break;
	    }
      }


      return true;
}

void dll_target::proc_utask(const NetUTask*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_UTASK;
      stmt_cur_->u_.utask_.def = lookup_scope_(net->task());
}

bool dll_target::proc_wait(const NetEvWait*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_WAIT;
      stmt_cur_->u_.wait_.stmt_ = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      stmt_cur_->u_.wait_.nevent = net->nevents();
      if (net->nevents() > 1) {
	    stmt_cur_->u_.wait_.events = (ivl_event_t*)
		  calloc(net->nevents(), sizeof(ivl_event_t*));
      }

      for (unsigned edx = 0 ;  edx < net->nevents() ;  edx += 1) {

	      /* Locate the event by name. Save the ivl_event_t in the
		 statement so that the generator can find it easily. */
	    const NetEvent*ev = net->event(edx);
	    ivl_scope_t ev_scope = lookup_scope_(ev->scope());
	    ivl_event_t ev_tmp=0;

	    for (unsigned idx = 0 ;  idx < ev_scope->nevent_ ;  idx += 1) {
		  const char*ename = ivl_event_basename(ev_scope->event_[idx]);
		  if (strcmp(ev->name(), ename) == 0) {
			ev_tmp = ev_scope->event_[idx];
			break;
		  }
	    }
	    // XXX should we assert(ev_tmp)?

	    if (net->nevents() == 1)
		  stmt_cur_->u_.wait_.event = ev_tmp;
	    else
		  stmt_cur_->u_.wait_.events[edx] = ev_tmp;

	      /* If this is an event with a probe, then connect up the
		 pins. This wasn't done during the ::event method because
		 the signals weren't scanned yet. */

	    if (ev->nprobe() >= 1) {
		  unsigned iany = 0;
		  unsigned ineg = ev_tmp->nany;
		  unsigned ipos = ineg + ev_tmp->nneg;

		  for (unsigned idx = 0 ;  idx < ev->nprobe() ;  idx += 1) {
			const NetEvProbe*pr = ev->probe(idx);
			unsigned base = 0;

			switch (pr->edge()) {
			    case NetEvProbe::ANYEDGE:
			      base = iany;
			      iany += pr->pin_count();
			      break;
			    case NetEvProbe::NEGEDGE:
			      base = ineg;
			      ineg += pr->pin_count();
			      break;
			    case NetEvProbe::POSEDGE:
			      base = ipos;
			      ipos += pr->pin_count();
			      break;
			}

			for (unsigned bit = 0
				   ; bit < pr->pin_count()
				   ; bit += 1) {
			      ivl_nexus_t nex = (ivl_nexus_t)
				    pr->pin(bit).nexus()->t_cookie();
			      assert(nex);
			      ev_tmp->pins[base+bit] = nex;
			}
		  }
	    }
      }

	/* The ivl_statement_t for the wait statement is not complete
	   until we calculate the sub-statement. */

      ivl_statement_t save_cur_ = stmt_cur_;
      stmt_cur_ = stmt_cur_->u_.wait_.stmt_;
      bool flag = net->emit_recurse(this);
      if (flag && (stmt_cur_->type_ == IVL_ST_NONE))
	    stmt_cur_->type_ = IVL_ST_NOOP;

      stmt_cur_ = save_cur_;

      return flag;
}

void dll_target::proc_while(const NetWhile*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_WHILE;
      stmt_cur_->u_.while_.stmt_ = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      assert(expr_ == 0);
      net->expr()->expr_scan(this);
      stmt_cur_->u_.while_.cond_ = expr_;
      expr_ = 0;

	/* Now generate the statement of the while loop. We know it is
	   a single statement, and we know that the
	   emit_proc_recurse() will call emit_proc() for it. */

      ivl_statement_t save_cur_ = stmt_cur_;
      stmt_cur_ = save_cur_->u_.while_.stmt_;
      net->emit_proc_recurse(this);
      stmt_cur_ = save_cur_;
}

/*
 * $Log: t-dll-proc.cc,v $
 * Revision 1.65  2004/10/04 01:10:55  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.64  2004/05/31 23:34:39  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.63  2004/05/19 03:18:40  steve
 *  Add ivl_target support for non-blocking assign of real.
 *
 * Revision 1.62  2003/12/19 01:27:10  steve
 *  Fix various unsigned compare warnings.
 *
 * Revision 1.61  2003/12/03 02:46:24  steve
 *  Add support for wait on list of named events.
 *
 * Revision 1.60  2003/06/24 01:38:03  steve
 *  Various warnings fixed.
 *
 * Revision 1.59  2003/05/14 05:26:41  steve
 *  Support real expressions in case statements.
 *
 * Revision 1.58  2003/05/07 19:56:20  steve
 *  Improve internal error message.
 *
 * Revision 1.57  2003/03/01 06:25:30  steve
 *  Add the lex_strings string handler, and put
 *  scope names and system task/function names
 *  into this table. Also, permallocate event
 *  names from the beginning.
 *
 * Revision 1.56  2003/01/30 16:23:08  steve
 *  Spelling fixes.
 *
 * Revision 1.55  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.54  2002/08/19 00:06:12  steve
 *  Allow release to handle removal of target net.
 *
 * Revision 1.53  2002/08/13 05:35:00  steve
 *  Do not elide named blocks.
 *
 * Revision 1.52  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.51  2002/08/07 00:54:39  steve
 *  Add force to nets.
 *
 * Revision 1.50  2002/08/04 18:28:15  steve
 *  Do not use hierarchical names of memories to
 *  generate vvp labels. -tdll target does not
 *  used hierarchical name string to look up the
 *  memory objects in the design.
 *
 * Revision 1.49  2002/06/16 20:39:12  steve
 *  Normalize run-time index expressions for bit selects
 *
 * Revision 1.48  2002/06/16 19:19:16  steve
 *  Generate runtime code to normalize indices.
 *
 * Revision 1.47  2002/06/05 03:44:25  steve
 *  Add support for memory words in l-value of
 *  non-blocking assignments, and remove the special
 *  NetAssignMem_ and NetAssignMemNB classes.
 *
 * Revision 1.46  2002/06/04 05:38:44  steve
 *  Add support for memory words in l-value of
 *  blocking assignments, and remove the special
 *  NetAssignMem class.
 *
 * Revision 1.45  2002/05/29 22:05:55  steve
 *  Offset lvalue index expressions.
 *
 * Revision 1.44  2002/05/27 00:08:45  steve
 *  Support carrying the scope of named begin-end
 *  blocks down to the code generator, and have
 *  the vvp code generator use that to support disable.
 *
 * Revision 1.43  2002/05/26 01:39:03  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 *
 * Revision 1.42  2002/04/21 22:31:02  steve
 *  Redo handling of assignment internal delays.
 *  Leave it possible for them to be calculated
 *  at run time.
 *
 * Revision 1.41  2002/01/19 19:02:08  steve
 *  Pass back target errors processing conditionals.
 *
 * Revision 1.40  2001/11/14 03:28:49  steve
 *  DLL target support for force and release.
 *
 * Revision 1.39  2001/11/01 04:25:31  steve
 *  ivl_target support for cassign.
 *
 * Revision 1.38  2001/10/31 05:24:52  steve
 *  ivl_target support for assign/deassign.
 *
 * Revision 1.37  2001/10/30 02:52:07  steve
 *  Stubs for assign/deassign for t-dll.
 *
 * Revision 1.36  2001/10/19 21:53:24  steve
 *  Support multiple root modules (Philip Blundell)
 *
 * Revision 1.35  2001/09/15 18:27:04  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.34  2001/08/25 23:50:03  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.33  2001/07/27 02:41:56  steve
 *  Fix binding of dangling function ports. do not elide them.
 *
 * Revision 1.32  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.31  2001/07/19 04:55:06  steve
 *  Support calculated delays in vvp.tgt.
 *
 * Revision 1.30  2001/06/21 23:23:14  steve
 *  Initialize stmt_cur_ substatements during dll case building.
 *
 * Revision 1.29  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 * Revision 1.28  2001/04/15 03:19:44  steve
 *  Oops, excessive test assert neets to be removed.
 *
 * Revision 1.27  2001/04/15 03:14:31  steve
 *  Handle noop as case statements.
 *
 * Revision 1.26  2001/04/15 02:58:11  steve
 *  vvp support for <= with internal delay.
 *
 * Revision 1.25  2001/04/07 19:26:32  steve
 *  Add the disable statemnent.
 *
 * Revision 1.24  2001/04/06 02:28:02  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.23  2001/04/05 03:20:57  steve
 *  Generate vvp code for the repeat statement.
 *
 * Revision 1.22  2001/04/04 04:50:35  steve
 *  Support forever loops in the tgt-vvp target.
 *
 * Revision 1.21  2001/04/03 04:50:37  steve
 *  Support non-blocking assignments.
 *
 * Revision 1.20  2001/04/02 02:28:12  steve
 *  Generate code for task calls.
 *
 * Revision 1.19  2001/04/01 06:52:28  steve
 *  support the NetWhile statement.
 *
 * Revision 1.18  2001/04/01 01:48:21  steve
 *  Redesign event information to support arbitrary edge combining.
 *
 * Revision 1.17  2001/03/31 17:36:39  steve
 *  Generate vvp code for case statements.
 *
 * Revision 1.16  2001/03/30 23:24:02  steve
 *  Make empty event sub-expression a noop.
 *
 * Revision 1.15  2001/03/30 05:49:52  steve
 *  Generate code for fork/join statements.
 *
 * Revision 1.14  2001/03/29 03:47:38  steve
 *  Behavioral trigger statements.
 *
 * Revision 1.13  2001/03/28 06:07:39  steve
 *  Add the ivl_event_t to ivl_target, and use that to generate
 *  .event statements in vvp way ahead of the thread that uses it.
 *
 * Revision 1.12  2001/03/27 06:27:40  steve
 *  Generate code for simple @ statements.
 *
 * Revision 1.11  2001/03/20 01:44:14  steve
 *  Put processes in the proper scope.
 *
 * Revision 1.10  2000/10/18 20:04:39  steve
 *  Add ivl_lval_t and support for assignment l-values.
 *
 * Revision 1.9  2000/10/08 04:01:54  steve
 *  Back pointers in the nexus objects into the devices
 *  that point to it.
 *
 *  Collect threads into a list in the design.
 *
 * Revision 1.8  2000/10/06 23:46:50  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.7  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 *
 * Revision 1.6  2000/09/30 02:18:15  steve
 *  ivl_expr_t support for binary operators,
 *  Create a proper ivl_scope_t object.
 *
 * Revision 1.5  2000/09/26 00:30:07  steve
 *  Add EX_NUMBER and ST_TRIGGER to dll-api.
 *
 * Revision 1.4  2000/09/23 05:15:07  steve
 *  Add enough tgt-verilog code to support hello world.
 *
 * Revision 1.3  2000/09/22 03:58:30  steve
 *  Access to the name of a system task call.
 *
 * Revision 1.2  2000/09/19 04:15:27  steve
 *  Introduce the means to get statement types.
 *
 * Revision 1.1  2000/09/18 01:24:32  steve
 *  Get the structure for ivl_statement_t worked out.
 *
 */

