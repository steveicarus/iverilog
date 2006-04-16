/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elab_lval.cc,v 1.34 2006/04/16 00:15:43 steve Exp $"
#endif

# include "config.h"

# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"

# include  <iostream>

/*
 * These methods generate a NetAssign_ object for the l-value of the
 * assignment. This is common code for the = and <= statements.
 *
 * What gets generated depends on the structure of the l-value. If the
 * l-value is a simple name (i.e., foo <= <value>) the the NetAssign_
 * is created the width of the foo reg and connected to all the
 * bits.
 *
 * If there is a part select (i.e., foo[3:1] <= <value>) the NetAssign_
 * is made only as wide as it needs to be (3 bits in this example) and
 * connected to the correct bits of foo. A constant bit select is a
 * special case of the part select.
 *
 * If the bit-select is non-constant (i.e., foo[<expr>] = <value>) the
 * NetAssign_ is made wide enough to connect to all the bits of foo,
 * then the mux expression is elaborated and attached to the
 * NetAssign_ node as a b_mux value. The target must interpret the
 * presence of a bmux value as taking a single bit and assigning it to
 * the bit selected by the bmux expression.
 *
 * If the l-value expression is non-trivial, but can be fully
 * evaluated at compile time (meaning any bit selects are constant)
 * then elaboration will make a single NetAssign_ that connects to a
 * synthetic reg that in turn connects to all the proper pins of the
 * l-value.
 *
 * This last case can turn up in statements like: {a, b[1]} = c;
 * rather then create a NetAssign_ for each item in the concatenation,
 * elaboration makes a single NetAssign_ and connects it up properly.
 */


/*
 * The default interpretation of an l-value to a procedural assignment
 * is to try to make a net elaboration, and see if the result is
 * suitable for assignment.
 */
NetAssign_* PExpr::elaborate_lval(Design*des,
				  NetScope*scope,
				  bool is_force) const
{
      NetNet*ll = 0;
      if (ll == 0) {
	    cerr << get_line() << ": Assignment l-value too complex."
		 << endl;
	    return 0;
      }

      NetAssign_*lv = new NetAssign_(ll);
      return lv;
}

/*
 * Concatenation expressions can appear as l-values. Handle them here.
 *
 * If adjacent l-values in the concatenation are not bit selects, then
 * merge them into a single NetAssign_ object. This can happen is code
 * like ``{ ...a, b, ...}''. As long as "a" and "b" do not have bit
 * selects (or the bit selects are constant) we can merge the
 * NetAssign_ objects.
 *
 * Be careful to get the bit order right. In the expression ``{a, b}''
 * a is the MSB and b the LSB. Connect the LSB to the low pins of the
 * NetAssign_ object.
 */
NetAssign_* PEConcat::elaborate_lval(Design*des,
				     NetScope*scope,
				     bool is_force) const
{
      if (repeat_) {
	    cerr << get_line() << ": error: Repeat concatenations make "
		  "no sense in l-value expressions. I refuse." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetAssign_*res = 0;

      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {

	    if (parms_[idx] == 0) {
		  cerr << get_line() << ": error: Empty expressions "
		       << "not allowed in concatenations." << endl;
		  des->errors += 1;
		  continue;
	    }

	    NetAssign_*tmp = parms_[idx]->elaborate_lval(des, scope, is_force);

	      /* If the l-value doesn't elaborate, the error was
		 already detected and printed. We just skip it and let
		 the compiler catch more errors. */
	    if (tmp == 0)
		  continue;

	    assert(tmp);


	      /* Link the new l-value to the previous one. */

	    NetAssign_*last = tmp;
	    while (last->more)
		  last = last->more;

	    last->more = res;
	    res = tmp;
      }

      return res;
}

/*
 * Handle the ident as an l-value. This includes bit and part selects
 * of that ident.
 */
NetAssign_* PEIdent::elaborate_lval(Design*des,
				    NetScope*scope,
				    bool is_force) const
{
      NetNet*       reg = 0;
      NetMemory*    mem = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      symbol_search(des, scope, path_, reg, mem, par, eve);

      if (mem) {
	    if (is_force) {
		  cerr << get_line() << ": error: Memories "
		       << "(" << path_ << " in this case)"
		       << " are not allowed"
		       << " as l-values to force statements." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    return elaborate_mem_lval_(des, scope, mem);
      }

      if (reg == 0) {
	    cerr << get_line() << ": error: Could not find variable ``"
		 << path_ << "'' in ``" << scope->name() <<
		  "''" << endl;

	    des->errors += 1;
	    return 0;
      }

      assert(reg);

      if (sel_ == SEL_IDX_UP)
	    return elaborate_lval_net_idx_up_(des, scope, reg);

      if (sel_ == SEL_IDX_DO)
	    return elaborate_lval_net_idx_do_(des, scope, reg);

	/* Get the signal referenced by the identifier, and make sure
	   it is a register. Wires are not allows in this context,
	   unless this is the l-value of a force. */
      if ((reg->type() != NetNet::REG) && !is_force) {
	    cerr << get_line() << ": error: " << path_ <<
		  " is not a reg/integer/time in " << scope->name() <<
		  "." << endl;
	    cerr << reg->get_line() << ":      : " << path_ <<
		  " is declared here as " << reg->type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      long msb, lsb;
      NetExpr*mux;

      if (msb_ || lsb_) {
	    assert(msb_ && lsb_);
	    if (sel_ != SEL_PART)
		  cerr << get_line() << ": internal error: not a part select?"
		       << endl;

	    assert(sel_ == SEL_PART);

	      /* This handles part selects. In this case, there are
		 two bit select expressions, and both must be
		 constant. Evaluate them and pass the results back to
		 the caller. */
	    verinum*vl = lsb_->eval_const(des, scope);
	    if (vl == 0) {
		  cerr << lsb_->get_line() << ": error: "
			"Part select expressions must be constant."
		       << endl;
		  cerr << lsb_->get_line() << ":      : This lsb expression "
			"violates the rule: " << *lsb_ << endl;
		  des->errors += 1;
		  return 0;
	    }
	    verinum*vm = msb_->eval_const(des, scope);
	    if (vm == 0) {
		  cerr << msb_->get_line() << ": error: "
			"Part select expressions must be constant."
		       << endl;
		  cerr << msb_->get_line() << ":      : This msb expression "
			"violates the rule: " << *msb_ << endl;
		  des->errors += 1;
		  return 0;
	    }

	    msb = vm->as_long();
	    lsb = vl->as_long();
	    mux = 0;

      } else if (! idx_.empty()) {

	      /* If there is only a single select expression, it is a
		 bit select. Evaluate the constant value and treat it
		 as a part select with a bit width of 1. If the
		 expression it not constant, then return the
		 expression as a mux. */
	    assert(msb_ == 0);
	    assert(lsb_ == 0);
	    assert(idx_.size() == 1);
	    verinum*v = idx_[0]->eval_const(des, scope);
	    if (v == 0) {
		  NetExpr*m = idx_[0]->elaborate_expr(des, scope);
		  assert(m);
		  msb = 0;
		  lsb = 0;
		  mux = m;

	    } else {

		  msb = v->as_long();
		  lsb = v->as_long();
		  mux = 0;
	    }

      } else {

	      /* No select expressions, so presume a part select the
		 width of the register. */

	    assert(msb_ == 0);
	    assert(lsb_ == 0);
	    msb = reg->msb();
	    lsb = reg->lsb();
	    mux = 0;
      }


      NetAssign_*lv;
      if (mux) {

	      /* If there is a non-constant bit select, make a
		 NetAssign_ to the target reg and attach a
		 bmux to select the target bit. */
	    lv = new NetAssign_(reg);

	      /* Correct the mux for the range of the vector. */
	    if (reg->msb() < reg->lsb())
		  mux = make_sub_expr(reg->lsb(), mux);
	    else if (reg->lsb() != 0)
		  mux = make_add_expr(mux, - reg->lsb());

	    lv->set_part(mux, 1);

      } else if (msb == reg->msb() && lsb == reg->lsb()) {

	      /* No bit select, and part select covers the entire
		 vector. Simplest case. */
	    lv = new NetAssign_(reg);

      } else {

	      /* If the bit/part select is constant, then make the
		 NetAssign_ only as wide as it needs to be and connect
		 only to the selected bits of the reg. */
	    unsigned loff = reg->sb_to_idx(lsb);
	    unsigned moff = reg->sb_to_idx(msb);
	    unsigned wid = moff - loff + 1;

	    if (moff < loff) {
		  cerr << get_line() << ": error: part select "
		       << reg->name() << "[" << msb<<":"<<lsb<<"]"
		       << " is reversed." << endl;
		  des->errors += 1;
		  return 0;
	    }

	      /* If the part select extends beyond the extreme of the
		 variable, then report an error. Note that loff is
		 converted to normalized form so is relative the
		 variable pins. */

	    if ((wid + loff) > reg->vector_width()) {
		  cerr << get_line() << ": error: bit/part select "
		       << reg->name() << "[" << msb<<":"<<lsb<<"]"
		       << " is out of range." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    lv = new NetAssign_(reg);
	    lv->set_part(new NetEConst(verinum(loff)), wid);
      }


      return lv;
}

NetAssign_* PEIdent::elaborate_lval_net_idx_up_(Design*des,
						NetScope*scope,
						NetNet*reg) const
{
      assert(lsb_);
      assert(msb_);

      if (reg->type() != NetNet::REG) {
	    cerr << get_line() << ": error: " << path_ <<
		  " is not a reg/integer/time in " << scope->name() <<
		  "." << endl;
	    cerr << reg->get_line() << ":      : " << path_ <<
		  " is declared here as " << reg->type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* Calculate the width expression (in the lsb_ position)
	   first. If the expression is not constant, error but guess 1
	   so we can keep going and find more errors. */
      NetExpr*wid_ex = elab_and_eval(des, scope, lsb_);
      NetEConst*wid_c = dynamic_cast<NetEConst*>(wid_ex);

      if (wid_c == 0) {
	    cerr << get_line() << ": error: Indexed part width must be "
		 << "constant. Expression in question is..." << endl;
	    cerr << get_line() << ":      : " << *wid_ex << endl;
	    des->errors += 1;
      }

      unsigned wid = wid_c? wid_c->value().as_ulong() : 1;
      delete wid_ex;

      NetExpr*base = elab_and_eval(des, scope, msb_);

	/* Correct the mux for the range of the vector. */
      if (reg->msb() < reg->lsb())
	    base = make_sub_expr(reg->lsb(), base);
      else if (reg->lsb() != 0)
	    base = make_add_expr(base, - reg->lsb());

      NetAssign_*lv = new NetAssign_(reg);
      lv->set_part(base, wid);

      return lv;
}

NetAssign_* PEIdent::elaborate_lval_net_idx_do_(Design*des,
						NetScope*scope,
						NetNet*reg) const
{
      assert(lsb_);
      assert(msb_);
      cerr << get_line() << ": internal error: don't know how to "
	    "deal with SEL_IDX_DO in lval?" << endl;
      des->errors += 1;
      return 0;
}

NetAssign_* PEIdent::elaborate_mem_lval_(Design*des, NetScope*scope,
					NetMemory*mem) const
{
      if (idx_.empty()) {
	    cerr << get_line() << ": error: Assign to memory \""
		 << mem->name() << "\" requires a word select index."
		 << endl;
	    des->errors += 1;
	    return 0;
      }

      if (idx_.size() != mem->dimensions()) {
	    cerr << get_line() << ": error: " << idx_.size()
		 << " indices do not properly address a "
		 << mem->dimensions() << "-dimension memory/array."
		 << endl;
	    des->errors += 1;
	    return 0;
      }

	// XXXX For now, require exactly 1 index.
      assert(idx_.size() == 1);

	/* Elaborate the address expression. */
      NetExpr*ix = elab_and_eval(des, scope, idx_[0]);
      if (ix == 0)
	    return 0;

      NetAssign_*lv = new NetAssign_(mem);
      lv->set_bmux(ix);

	/* If there is no extra part select, then we are done. */
      if (msb_ == 0 && lsb_ == 0) {
	    lv->set_part(0, mem->width());
	    return lv;
      }

      assert(sel_ != SEL_NONE);
      assert(msb_ && lsb_);

      if (sel_ == SEL_PART) {
	    NetExpr*le = elab_and_eval(des, scope, lsb_);
	    NetExpr*me = elab_and_eval(des, scope, msb_);

	    NetEConst*lec = dynamic_cast<NetEConst*>(le);
	    NetEConst*mec = dynamic_cast<NetEConst*>(me);

	    if (lec == 0 || mec == 0) {
		  cerr << get_line() << ": error: Part select "
		       << "expressions must be constant." << endl;
		  des->errors += 1;
		  delete le;
		  delete me;
		  return lv;
	    }

	    verinum wedv = mec->value() - lec->value();
	    unsigned wid = wedv.as_long() + 1;

	    lv->set_part(lec, wid);
	    return lv;
      }

      assert(sel_ == SEL_IDX_UP || sel_ == SEL_IDX_DO);

      NetExpr*wid_ex = elab_and_eval(des, scope, lsb_);
      NetEConst*wid_ec = dynamic_cast<NetEConst*> (wid_ex);
      if (wid_ec == 0) {
	    cerr << lsb_->get_line() << ": error: "
		 << "Second expression of indexed part select "
		 << "most be constant." << endl;
	    des->errors += 1;
	    return lv;
      }

      unsigned wid = wid_ec->value().as_ulong();

      NetExpr*base_ex = elab_and_eval(des, scope, msb_);
      if (base_ex == 0) {
	    return 0;
      }

      if (sel_ == SEL_IDX_DO && wid > 1) {
	    base_ex = make_add_expr(base_ex, 1-(long)wid);
      }

      lv->set_part(base_ex, wid);
      return lv;
}

NetAssign_* PENumber::elaborate_lval(Design*des, NetScope*, bool) const
{
      cerr << get_line() << ": error: Constant values not allowed "
	   << "in l-value expressions." << endl;
      des->errors += 1;
      return 0;
}

/*
 * $Log: elab_lval.cc,v $
 * Revision 1.34  2006/04/16 00:15:43  steve
 *  Fix part selects in l-values.
 *
 * Revision 1.33  2006/02/02 02:43:57  steve
 *  Allow part selects of memory words in l-values.
 *
 * Revision 1.32  2005/07/11 16:56:50  steve
 *  Remove NetVariable and ivl_variable_t structures.
 *
 * Revision 1.31  2004/12/29 23:55:43  steve
 *  Unify elaboration of l-values for all proceedural assignments,
 *  including assing, cassign and force.
 *
 *  Generate NetConcat devices for gate outputs that feed into a
 *  vector results. Use this to hande gate arrays. Also let gate
 *  arrays handle vectors of gates when the outputs allow for it.
 *
 * Revision 1.30  2004/12/11 02:31:25  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.29  2004/10/04 01:10:52  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.28  2004/08/28 14:59:44  steve
 *  More detailed error message about bad variable.
 *
 * Revision 1.27  2003/09/19 03:30:05  steve
 *  Fix name search in elab_lval.
 */

