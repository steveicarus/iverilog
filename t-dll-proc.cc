/*
 * Copyright (c) 2000-2022 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  <iostream>

# include  <cstring>
# include  "target.h"
# include  "ivl_target.h"
# include  "compiler.h"
# include  "t-dll.h"
# include  "netclass.h"
# include  <cstdlib>
# include  "ivl_alloc.h"
# include  "ivl_assert.h"

bool dll_target::process(const NetProcTop*net)
{
      bool rc_flag = true;

      ivl_process_t obj = (struct ivl_process_s*)
	    calloc(1, sizeof(struct ivl_process_s));

      obj->type_ = net->type();
      obj->analog_flag = 0;

      FILE_NAME(obj, net);

	/* Save the scope of the process. */
      obj->scope_ = lookup_scope_(net->scope());

      obj->nattr = net->attr_cnt();
      obj->attr = fill_in_attributes(net);

	/* This little bit causes the process to be completely
	   generated so that it can be passed to the DLL. The
	   stmt_cur_ member is used to hold a pointer to the current
	   statement in progress, and the emit_proc() method fills in
	   that object.

	   We know a few things about the current statement: we are
	   not in the middle of one, and when we are done, we have our
	   statement back. The asserts check these conditions. */

      assert(stmt_cur_ == 0);
      stmt_cur_ = (struct ivl_statement_s*)calloc(1, sizeof*stmt_cur_);
      rc_flag = net->statement()->emit_proc(this) && rc_flag;

      assert(stmt_cur_);
      obj->stmt_ = stmt_cur_;
      stmt_cur_ = 0;

	/* Save the process in the design. */
      obj->next_ = des_.threads_;
      des_.threads_ = obj;

      return rc_flag;
}

void dll_target::task_def(const NetScope*net)
{
      ivl_scope_t scop = lookup_scope_(net);
      const NetTaskDef*def = net->task_def();

      assert(def);
      assert(def->proc());
      assert(stmt_cur_ == 0);
      stmt_cur_ = (struct ivl_statement_s*)calloc(1, sizeof*stmt_cur_);
      def->proc()->emit_proc(this);

      assert(stmt_cur_);
      scop->def = stmt_cur_;
      stmt_cur_ = 0;

      scop->ports = def->port_count();
      if (scop->ports > 0) {
	    scop->u_.port = new ivl_signal_t[scop->ports];
	    for (unsigned idx = 0 ;  idx < scop->ports ;  idx += 1)
		  scop->u_.port[idx] = find_signal(des_, def->port(idx));
      }

}

bool dll_target::func_def(const NetScope*net)
{
      ivl_scope_t scop = lookup_scope_(net);
      const NetFuncDef*def = net->func_def();

      assert(def);
      assert(def->proc());
      assert(stmt_cur_ == 0);
      stmt_cur_ = (struct ivl_statement_s*)calloc(1, sizeof*stmt_cur_);
      def->proc()->emit_proc(this);

      assert(stmt_cur_);
      scop->def = stmt_cur_;
      stmt_cur_ = 0;

      scop->ports = def->port_count() + 1;
      if (scop->ports > 0) {
	    scop->u_.port = new ivl_signal_t[scop->ports];
	    for (unsigned idx = 1 ;  idx < scop->ports ;  idx += 1)
		  scop->u_.port[idx] = find_signal(des_, def->port(idx-1));
      }

	/* FIXME: the ivl_target API expects port-0 to be the output
	   port. This assumes that the return value is a signal, which
	   is *not* correct. Someday, I'm going to have to change
	   this, but that will break code generators that use this
	   result. */
      if (const NetNet*ret_sig = def->return_sig())
	    scop->u_.port[0] = find_signal(des_, ret_sig);
      else
	    scop->u_.port[0] = 0;

	/* If there is no return value, then this is a void function. */

      return true;

}

/*
 * This private function makes the assignment lvals for the various
 * kinds of assignment statements.
 */
bool dll_target::make_assign_lvals_(const NetAssignBase*net)
{
      bool flag = true;
      assert(stmt_cur_);

      unsigned cnt = net->l_val_count();

      stmt_cur_->u_.assign_.lvals_ = cnt;
      stmt_cur_->u_.assign_.lval_  = new struct ivl_lval_s[cnt];
      stmt_cur_->u_.assign_.delay  = 0;

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
	    struct ivl_lval_s*cur = stmt_cur_->u_.assign_.lval_ + idx;
	    const NetAssign_*asn = net->l_val(idx);
	    flag &= make_single_lval_(net, cur, asn);
      }

      return flag;
}

bool dll_target::make_single_lval_(const LineInfo*li, struct ivl_lval_s*cur, const NetAssign_*asn)
{
      bool flag = true;

      const NetExpr*loff = asn->get_base();

      if (loff == 0) {
	    cur->loff = 0;
	    cur->sel_type = IVL_SEL_OTHER;
      } else {
	    loff->expr_scan(this);
	    cur->loff = expr_;
	    cur->sel_type = asn->select_type();
	    expr_ = 0;
      }

      cur->width_ = asn->lwidth();

      if (asn->sig()) {
	    cur->type_ = IVL_LVAL_REG;
	    cur->n.sig = find_signal(des_, asn->sig());

      } else {
	    const NetAssign_*asn_nest = asn->nest();
	    ivl_assert(*li, asn_nest);
	    struct ivl_lval_s*cur_nest = new struct ivl_lval_s;
	    make_single_lval_(li, cur_nest, asn_nest);

	    cur->type_ = IVL_LVAL_LVAL;
	    cur->n.nest = cur_nest;
      }

      cur->idx = 0;
	// If there is a word select expression, it is
	// really an array index. Note that the word index
	// expression is already converted to canonical
	// form by elaboration.
      if (asn->word()) {
	    assert(expr_ == 0);
	    asn->word()->expr_scan(this);
	    cur->type_ = IVL_LVAL_ARR;
	    cur->idx = expr_;
	    expr_ = 0;
      }

      cur->property_idx = asn->get_property_idx();

      return flag;
}

void dll_target::proc_alloc(const NetAlloc*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_ALLOC;
      stmt_cur_->u_.alloc_.scope = lookup_scope_(net->scope());
}

/*
 */
bool dll_target::proc_assign(const NetAssign*net)
{
      bool flag = true;

      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_ASSIGN;
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->u_.assign_.delay = 0;

	/* Make the lval fields. */
      flag &= make_assign_lvals_(net);

      stmt_cur_->u_.assign_.oper = net->assign_operator();
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

      return flag;
}


void dll_target::proc_assign_nb(const NetAssignNB*net)
{
      const NetExpr* delay_exp = net->get_delay();
      const NetExpr* cnt_exp = net->get_count();
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_ASSIGN_NB;
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->u_.assign_.delay  = 0;
      stmt_cur_->u_.assign_.count  = 0;
      stmt_cur_->u_.assign_.nevent  = 0;

	/* Make the lval fields. */
      make_assign_lvals_(net);

	/* Make the rval field. */
      assert(expr_ == 0);
      net->rval()->expr_scan(this);
      stmt_cur_->u_.assign_.rval_ = expr_;
      expr_ = 0;

	/* Process a delay if it exists. */
      if (const NetEConst*delay_num = dynamic_cast<const NetEConst*>(delay_exp)) {
	    verinum val = delay_num->value();
	    ivl_expr_t de = new struct ivl_expr_s;
	    de->type_ = IVL_EX_DELAY;
	    de->width_  = 8 * sizeof(uint64_t);
	    de->signed_ = 0;
	    de->u_.delay_.value = val.as_ulong64();
	    stmt_cur_->u_.assign_.delay = de;

      } else if (delay_exp != 0) {
	    delay_exp->expr_scan(this);
	    stmt_cur_->u_.assign_.delay = expr_;
	    expr_ = 0;
      }

	/* Process a count if it exists. */
      if (const NetEConst*cnt_num = dynamic_cast<const NetEConst*>(cnt_exp)) {
	    verinum val = cnt_num->value();
	    ivl_expr_t cnt = new struct ivl_expr_s;
	    cnt->type_ = IVL_EX_ULONG;
	    cnt->width_  = 8 * sizeof(unsigned long);
	    cnt->signed_ = 0;
	    cnt->u_.ulong_.value = val.as_ulong();
	    stmt_cur_->u_.assign_.count = cnt;

      } else if (cnt_exp != 0) {
	    cnt_exp->expr_scan(this);
	    stmt_cur_->u_.assign_.count = expr_;
	    expr_ = 0;
      }

	/* Process the events if they exist. This is a copy of code
	 * from NetEvWait below. */
      if (net->nevents() > 0) {
	    stmt_cur_->u_.assign_.nevent = net->nevents();
	    if (net->nevents() > 1) {
		  stmt_cur_->u_.assign_.events = (ivl_event_t*)
		        calloc(net->nevents(), sizeof(ivl_event_t*));
	    }

	    for (unsigned edx = 0 ;  edx < net->nevents() ;  edx += 1) {

		    /* Locate the event by name. Save the ivl_event_t in the
		       statement so that the generator can find it easily. */
		  const NetEvent*ev = net->event(edx);
		  ivl_scope_t ev_scope = lookup_scope_(ev->scope());
		  ivl_event_t ev_tmp=0;

		  assert(ev_scope);
		  assert(ev_scope->nevent_ > 0);
		  for (unsigned idx = 0;  idx < ev_scope->nevent_; idx += 1) {
			const char*ename =
			      ivl_event_basename(ev_scope->event_[idx]);
			if (strcmp(ev->name(), ename) == 0) {
			      ev_tmp = ev_scope->event_[idx];
			      break;
			}
		  }
		  // XXX should we assert(ev_tmp)?

		  if (net->nevents() == 1)
			stmt_cur_->u_.assign_.event = ev_tmp;
		  else
			stmt_cur_->u_.assign_.events[edx] = ev_tmp;

		    /* If this is an event with a probe, then connect up the
		       pins. This wasn't done during the ::event method because
		       the signals weren't scanned yet. */

		  if (ev->nprobe() >= 1) {
			unsigned iany = 0;
			unsigned ineg = ev_tmp->nany;
			unsigned ipos = ineg + ev_tmp->nneg;
			unsigned iedg = ipos + ev_tmp->npos;

			for (unsigned idx = 0;  idx < ev->nprobe();  idx += 1) {
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
				  case NetEvProbe::EDGE:
				    base = iedg;
				    iedg += pr->pin_count();
				    break;
			      }

			      for (unsigned bit = 0; bit < pr->pin_count();
				   bit += 1) {
				    ivl_nexus_t nex = (ivl_nexus_t)
				          pr->pin(bit).nexus()->t_cookie();
				    assert(nex);
				    ev_tmp->pins[base+bit] = nex;
			      }
			}
		  }
	    }
      }
}

bool dll_target::proc_block(const NetBlock*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

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

	/* If there is exactly one statement and the block is not a
	   fork/join_none, there is no need for the block wrapper,
	   generate the contained statement instead. */
      if ((count == 1) && (net->subscope() == 0) &&
	  (net->type() != NetBlock::PARA_JOIN_NONE)) {
	    return net->proc_first()->emit_proc(this);
      }


	/* Handle the general case. The block has some statements in
	   it, so fill in the block fields of the existing statement,
	   and generate the contents for the statement array. */

      switch (net->type()) {
	  case NetBlock::SEQU:
	    stmt_cur_->type_ = IVL_ST_BLOCK;
	    break;
	  case NetBlock::PARA:
	    stmt_cur_->type_ = IVL_ST_FORK;
	    break;
	  case NetBlock::PARA_JOIN_ANY:
	    stmt_cur_->type_ = IVL_ST_FORK_JOIN_ANY;
	    break;
	  case NetBlock::PARA_JOIN_NONE:
	    stmt_cur_->type_ = IVL_ST_FORK_JOIN_NONE;
	    break;
      }
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

bool dll_target::proc_break(const NetBreak*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);
      stmt_cur_->type_ = IVL_ST_BREAK;
      return true;
}

/*
 * A. case statement is in turn an array of statements with gate
 * expressions. This builds arrays of the right size and builds the
 * ivl_expr_t and ivl_statement_s arrays for the substatements.
 */
void dll_target::proc_case(const NetCase*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

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

      stmt_cur_->u_.case_.quality = net->case_quality();
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
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_CASSIGN;

	/* Make the l-value fields. */
      make_assign_lvals_(net);

      assert(expr_ == 0);
      net->rval()->expr_scan(this);
      stmt_cur_->u_.assign_.rval_ = expr_;
      expr_ = 0;

      return true;
}

bool dll_target::proc_condit(const NetCondit*net)
{
      bool rc_flag = true;

      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_CONDIT;
      stmt_cur_->u_.condit_.stmt_ = (struct ivl_statement_s*)
	    calloc(2, sizeof(struct ivl_statement_s));

      assert(expr_ == 0);
      net->expr()->expr_scan(this);
      stmt_cur_->u_.condit_.cond_ = expr_;
      if (expr_ == 0)
	    rc_flag = false;
      expr_ = 0;

      ivl_statement_t save_cur_ = stmt_cur_;

      stmt_cur_ = save_cur_->u_.condit_.stmt_+0;
      rc_flag = net->emit_recurse_if(this) && rc_flag;

      stmt_cur_ = save_cur_->u_.condit_.stmt_+1;
      rc_flag = net->emit_recurse_else(this) && rc_flag;

      stmt_cur_ = save_cur_;
      return rc_flag;
}

bool dll_target::proc_continue(const NetContinue*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);
      stmt_cur_->type_ = IVL_ST_CONTINUE;
      return true;
}

bool dll_target::proc_deassign(const NetDeassign*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_DEASSIGN;

	/* Make the l-value fields. */
      make_assign_lvals_(net);

      return true;
}

bool dll_target::proc_delay(const NetPDelay*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

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
	    stmt_cur_->u_.delay_.value = net->delay();
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
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_DISABLE;
      stmt_cur_->u_.disable_.flow_control = net->flow_control();
      const NetScope* dis_scope = net->target();
	/* A normal disable. */
      if (dis_scope) stmt_cur_->u_.disable_.scope = lookup_scope_(dis_scope);
	/* A SystemVerilog disable fork. */
      else stmt_cur_->u_.disable_.scope = 0;
      return true;
}

void dll_target::proc_do_while(const NetDoWhile*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_DO_WHILE;
      stmt_cur_->u_.while_.stmt_ = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      assert(expr_ == 0);
      net->expr()->expr_scan(this);
      stmt_cur_->u_.while_.cond_ = expr_;
      expr_ = 0;

	/* Now generate the statement of the do/while loop. We know it is
	   a single statement, and we know that the
	   emit_proc_recurse() will call emit_proc() for it. */

      ivl_statement_t save_cur_ = stmt_cur_;
      stmt_cur_ = save_cur_->u_.while_.stmt_;
      net->emit_proc_recurse(this);
      stmt_cur_ = save_cur_;
}

bool dll_target::proc_force(const NetForce*net)
{

      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_FORCE;

	/* Make the l-value fields. */
      make_assign_lvals_(net);

      assert(expr_ == 0);
      net->rval()->expr_scan(this);
      stmt_cur_->u_.assign_.rval_ = expr_;
      expr_ = 0;

      return true;
}

void dll_target::proc_forever(const NetForever*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_FOREVER;

      ivl_statement_t tmp = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      ivl_statement_t save_cur_ = stmt_cur_;
      stmt_cur_ = tmp;

      net->emit_recurse(this);

      save_cur_->u_.forever_.stmt_ = stmt_cur_;
      stmt_cur_ = save_cur_;
}

bool dll_target::proc_forloop(const NetForLoop*net)
{
      ivl_statement_t tmp;
      bool rc, res=true;

      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);
      stmt_cur_->type_ = IVL_ST_FORLOOP;

      ivl_statement_t save_cur_ = stmt_cur_;

      // Note that the init statement is optional. If it is not present,
      // then the emit_recurse_init will not generate a statement.
      tmp = (struct ivl_statement_s*)calloc(1, sizeof(struct ivl_statement_s));
      stmt_cur_ = tmp;
      rc = net->emit_recurse_init(this);
      if (stmt_cur_->type_ != IVL_ST_NONE)
	    save_cur_->u_.forloop_.init_stmt = stmt_cur_;
      else {
	    free(tmp);
	    save_cur_->u_.forloop_.init_stmt = nullptr;
      }
      res = res && rc;

      tmp = (struct ivl_statement_s*)calloc(1, sizeof(struct ivl_statement_s));
      stmt_cur_ = tmp;
      rc = net->emit_recurse_stmt(this);
      save_cur_->u_.forloop_.stmt = stmt_cur_;
      res = res && rc;

      tmp = (struct ivl_statement_s*)calloc(1, sizeof(struct ivl_statement_s));
      stmt_cur_ = tmp;
      rc = net->emit_recurse_step(this);
      if (stmt_cur_->type_ != IVL_ST_NONE)
	    save_cur_->u_.forloop_.step = stmt_cur_;
      else {
	    free(tmp);
	    save_cur_->u_.forloop_.step = nullptr;
      }
      res = res && rc;

      assert(expr_ == nullptr);
      rc = net->emit_recurse_condition(this);
      save_cur_->u_.forloop_.condition = expr_;
      expr_ = nullptr;
      res = res && rc;

      stmt_cur_ = save_cur_;
      return res;
}

void dll_target::proc_free(const NetFree*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_FREE;
      stmt_cur_->u_.free_.scope = lookup_scope_(net->scope());
}

bool dll_target::proc_release(const NetRelease*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_RELEASE;

	/* Make the l-value fields. */
      make_assign_lvals_(net);

      return true;
}

void dll_target::proc_repeat(const NetRepeat*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

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
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_STASK;
	/* System task names are lex_strings strings. */
      stmt_cur_->u_.stask_.name_ = net->name();
      stmt_cur_->u_.stask_.sfunc_as_task_ = net->sfunc_as_task();
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
      FILE_NAME(stmt_cur_, net);

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

bool dll_target::proc_nb_trigger(const NetEvNBTrig*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_NB_TRIGGER;
      stmt_cur_->u_.wait_.nevent = 1;
      stmt_cur_->u_.wait_.delay = 0;

      if (const NetExpr*expr = net->delay()) {
	    assert(expr_ == 0);
	    expr->expr_scan(this);
	    stmt_cur_->u_.wait_.delay = expr_;
	    expr_ = 0;
      }

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
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_UTASK;
      stmt_cur_->u_.utask_.def = lookup_scope_(net->task());
}

bool dll_target::proc_wait(const NetEvWait*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_WAIT;
      stmt_cur_->u_.wait_.stmt_ = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      stmt_cur_->u_.wait_.nevent = net->nevents();

	/* This is a wait fork statement. */
      if ((net->nevents() == 1) && (net->event(0) == 0)) {
	    stmt_cur_->u_.wait_.needs_t0_trigger = 0;
	    stmt_cur_->u_.wait_.event = 0;
	    stmt_cur_->type_ = IVL_ST_WAIT;
	    stmt_cur_->u_.wait_.stmt_->type_ = IVL_ST_NOOP;
	    return true;
      }

      stmt_cur_->u_.wait_.needs_t0_trigger = net->has_t0_trigger();

	// This event processing code is also in the NB assign above.
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

	    assert(ev_scope);
	    assert(ev_scope->nevent_ > 0);
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
		  unsigned iedg = ipos + ev_tmp->npos;

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
			    case NetEvProbe::EDGE:
			      base = iedg;
			      iedg += pr->pin_count();
			      break;
			}

			for (unsigned bit = 0; bit < pr->pin_count(); bit += 1) {
			      ivl_nexus_t nex = (ivl_nexus_t)
				    pr->pin(bit).nexus()->t_cookie();
			      ivl_assert(*ev, nex);
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
      FILE_NAME(stmt_cur_, net);

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
