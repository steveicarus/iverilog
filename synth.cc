/*
 * Copyright (c) 1999-2017 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  "functor.h"
# include  "netlist.h"

/*
 * This functor scans the behavioral code, looking for expressions to
 * synthesize. Although it uses the proc_match_t class, it doesn't
 * actually match anything, but transforms expressions into structural
 * netlists. The product of this should be a process where all the
 * expressions have been reduced to a signal ident, which references
 * the NetNet of the now synthesized expression.
 */
class do_expr  : public proc_match_t {

    public:
      do_expr(Design*d, NetScope*s)
      : des_(d), scope_(s) { }

    private:

      Design*des_;
      NetScope*scope_;

      virtual int assign(NetAssign*);
      virtual int assign_nb(NetAssignNB*);
      virtual int event_wait(NetEvWait*);
      virtual int condit(NetCondit*);
};


int do_expr::assign(NetAssign*stmt)
{
      if (dynamic_cast<NetESignal*>(stmt->rval()))
	    return 0;

      NetNet*tmp = stmt->rval()->synthesize(des_, scope_, stmt->rval());
      if (tmp == 0)
	    return 0;

      NetESignal*tmpe = new NetESignal(tmp);
      stmt->set_rval(tmpe);

      return 0;
}

int do_expr::assign_nb(NetAssignNB*stmt)
{
      if (dynamic_cast<NetESignal*>(stmt->rval()))
	    return 0;

      NetNet*tmp = stmt->rval()->synthesize(des_, scope_, stmt->rval());
      if (tmp == 0)
	    return 0;

      NetESignal*tmpe = new NetESignal(tmp);
      stmt->set_rval(tmpe);

      return 0;
}

int do_expr::condit(NetCondit*stmt)
{
	/* synthesize the condition expression, if necessary. */
      if (! dynamic_cast<NetESignal*>(stmt->expr())) {
	    NetNet*tmp = stmt->expr()->synthesize(des_, scope_, stmt->expr());

	    if (tmp) {
		  NetESignal*tmpe = new NetESignal(tmp);
		  stmt->set_expr(tmpe);
	    }

      }

	/* Now recurse through the if and else clauses. */
      if (NetProc*tmp = stmt->if_clause())
	    tmp->match_proc(this);

      if (NetProc*tmp = stmt->else_clause())
	    tmp->match_proc(this);

      return 0;
}

int do_expr::event_wait(NetEvWait*stmt)
{
      NetProc*tmp = stmt->statement();
      if (tmp)
	    return tmp->match_proc(this);
      else
	    return 0;
}

class synth_f  : public functor_t {

    public:
      synth_f() { top_ = NULL; }
      void process(Design*, NetProcTop*);

    private:
      void proc_always_(Design*);
      void proc_initial_(Design*);
      void proc_final_(Design*);

      NetProcTop*top_;
};


/*
 * Look at a process, and divide the problem into always and initial
 * threads.
 */
void synth_f::process(Design*des, NetProcTop*top)
{
      top_ = top;
      switch (top->type()) {
	  case IVL_PR_ALWAYS:
	  case IVL_PR_ALWAYS_COMB:
	  case IVL_PR_ALWAYS_FF:
	  case IVL_PR_ALWAYS_LATCH:
	    proc_always_(des);
	    break;
	  case IVL_PR_INITIAL:
	    proc_initial_(des);
	    break;
	  case IVL_PR_FINAL:
	    proc_final_(des);
	    break;
      }
}

void synth_f::proc_always_(Design*des)
{
      do_expr expr_pat(des, top_->scope());
      top_->statement()->match_proc(&expr_pat);
}

void synth_f::proc_initial_(Design*des)
{
      do_expr expr_pat(des, top_->scope());
      top_->statement()->match_proc(&expr_pat);
}

void synth_f::proc_final_(Design*des)
{
      do_expr expr_pat(des, top_->scope());
      top_->statement()->match_proc(&expr_pat);
}

void synth(Design*des)
{
      synth_f synth_obj;
      des->functor(&synth_obj);
}
