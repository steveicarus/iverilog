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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: t-dll-proc.cc,v 1.12 2001/03/27 06:27:40 steve Exp $"
#endif

# include  "target.h"
# include  "ivl_target.h"
# include  "compiler.h"
# include  "t-dll.h"
# include  <malloc.h>


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

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
	    struct ivl_lval_s*cur = stmt_cur_->u_.assign_.lval_ + idx;
	    const NetAssign_*asn = net->l_val(idx);

	    cur->width_ = asn->pin_count();

	    if (cur->width_ > 1) {
		  cur->n.pins_ = new ivl_nexus_t[cur->width_];
		  for (unsigned pp = 0 ;  pp < cur->width_ ;  pp += 1) {
			const Nexus*nex = asn->pin(pp).nexus();
			assert(nex->t_cookie());
			cur->n.pins_[pp] = (ivl_nexus_t)nex->t_cookie();
		  }

	    } else {
		  const Nexus*nex = asn->pin(0).nexus();
		  assert(nex->t_cookie());
		  cur->n.pin_ = (ivl_nexus_t)nex->t_cookie();
	    }

	    cur->mux = 0;
	    if (asn->bmux()) {
		  assert(expr_ == 0);
		  asn->bmux()->expr_scan(this);
		  cur->mux = expr_;
		  expr_ = 0;
	    }
      }

      assert(expr_ == 0);
      net->rval()->expr_scan(this);
      stmt_cur_->u_.assign_.rval_ = expr_;
      expr_ = 0;
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
      if (count == 1) {
	    return net->proc_first()->emit_proc(this);
      }


	/* Handle the general case. The block has some statements in
	   it, so fill in the block fields of the existing statement,
	   and generate the contents for the statement array. */

      stmt_cur_->type_ = IVL_ST_BLOCK;
      stmt_cur_->u_.block_.nstmt_ = count;
      stmt_cur_->u_.block_.stmt_ = (struct ivl_statement_s*)
	    calloc(count, sizeof(struct ivl_statement_s));

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

void dll_target::proc_condit(const NetCondit*net)
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
      net->emit_recurse_if(this);

      stmt_cur_ = save_cur_->u_.condit_.stmt_+1;
      net->emit_recurse_else(this);

      stmt_cur_ = save_cur_;
}

bool dll_target::proc_delay(const NetPDelay*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      ivl_statement_t tmp = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      if (const NetExpr*expr = net->expr()) {

	    stmt_cur_->type_ = IVL_ST_DELAYX;
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

void dll_target::proc_stask(const NetSTask*net)
{
      unsigned nparms = net->nparms();
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_STASK;
      stmt_cur_->u_.stask_.name_ = strdup(net->name());
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
      return true;
}

bool dll_target::proc_wait(const NetEvWait*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_WAIT;
      stmt_cur_->u_.wait_.stmt_ = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

      if (net->nevents() != 1) {
	    cerr << "internal error: multiple events not supported." << endl;
	    return false;
      }

      NetEvent*ev = net->event(0);
      assert(ev->nprobe() == 1);

      const NetEvProbe*pr = ev->probe(0);
      assert(pr->pin_count() == 1);

      switch (pr->edge()) {
	  case NetEvProbe::ANYEDGE:
	    stmt_cur_->u_.wait_.edge_ = IVL_EDGE_ANY;
	    break;
	  case NetEvProbe::NEGEDGE:
	    stmt_cur_->u_.wait_.edge_ = IVL_EDGE_NEG;
	    break;
	  case NetEvProbe::POSEDGE:
	    stmt_cur_->u_.wait_.edge_ = IVL_EDGE_POS;
	    break;
      }

      const Nexus*nex = pr->pin(0).nexus();
      assert(nex);
      assert(nex->t_cookie());
      stmt_cur_->u_.wait_.cond_ = (ivl_nexus_t) nex->t_cookie();

      ivl_statement_t save_cur_ = stmt_cur_;
      stmt_cur_ = stmt_cur_->u_.wait_.stmt_;
      bool flag = net->emit_recurse(this);
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

	/* XXXX Nothing about the expression? */

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

