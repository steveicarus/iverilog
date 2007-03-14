/*
 * Copyright (c) 2000-2006 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elab_lval.cc,v 1.42 2007/03/14 05:06:49 steve Exp $"
#endif

# include "config.h"

# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "compiler.h"
# include  <iostream>
# include  "ivl_assert.h"

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
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      symbol_search(des, scope, path_, reg, par, eve);
      if (reg == 0) {
	    cerr << get_line() << ": error: Could not find variable ``"
		 << path_ << "'' in ``" << scope->name() <<
		  "''" << endl;

	    des->errors += 1;
	    return 0;
      }

      assert(reg);

	// This is the special case that the l-value is an entire
	// memory. This is, in fact, an error.
      if (reg->array_dimensions() > 0 && idx_.size() == 0) {
	    cerr << get_line() << ": error: Cannot assign to array "
		 << path_ << ". Did you forget a word index?" << endl;
	    des->errors += 1;
	    return 0;
      }

      if (reg->array_dimensions() > 0)
	    return elaborate_lval_net_word_(des, scope, reg);

      if (sel_ == SEL_PART) {
	    NetAssign_*lv = new NetAssign_(reg);
	    elaborate_lval_net_part_(des, scope, lv);
	    return lv;
      }

      if (sel_ == SEL_IDX_UP) {
	    NetAssign_*lv = new NetAssign_(reg);
	    elaborate_lval_net_idx_up_(des, scope, lv);
	    return lv;
      }

      if (sel_ == SEL_IDX_DO) {
	    NetAssign_*lv = new NetAssign_(reg);
	    elaborate_lval_net_idx_do_(des, scope, lv);
	    return lv;
      }

	/* Get the signal referenced by the identifier, and make sure
	   it is a register. Wires are not allows in this context,
	   unless this is the l-value of a force. */
      if ((reg->type() != NetNet::REG) && !is_force) {
	    cerr << get_line() << ": error: " << path_ <<
		  " is not a valid l-value in " << scope->name() <<
		  "." << endl;
	    cerr << reg->get_line() << ":      : " << path_ <<
		  " is declared here as " << reg->type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      ivl_assert(*this, msb_ == 0);
      ivl_assert(*this, lsb_ == 0);
      long msb, lsb;
      NetExpr*mux;

      if (! idx_.empty()) {

	      /* If there is only a single select expression, it is a
		 bit select. Evaluate the constant value and treat it
		 as a part select with a bit width of 1. If the
		 expression it not constant, then return the
		 expression as a mux. */

	    ivl_assert(*this, idx_.size() == 1);

	    NetExpr*index_expr = elab_and_eval(des, scope, idx_[0], -1);

	    if (NetEConst*index_con = dynamic_cast<NetEConst*> (index_expr)) {
		  msb = index_con->value().as_long();
		  lsb = index_con->value().as_long();
		  mux = 0;

	    } else {
		  msb = 0;
		  lsb = 0;
		  mux = index_expr;
	    }

      } else {

	      /* No select expressions, so presume a part select the
		 width of the register. */

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

NetAssign_* PEIdent::elaborate_lval_net_word_(Design*des,
					      NetScope*scope,
					      NetNet*reg) const
{
      assert(idx_.size() == 1);

      NetExpr*word = elab_and_eval(des, scope, idx_[0], -1);

	// If there is a non-zero base to the memory, then build an
	// expression to calculate the canonical address.
      if (long base = reg->array_first()) {

	    word = make_add_expr(word, 0-base);
	    if (NetExpr*tmp = word->eval_tree()) {
		  word = tmp;
	    }
      }

      NetAssign_*lv = new NetAssign_(reg);
      lv->set_word(word);

      if (debug_elaborate)
	    cerr << get_line() << ": debug: Set array word=" << *word << endl;

	// Test for the case that the index is a constant, and is out
	// of bounds. The "word" expression is the word index already
	// converted to canonical address, so this just needs to check
	// that the address is not too big.
      if (NetEConst*word_const = dynamic_cast<NetEConst*>(word)) {
	    verinum word_val = word_const->value();
	    long index = word_val.as_long();
	    if (index < 0 || index >= reg->array_count()) {
		  cerr << get_line() << ": warning: Constant array index "
		       << (index + reg->array_first())
		       << " is out of range for array "
		       << reg->name() << "." << endl;
	    }
      }

	/* An array word may also have part selects applied to them. */

      if (sel_ == SEL_PART)
	    elaborate_lval_net_part_(des, scope, lv);

      if (sel_ == SEL_IDX_UP)
	    elaborate_lval_net_idx_up_(des, scope, lv);

      if (sel_ == SEL_IDX_DO)
	    elaborate_lval_net_idx_do_(des, scope, lv);

      return lv;
}

bool PEIdent::elaborate_lval_net_part_(Design*des,
				       NetScope*scope,
				       NetAssign_*lv) const
{
      long msb, lsb;
      bool flag = calculate_parts_(des, scope, msb, lsb);
      if (!flag)
	    return false;

      NetNet*reg = lv->sig();
      assert(reg);

      if (msb == reg->msb() && lsb == reg->lsb()) {

	      /* No bit select, and part select covers the entire
		 vector. Simplest case. */

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
		  return false;
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
		  return false;
	    }

	    lv->set_part(new NetEConst(verinum(loff)), wid);
      }

      return true;
}

bool PEIdent::elaborate_lval_net_idx_up_(Design*des,
					 NetScope*scope,
					 NetAssign_*lv) const
{
      assert(lsb_);
      assert(msb_);

      NetNet*reg = lv->sig();
      assert(reg);

      if (reg->type() != NetNet::REG) {
	    cerr << get_line() << ": error: " << path_ <<
		  " is not a reg/integer/time in " << scope->name() <<
		  "." << endl;
	    cerr << reg->get_line() << ":      : " << path_ <<
		  " is declared here as " << reg->type() << "." << endl;
	    des->errors += 1;
	    return false;
      }

      unsigned long wid;
      calculate_up_do_width_(des, scope, wid);

      NetExpr*base = elab_and_eval(des, scope, msb_, -1);

	/* Correct the mux for the range of the vector. */
      if (reg->msb() < reg->lsb())
	    base = make_sub_expr(reg->lsb(), base);
      else if (reg->lsb() != 0)
	    base = make_add_expr(base, - reg->lsb());

      if (debug_elaborate)
	    cerr << get_line() << ": debug: Set part select width="
		 << wid << ", base=" << *base << endl;

      lv->set_part(base, wid);

      return true;
}

bool PEIdent::elaborate_lval_net_idx_do_(Design*des,
					 NetScope*scope,
					 NetAssign_*lv) const
{
      assert(lsb_);
      assert(msb_);
      cerr << get_line() << ": internal error: don't know how to "
	    "deal with SEL_IDX_DO in lval?" << endl;
      des->errors += 1;
      return false;
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
 * Revision 1.42  2007/03/14 05:06:49  steve
 *  Replace some asserts with ivl_asserts.
 *
 * Revision 1.41  2007/03/05 05:59:10  steve
 *  Handle processes within generate loops.
 *
 * Revision 1.40  2007/02/27 05:14:38  steve
 *  Detect and warn about lval array index out fo bounds.
 *
 * Revision 1.39  2007/02/01 05:25:26  steve
 *  Error message better reflects more general reality.
 *
 * Revision 1.38  2007/01/16 05:44:15  steve
 *  Major rework of array handling. Memories are replaced with the
 *  more general concept of arrays. The NetMemory and NetEMemory
 *  classes are removed from the ivl core program, and the IVL_LPM_RAM
 *  lpm type is removed from the ivl_target API.
 *
 * Revision 1.37  2006/11/04 06:19:25  steve
 *  Remove last bits of relax_width methods, and use test_width
 *  to calculate the width of an r-value expression that may
 *  contain unsized numbers.
 *
 * Revision 1.36  2006/06/02 04:48:50  steve
 *  Make elaborate_expr methods aware of the width that the context
 *  requires of it. In the process, fix sizing of the width of unary
 *  minus is context determined sizes.
 *
 * Revision 1.35  2006/04/16 00:54:04  steve
 *  Cleanup lval part select handling.
 *
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

