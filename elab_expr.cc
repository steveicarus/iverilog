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
#ident "$Id: elab_expr.cc,v 1.43 2001/11/07 04:01:59 steve Exp $"
#endif

# include "config.h"


# include  "pform.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "util.h"

NetExpr* PExpr::elaborate_expr(Design*des, NetScope*) const
{
      cerr << get_line() << ": internal error: I do not know how to elaborate"
	   << " expression. " << endl;
      cerr << get_line() << ":               : Expression is: " << *this
	   << endl;
      des->errors += 1;
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
      return tmp;
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

	  case '/':
	  case '%':
	    tmp = new NetEBDiv(op_, lp, rp);
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

/*
 * Given a call to a system function, generate the proper expression
 * nodes to represent the call in the netlist. Since we don't support
 * size_tf functions, make assumptions about widths based on some
 * known function names.
 */
NetExpr* PECallFunction::elaborate_sfunc_(Design*des, NetScope*scope) const
{
      unsigned wid = 32;

      if (name_ == "$time")
	    wid = 64;


	/* How many parameters are there? The Verilog language allows
	   empty parameters in certain contexts, so the parser will
	   allow things like func(1,,3). It will also cause func() to
	   be interpreted as a single empty parameter.

	   Functions cannot really take empty parameters, but the
	   case ``func()'' is the same as no parmaters at all. So
	   catch that special case here. */
      unsigned nparms = parms_.count();
      if ((nparms == 1) && (parms_[0] == 0))
	    nparms = 0;

      NetESFunc*fun = new NetESFunc(name_, wid, nparms);

	/* Now run through the expected parameters. If we find that
	   there are missing parameters, print an error message.

	   While we're at it, try to evaluate the function parameter
	   expression as much as possible, and use the reduced
	   expression if one is created. */

      unsigned missing_parms = 0;
      for (unsigned idx = 0 ;  idx < nparms ;  idx += 1) {
	    PExpr*expr = parms_[idx];
	    if (expr) {
		  NetExpr*tmp1 = expr->elaborate_expr(des, scope);
		  if (NetExpr*tmp2 = tmp1->eval_tree()) {
			delete tmp1;
			fun->parm(idx, tmp2);
		  } else {
			fun->parm(idx, tmp1);
		  }

	    } else {
		  missing_parms += 1;
		  fun->parm(idx, 0);
	    }
      }

      if (missing_parms > 0) {
	    cerr << get_line() << ": error: The function " << name_
		 << " has been called with empty parameters." << endl;
	    cerr << get_line() << ":      : Verilog doesn't allow "
		 << "passing empty parameters to functions." << endl;
	    des->errors += 1;
      }

      return fun;
}

NetExpr* PECallFunction::elaborate_expr(Design*des, NetScope*scope) const
{
      if (name_[0] == '$')
	    return elaborate_sfunc_(des, scope);

      NetFuncDef*def = des->find_function(scope, name_);
      if (def == 0) {
	    cerr << get_line() << ": error: No function " << name_ <<
		  " in this context (" << scope->name() << ")." << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(def);

      NetScope*dscope = des->find_scope(def->name());
      assert(dscope);

	/* How many parameters have I got? Normally the size of the
	   list is correct, but there is the special case of a list of
	   1 nil pointer. This is how the parser tells me of no
	   parameter. In other words, ``func()'' is 1 nil parameter. */

      unsigned parms_count = parms_.count();
      if ((parms_count == 1) && (parms_[0] == 0))
	    parms_count = 0;

      if (dscope->type() != NetScope::FUNC) {
	    cerr << get_line() << ": error: Attempt to call scope "
		 << dscope->name() << " as a function." << endl;
	    des->errors += 1;
	    return 0;
      }

      if ((parms_count+1) != dscope->func_def()->port_count()) {
	    cerr << get_line() << ": error: Function " << dscope->name()
		 << " expects " << (dscope->func_def()->port_count()-1)
		 << " parameters, you passed " << parms_count << "."
		 << endl;
	    des->errors += 1;
	    return 0;
      }

      svector<NetExpr*> parms (parms_count);

	/* Elaborate the input expressions for the function. This is
	   done in the scope of the function call, and not the scope
	   of the function being called. The scope of the called
	   function is elaborated when the definition is elaborated. */

      unsigned missing_parms = 0;
      for (unsigned idx = 0 ;  idx < parms.count() ;  idx += 1) {
	    PExpr*tmp = parms_[idx];
	    if (tmp) {
		  parms[idx] = tmp->elaborate_expr(des, scope);

	    } else {
		  missing_parms += 1;
		  parms[idx] = 0;
	    }
      }

      if (missing_parms > 0) {
	    cerr << get_line() << ": error: The function " << name_
		 << " has been called with empty parameters." << endl;
	    cerr << get_line() << ":      : Verilog doesn't allow "
		 << "passing empty parameters to functions." << endl;
	    des->errors += 1;
      }


	/* Look for the return value signal for the called
	   function. This return value is a magic signal in the scope
	   of the function, that has the name of the function. The
	   function code assigns to this signal to return a value.

	   dscope, in this case, is the scope of the function, so the
	   return value is the name within that scope. */

      string rname = name_;
      NetNet*res = des->find_signal(dscope, parse_last_name(rname));
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
      NetEUFunc*func = new NetEUFunc(dscope, eres, parms);
      return func;
}


NetExpr* PEConcat::elaborate_expr(Design*des, NetScope*scope) const
{
      unsigned repeat = 1;

	/* If there is a repeat expression, then evaluate the constant
	   value and set the repeat count. */
      if (repeat_) {
	    NetExpr*tmp = elab_and_eval(des, scope, repeat_);
	    assert(tmp);
	    NetEConst*rep = dynamic_cast<NetEConst*>(tmp);

	    if (rep == 0) {
		  cerr << get_line() << ": error: "
			"concatenation repeat expression cannot be evaluated."
		       << endl;
		  cerr << get_line() << ":      : The expression is: "
		       << *tmp << endl;
		  des->errors += 1;
		  return 0;
	    }

	    repeat = rep->value().as_ulong();
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

	    if (! ex->has_width()) {
		  cerr << ex->get_line() << ": error: operand of "
		       << "concatenation has indefinite width: "
		       << *ex << endl;
		  des->errors += 1;
	    }

	    tmp->set(idx, ex);
      }

      return tmp;
}

NetExpr* PEFNumber::elaborate_expr(Design*des, NetScope*scope) const
{
      long val = value_->as_long();
      return new NetEConst(verinum(val));
}

NetExpr* PEIdent::elaborate_expr(Design*des, NetScope*scope) const
{
      assert(text_[0] != '$');

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
      if (NetNet*net = des->find_signal(scope, text_)) {

	      // If this is a part select of a signal, then make a new
	      // temporary signal that is connected to just the
	      // selected bits. The lsb_ and msb_ expressions are from
	      // the foo[msb:lsb] expression in the original.
	    if (lsb_) {
		  assert(msb_);
		  verinum*lsn = lsb_->eval_const(des, scope);
		  verinum*msn = msb_->eval_const(des, scope);
		  if ((lsn == 0) || (msn == 0)) {
			cerr << get_line() << ": error: "
			      "Part select expresions must be "
			      "constant expressions." << endl;
			des->errors += 1;
			return 0;
		  }

		  assert(lsn);
		  assert(msn);

		    /* The indices of part selects are signed
		       integers, so allow negative values. However,
		       the width that they represent is
		       unsigned. Remember that any order is possible,
		       i.e. [1:0], [-4,6], etc. */

		  long lsv = lsn->as_long();
		  long msv = msn->as_long();
		  unsigned long wid = 1 + ((msv>lsv)? (msv-lsv) : (lsv-msv));
		  if (wid > net->pin_count()) {
			cerr << get_line() << ": error: part select ["
			     << msv << ":" << lsv << "] out of range."
			     << endl;
			des->errors += 1;
			delete lsn;
			delete msn;
			return 0;
		  }
		  assert(wid <= net->pin_count());

		  if (net->sb_to_idx(msv) < net->sb_to_idx(lsv)) {
			cerr << get_line() << ": error: part select ["
			     << msv << ":" << lsv << "] out of order."
			     << endl;
			des->errors += 1;
			delete lsn;
			delete msn;
			return 0;
		  }


		  if (net->sb_to_idx(msv) >= net->pin_count()) {
			cerr << get_line() << ": error: part select ["
			     << msv << ":" << lsv << "] out of range."
			     << endl;
			des->errors += 1;
			delete lsn;
			delete msn;
			return 0;
		  }

		  NetESignal*tmp = new NetESignal(net,
						  net->sb_to_idx(msv),
						  net->sb_to_idx(lsv));
		  tmp->set_line(*this);

		  return tmp;
	    }

	      // If the bit select is constant, then treat it similar
	      // to the part select, so that I save the effort of
	      // making a mux part in the netlist.
	    verinum*msn;
	    if (msb_ && (msn = msb_->eval_const(des, scope))) {
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

		  NetESignal*tmp = new NetESignal(net, idx, idx);
		  tmp->set_line(*this);

		  return tmp;
	    }

	    NetESignal*node = new NetESignal(net);
	    assert(idx_ == 0);

	      // Non-constant bit select? punt and make a subsignal
	      // device to mux the bit in the net.
	    if (msb_) {
		  NetExpr*ex = msb_->elaborate_expr(des, scope);
		  NetEBitSel*ss = new NetEBitSel(node, ex);
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
      if (NetMemory*mem = des->find_memory(scope, text_)) {
	    if (msb_ == 0) {
		  NetEMemory*node = new NetEMemory(mem);
		  node->set_line(*this);
		  return node;
	    }
	    assert(msb_ != 0);
	    if (lsb_) {
		  cerr << get_line() << ": error: part select of a memory: "
		       << mem->name() << endl;
		  des->errors += 1;
		  return 0;
	    }

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

	// Finally, if this is a scope name, then return that. Look
	// first to see if this is a name of a local scope. Failing
	// that, search globally for a heirarchical name.
      if (NetScope*nsc = scope->child(text_)) {
	    NetEScope*tmp = new NetEScope(nsc);
	    tmp->set_line(*this);
	    return tmp;
      }

	// NOTE: This search pretty much assumes that text_ is a
	// complete hierarchical name, since there is no mention of
	// the current scope in the call to find_scope.
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
	  case '!': // Logical NOT
	  case '&': // Reduction AND
	  case '|': // Reduction OR
	  case '^': // Reduction XOR
	  case 'A': // Reduction NAND (~&)
	  case 'N': // Reduction NOR (~|)
	  case 'X': // Reduction NXOR (~^)
	    tmp = new NetEUReduce(op_, ip);
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
 * Revision 1.43  2001/11/07 04:01:59  steve
 *  eval_const uses scope instead of a string path.
 *
 * Revision 1.42  2001/07/29 22:22:40  steve
 *  support local reference to scope in expressions.
 *
 * Revision 1.41  2001/07/27 04:51:44  steve
 *  Handle part select expressions as variants of
 *  NetESignal/IVL_EX_SIGNAL objects, instead of
 *  creating new and useless temporary signals.
 *
 * Revision 1.40  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.39  2001/06/30 21:28:35  steve
 *  Detect parameter mismatches.
 *
 * Revision 1.38  2001/06/23 19:53:03  steve
 *  Look up functor return register with tail of name.
 *
 * Revision 1.37  2001/04/06 02:28:02  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.36  2001/02/10 20:29:39  steve
 *  In the context of range declarations, use elab_and_eval instead
 *  of the less robust eval_const methods.
 */

