/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elab_expr.cc,v 1.19 2000/03/20 16:57:22 steve Exp $"
#endif


# include  "pform.h"
# include  "netlist.h"

NetExpr* PExpr::elaborate_expr(Design*des, NetScope*) const
{
      cerr << get_line() << ": I do not know how to elaborate expression: "
	   << *this << endl;
      return 0;
}

/*
 * Elaborate binary expressions. This involves elaborating the left
 * and right sides, and creating one of a variety of different NetExpr
 * types. 
 */
NetEBinary* PEBinary::elaborate_expr(Design*des, NetScope*scope) const
{
      NetExpr*lp = left_->elaborate_expr(des, scope);
      NetExpr*rp = right_->elaborate_expr(des, scope);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

	/* If either expression can be evaluated ahead of time, then
	   do so. This can prove helpful later. */
      { NetExpr*tmp;
        tmp = lp->eval_tree();
	if (tmp) {
	      delete lp;
	      lp = tmp;
	}
	tmp = rp->eval_tree();
	if (tmp) {
	      delete rp;
	      rp = tmp;
	}
      }

      NetEBinary*tmp = elaborate_expr_base_(des, lp, rp);
}

/*
 * This is common elaboration of the operator. It presumes that the
 * operands are elaborated as necessary, and all I need to do is make
 * the correct NetEBinary object and connect the parameters.
 */
NetEBinary* PEBinary::elaborate_expr_base_(Design*des,
					   NetExpr*lp, NetExpr*rp) const
{
      bool flag;
      NetEBinary*tmp;

      switch (op_) {
	  default:
	    tmp = new NetEBinary(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case 'a':
	  case 'o':
	    tmp = new NetEBLogic(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case '*':
	    tmp = new NetEBMult(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case 'l':
	  case 'r':
	    tmp = new NetEBShift(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case '^':
	  case '&':
	  case '|':
	  case 'O':
	  case 'X':
	    tmp = new NetEBBits(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case '+':
	  case '-':
	    tmp = new NetEBAdd(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case 'e': /* == */
	  case 'E': /* === */
	  case 'n': /* != */
	  case 'N': /* !== */
	  case 'L': /* <= */
	  case 'G': /* >= */
	  case '<':
	  case '>':
	    tmp = new NetEBComp(op_, lp, rp);
	    tmp->set_line(*this);
	    flag = tmp->set_width(1);
	    if (flag == false) {
		  cerr << get_line() << ": internal error: "
			"expression bit width of comparison != 1." << endl;
		  des->errors += 1;
	    }
	    break;
      }

      return tmp;
}

NetESFunc* PECallFunction::elaborate_sfunc_(Design*des, NetScope*) const
{
      cerr << get_line() << ": sorry: system functions not supported."
	   << endl;
      des->errors += 1;
      return 0;
}

NetExpr* PECallFunction::elaborate_expr(Design*des, NetScope*scope) const
{
      if (name_[0] == '$')
	    return elaborate_sfunc_(des, scope);

      NetFuncDef*def = des->find_function(scope->name(), name_);
      if (def == 0) {
	    cerr << get_line() << ": error: No function " << name_ <<
		  " in this context (" << scope->name() << ")." << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(def);


      svector<NetExpr*> parms (parms_.count());

	/* Elaborate the input expressions for the function. This is
	   done in the scope of the function call, and not the scope
	   of the function being called. The scope of the called
	   function is elaborated when the definition is elaborated. */

      for (unsigned idx = 0 ;  idx < parms.count() ;  idx += 1) {
	    NetExpr*tmp = parms_[idx]->elaborate_expr(des, scope);
	    parms[idx] = tmp;
      }


	/* Look for the return value signal for the called
	   function. This return value is a magic signal in the scope
	   of the function, that has the name of the function. The
	   function code assigns to this signal to return a value. */

      NetNet*res = des->find_signal(def->name(), name_);
      if (res == 0) {
	    cerr << get_line() << ": internal error: Unable to locate "
		  "function return value for " << name_ << " in " <<
		  def->name() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      assert(res);
      NetESignal*eres = new NetESignal(res);
      assert(eres);
      NetEUFunc*func = new NetEUFunc(def, eres, parms);
      return func;
}


NetExpr* PEConcat::elaborate_expr(Design*des, NetScope*scope) const
{
      unsigned repeat = 1;

	/* If there is a repeat expression, then evaluate the constant
	   value and set the repeat count. */
      if (repeat_) {
	    verinum*vrep = repeat_->eval_const(des, scope->name());
	    if (vrep == 0) {
		  cerr << get_line() << ": error: "
			"concatenation repeat expression cannot be evaluated."
		       << endl;
		  des->errors += 1;
		  return 0;
	    }

	    repeat = vrep->as_ulong();
	    delete vrep;
      }

	/* Make the empty concat expression. */
      NetEConcat*tmp = new NetEConcat(parms_.count(), repeat);
      tmp->set_line(*this);

	/* Elaborate all the parameters and attach them to the concat node. */
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    assert(parms_[idx]);
	    NetExpr*ex = parms_[idx]->elaborate_expr(des, scope);
	    if (ex == 0) continue;
	    ex->set_line(*parms_[idx]);
	    tmp->set(idx, ex);
      }

      return tmp;
}

NetExpr* PEIdent::elaborate_expr(Design*des, NetScope*scope) const
{
	// System identifiers show up in the netlist as identifiers.
      if (text_[0] == '$')
	    return new NetEIdent(text_, 64);

	//string name = path+"."+text_;
      assert(scope);

	// If the identifier name is a parameter name, then return
	// a reference to the parameter expression.
      if (const NetExpr*ex = des->find_parameter(scope, text_)) {
	    NetExpr*tmp;
	    if (dynamic_cast<const NetExpr*>(ex))
		  tmp = ex->dup_expr();
	    else
		  tmp = new NetEParam(des, scope, text_);

	    tmp->set_line(*this);
	    return tmp;
      }

	// If the identifier names a signal (a register or wire)
	// then create a NetESignal node to handle it.
      if (NetNet*net = des->find_signal(scope->name(), text_)) {

	      // If this is a part select of a signal, then make a new
	      // temporary signal that is connected to just the
	      // selected bits.
	    if (lsb_) {
		  assert(msb_);
		  verinum*lsn = lsb_->eval_const(des, scope->name());
		  verinum*msn = msb_->eval_const(des, scope->name());
		  if ((lsn == 0) || (msn == 0)) {
			cerr << get_line() << ": error: "
			      "Part select expresions must be "
			      "constant expressions." << endl;
			des->errors += 1;
			return 0;
		  }

		  assert(lsn);
		  assert(msn);
		  unsigned long lsv = lsn->as_ulong();
		  unsigned long msv = msn->as_ulong();
		  unsigned long wid = 1 + ((msv>lsv)? (msv-lsv) : (lsv-msv));
		  assert(wid <= net->pin_count());
		  assert(net->sb_to_idx(msv) >= net->sb_to_idx(lsv));

		  string tname = des->local_symbol(scope->name());
		  NetTmp*tsig = new NetTmp(tname, wid);

		    // Connect the pins from the lsb up to the msb.
		  unsigned off = net->sb_to_idx(lsv);
		  for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
			connect(tsig->pin(idx), net->pin(idx+off));

		  NetESignal*tmp = new NetESignal(tsig);
		  tmp->set_line(*this);

		  des->add_signal(tsig);
		  return tmp;
	    }

	      // If the bit select is constant, then treat it similar
	      // to the part select, so that I save the effort of
	      // making a mux part in the netlist.
	    verinum*msn;
	    if (msb_ && (msn = msb_->eval_const(des, scope->name()))) {
		  assert(idx_ == 0);
		  unsigned long msv = msn->as_ulong();
		  unsigned idx = net->sb_to_idx(msv);

		  if (idx >= net->pin_count()) {
			cerr << get_line() << ": internal error: "
			     << "bit " << msv << " out of range of net "
			     << net->name() << "[" << net->msb()
			     << ":" << net->lsb() << "]." << endl;
			return 0;
		  }

		  string tname = des->local_symbol(scope->name());
		  NetTmp*tsig = new NetTmp(tname);
		  connect(tsig->pin(0), net->pin(idx));
		  NetESignal*tmp = new NetESignal(tsig);
		  tmp->set_line(*this);

		  des->add_signal(tsig);
		  return tmp;
	    }

	    NetESignal*node = new NetESignal(net);
	    assert(idx_ == 0);

	      // Non-constant bit select? punt and make a subsignal
	      // device to mux the bit in the net.
	    if (msb_) {
		  NetExpr*ex = msb_->elaborate_expr(des, scope);
		  NetESubSignal*ss = new NetESubSignal(node, ex);
		  ss->set_line(*this);
		  return ss;
	    }

	      // All else fails, return the signal itself as the
	      // expression.
	    assert(msb_ == 0);
	    return node;
      }

	// If the identifier names a memory, then this is a
	// memory reference and I must generate a NetEMemory
	// object to handle it.
      if (NetMemory*mem = des->find_memory(scope->name(), text_)) {
	    if (msb_ == 0) {
		  NetEMemory*node = new NetEMemory(mem);
		  node->set_line(*this);
		  return node;
	    }
	    assert(msb_ != 0);
	    assert(lsb_ == 0);
	    assert(idx_ == 0);
	    NetExpr*i = msb_->elaborate_expr(des, scope);
	    if (msb_ && i == 0) {
		  cerr << get_line() << ": error: Unable to exaborate "
			"index expression `" << *msb_ << "'" << endl;
		  des->errors += 1;
		  return 0;
	    }

	    NetEMemory*node = new NetEMemory(mem, i);
	    node->set_line(*this);
	    return node;
      }

	// Finally, if this is a scope name, then return that.
      if (NetScope*nsc = des->find_scope(text_)) {
	    NetEScope*tmp = new NetEScope(nsc);
	    tmp->set_line(*this);
	    return tmp;
      }

	// I cannot interpret this identifier. Error message.
      cerr << get_line() << ": error: Unable to bind wire/reg/memory "
	    "`" << text_ << "' in `" << scope->name() << "'" << endl;
      des->errors += 1;
      return 0;
}

NetEConst* PENumber::elaborate_expr(Design*des, NetScope*) const
{
      assert(value_);
      NetEConst*tmp = new NetEConst(*value_);
      tmp->set_line(*this);
      return tmp;
}

NetEConst* PEString::elaborate_expr(Design*des, NetScope*) const
{
      NetEConst*tmp = new NetEConst(value());
      tmp->set_line(*this);
      return tmp;
}

/*
 * Elaborate the Ternary operator. I know that the expressions were
 * parsed so I can presume that they exist, and call elaboration
 * methods. If any elaboration fails, then give up and return 0.
 */
NetETernary*PETernary::elaborate_expr(Design*des, NetScope*scope) const
{
      assert(expr_);
      assert(tru_);
      assert(fal_);

      NetExpr*con = expr_->elaborate_expr(des, scope);
      if (con == 0)
	    return 0;

      NetExpr*tru = tru_->elaborate_expr(des, scope);
      if (tru == 0) {
	    delete con;
	    return 0;
      }

      NetExpr*fal = fal_->elaborate_expr(des, scope);
      if (fal == 0) {
	    delete con;
	    delete tru;
	    return 0;
      }

      NetETernary*res = new NetETernary(con, tru, fal);
      return res;
}

NetEUnary* PEUnary::elaborate_expr(Design*des, NetScope*scope) const
{
      NetExpr*ip = expr_->elaborate_expr(des, scope);
      if (ip == 0) return 0;

      /* Should we evaluate expressions ahead of time,
       * just like in PEBinary::elaborate_expr() ?
       */

      NetEUnary*tmp;
      switch (op_) {
	  default:
	    tmp = new NetEUnary(op_, ip);
	    tmp->set_line(*this);
	    break;
	  case '~':
	    tmp = new NetEUBits(op_, ip);
	    tmp->set_line(*this);
	    break;
      }
      return tmp;
}

/*
 * $Log: elab_expr.cc,v $
 * Revision 1.19  2000/03/20 16:57:22  steve
 *  select correct bit when reg has non-zero lsb.
 *
 * Revision 1.18  2000/03/12 18:22:11  steve
 *  Binary and unary operators in parameter expressions.
 *
 * Revision 1.17  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.16  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.15  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.14  2000/01/01 06:18:00  steve
 *  Handle synthesis of concatenation.
 *
 * Revision 1.13  1999/12/12 06:03:14  steve
 *  Allow memories without indices in expressions.
 *
 * Revision 1.12  1999/11/30 04:54:01  steve
 *  Match scope names as last resort.
 *
 * Revision 1.11  1999/11/28 23:42:02  steve
 *  NetESignal object no longer need to be NetNode
 *  objects. Let them keep a pointer to NetNet objects.
 *
 * Revision 1.10  1999/11/27 19:07:57  steve
 *  Support the creation of scopes.
 *
 * Revision 1.9  1999/11/21 17:35:37  steve
 *  Memory name lookup handles scopes.
 *
 * Revision 1.8  1999/11/10 02:52:24  steve
 *  Create the vpiMemory handle type.
 *
 * Revision 1.7  1999/10/18 00:02:21  steve
 *  Catch unindexed memory reference.
 *
 * Revision 1.6  1999/09/30 02:43:02  steve
 *  Elaborate ~^ and ~| operators.
 *
 * Revision 1.5  1999/09/30 00:48:49  steve
 *  Cope with errors during ternary operator elaboration.
 *
 * Revision 1.4  1999/09/29 22:57:10  steve
 *  Move code to elab_expr.cc
 *
 * Revision 1.3  1999/09/25 02:57:30  steve
 *  Parse system function calls.
 *
 * Revision 1.2  1999/09/21 00:13:40  steve
 *  Support parameters that reference other paramters.
 *
 * Revision 1.1  1999/09/20 02:21:10  steve
 *  Elaborate parameters in phases.
 *
 */

