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
#ident "$Id: t-dll-proc.cc,v 1.4 2000/09/23 05:15:07 steve Exp $"
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

      if (process_) {
	    int rc = (process_)(obj);
	    return rc == 0;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_process function." << endl;
	    return false;
      }

      return false;
}

/*XXXX
 * Currently, this doesn't do anything really, so stub it out.
 */
void dll_target::proc_assign(const NetAssign*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_ASSIGN;
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
	    expr_ = 0;
	    net->parm(idx)->expr_scan(this);
	    stmt_cur_->u_.stask_.parms_[idx] = expr_;
      }

}

bool dll_target::proc_wait(const NetEvWait*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);

      stmt_cur_->type_ = IVL_ST_WAIT;
      stmt_cur_->u_.wait_.stmt_ = (struct ivl_statement_s*)
	    calloc(1, sizeof(struct ivl_statement_s));

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

