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
#ident "$Id: elab_expr.cc,v 1.52 2002/04/13 02:33:17 steve Exp $"
#endif

# include "config.h"


# include  "pform.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "util.h"

NetExpr* PExpr::elaborate_expr(Design*des, NetScope*, bool) const
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
NetEBinary* PEBinary::elaborate_expr(Design*des, NetScope*scope, bool) const
{
      assert(left_);
      assert(right_);

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

	/* Catch the special case that the system function is the
	   $signed function. This function is special, in that it does
	   not lead to executable code but takes the single parameter
	   and makes it into a signed expression. No bits are changed,
	   it just changes the interpretation. */
      if (strcmp(path_.peek_name(0), "$signed") == 0) {
	    if ((parms_.count() != 1) || (parms_[0] == 0)) {
		  cerr << get_line() << ": error: The $signed() function "
		       << "takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    PExpr*expr = parms_[0];
	    NetExpr*sub = expr->elaborate_expr(des, scope, true);
	    sub->cast_signed(true);
	    return sub;
      }

      unsigned wid = 32;

      if (strcmp(path_.peek_name(0), "$time") == 0)
	    wid = 64;
      if (strcmp(path_.peek_name(0), "$stime") == 0)
	    wid = 32;


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

      NetESFunc*fun = new NetESFunc(path_.peek_name(0), wid, nparms);

	/* Now run through the expected parameters. If we find that
	   there are missing parameters, print an error message.

	   While we're at it, try to evaluate the function parameter
	   expression as much as possible, and use the reduced
	   expression if one is created. */

      unsigned missing_parms = 0;
      for (unsigned idx = 0 ;  idx < nparms ;  idx += 1) {
	    PExpr*expr = parms_[idx];
	    if (expr) {
		  NetExpr*tmp1 = expr->elaborate_expr(des, scope, true);
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
	    cerr << get_line() << ": error: The function "
		 << path_.peek_name(0)
		 << " has been called with empty parameters." << endl;
	    cerr << get_line() << ":      : Verilog doesn't allow "
		 << "passing empty parameters to functions." << endl;
	    des->errors += 1;
      }

      return fun;
}

NetExpr* PECallFunction::elaborate_expr(Design*des, NetScope*scope, bool) const
{
      if (path_.peek_name(0)[0] == '$')
	    return elaborate_sfunc_(des, scope);

      NetFuncDef*def = des->find_function(scope, path_);
      if (def == 0) {
	    cerr << get_line() << ": error: No function " << path_ <<
		  " in this context (" << scope->name() << ")." << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(def);

      NetScope*dscope = def->scope();
      assert(dscope);

      if (! check_call_matches_definition_(des, dscope))
	    return 0;

      unsigned parms_count = parms_.count();
      if ((parms_count == 1) && (parms_[0] == 0))
	    parms_count = 0;

      

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
	    cerr << get_line() << ": error: The function " << path_
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

      NetNet*res = dscope->find_signal(dscope->basename());
      if (res == 0) {
	    cerr << get_line() << ": internal error: Unable to locate "
		  "function return value for " << path_ << " in " <<
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


NetExpr* PEConcat::elaborate_expr(Design*des, NetScope*scope, bool) const
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

NetExpr* PEFNumber::elaborate_expr(Design*des, NetScope*scope, bool) const
{
      long val = value_->as_long();
      return new NetEConst(verinum(val));
}

/*
 * Elaborate an identifier in an expression. The identifier can be a
 * parameter name, a signal name or a memory name. It can also be a
 * scope name (Return a NetEScope) but only certain callers can use
 * scope names. However, we still support it here.
 *
 * Function names are not handled here, they are detected by the
 * parser and are elaborated by PECallFunction.
 *
 * The signal name may be escaped, but that affects nothing here.
 */
NetExpr* PEIdent::elaborate_expr(Design*des, NetScope*scope,
				 bool sys_task_arg) const
{
      assert(scope);

	// If the identifier name is a parameter name, then return
	// a reference to the parameter expression.
      if (const NetExpr*ex = des->find_parameter(scope, path_)) {
	    NetExpr*tmp;
	    if (dynamic_cast<const NetExpr*>(ex))
		  tmp = ex->dup_expr();
	    else
		  tmp = new NetEParam(des, scope, path_);

	    if (msb_ && lsb_) {
		  cerr << get_line() << ": error: part select of "
		       << "parameter " << path_ << " in " << scope->name()
		       << " is illegal." << endl;
		  des->errors += 1;

	    } else if (msb_) {
		    /* Handle the case where a parameter has a bit
		       select attached to it. Generate a NetESelect
		       object to select the bit as desired. */
		  NetExpr*mtmp = msb_->elaborate_expr(des, scope);
		  NetESelect*stmp = new NetESelect(tmp, mtmp, 1);
		  tmp->set_line(*this);
		  tmp = stmp;
	    }


	    tmp->set_line(*this);
	    return tmp;
      }

	// If the identifier names a signal (a register or wire)
	// then create a NetESignal node to handle it.
      if (NetNet*net = des->find_signal(scope, path_)) {

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
      if (NetMemory*mem = des->find_memory(scope, path_)) {
	    if (msb_ == 0) {

		    // If this memory is an argument to a system task,
		    // then it is OK for it to not have an index.
		  if (sys_task_arg) {
			NetEMemory*node = new NetEMemory(mem);
			node->set_line(*this);
			return node;
		  }

		    // If it is not a simple system task argument,
		    // this a missing index is an error.
		  cerr << get_line() << ": error: memory " << mem->name()
		       << " needs an index in this context." << endl;
		  des->errors += 1;
		  return 0;
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
      if ((path_.peek_name(1) == 0))
	    if (NetScope*nsc = scope->child(path_.peek_name(0))) {
		  NetEScope*tmp = new NetEScope(nsc);
		  tmp->set_line(*this);
		  return tmp;
	    }

	// NOTE: This search pretty much assumes that text_ is a
	// complete hierarchical name, since there is no mention of
	// the current scope in the call to find_scope.
      if (NetScope*nsc = des->find_scope(path_)) {
	    NetEScope*tmp = new NetEScope(nsc);
	    tmp->set_line(*this);
	    return tmp;
      }

	// I cannot interpret this identifier. Error message.
      cerr << get_line() << ": error: Unable to bind wire/reg/memory "
	    "`" << path_ << "' in `" << scope->name() << "'" << endl;
      des->errors += 1;
      return 0;
}

NetEConst* PENumber::elaborate_expr(Design*des, NetScope*, bool) const
{
      assert(value_);
      NetEConst*tmp = new NetEConst(*value_);
      tmp->set_line(*this);
      return tmp;
}

NetEConst* PEString::elaborate_expr(Design*des, NetScope*, bool) const
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
NetETernary*PETernary::elaborate_expr(Design*des, NetScope*scope, bool) const
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

NetEUnary* PEUnary::elaborate_expr(Design*des, NetScope*scope, bool) const
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
 * Revision 1.52  2002/04/13 02:33:17  steve
 *  Detect missing indices to memories (PR#421)
 *
 * Revision 1.51  2002/03/09 02:10:22  steve
 *  Add the NetUserFunc netlist node.
 *
 * Revision 1.50  2002/01/28 00:52:41  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.49  2002/01/11 05:25:45  steve
 *  The stime system function is 32bits.
 *
 * Revision 1.48  2001/12/31 00:08:14  steve
 *  Support $signed cast of expressions.
 *
 * Revision 1.47  2001/12/29 20:41:30  steve
 *  Allow escaped $ in identifiers.
 *
 * Revision 1.46  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.45  2001/11/19 02:54:12  steve
 *  Handle division and modulus by zero while
 *  evaluating run-time constants.
 *
 * Revision 1.44  2001/11/19 01:54:14  steve
 *  Port close cropping behavior from mcrgb
 *  Move window array reset to libmc.
 */

