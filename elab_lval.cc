/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "compiler.h"
# include  <cstdlib>
# include  <iostream>
# include  <climits>
# include  "ivl_assert.h"

/*
 * These methods generate a NetAssign_ object for the l-value of the
 * assignment. This is common code for the = and <= statements.
 *
 * What gets generated depends on the structure of the l-value. If the
 * l-value is a simple name (i.e., foo <= <value>) then the NetAssign_
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
	    cerr << get_fileline() << ": Assignment l-value too complex."
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
	    cerr << get_fileline() << ": error: Repeat concatenations make "
		  "no sense in l-value expressions. I refuse." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetAssign_*res = 0;

      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {

	    if (parms_[idx] == 0) {
		  cerr << get_fileline() << ": error: Empty expressions "
		       << "not allowed in concatenations." << endl;
		  des->errors += 1;
		  continue;
	    }

	    NetAssign_*tmp = parms_[idx]->elaborate_lval(des, scope, is_force);

	    if (tmp->expr_type() == IVL_VT_REAL) {
		  cerr << parms_[idx]->get_fileline() << ": error: "
		       << "concatenation operand can not be real: "
		       << *parms_[idx] << endl;
		  des->errors += 1;
		  continue;
	    }

	      /* If the l-value doesn't elaborate, the error was
		 already detected and printed. We just skip it and let
		 the compiler catch more errors. */
	    if (tmp == 0) continue;

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

      symbol_search(this, des, scope, path_, reg, par, eve);
      if (reg == 0) {
	    cerr << get_fileline() << ": error: Could not find variable ``"
		 << path_ << "'' in ``" << scope_path(scope) <<
		  "''" << endl;

	    des->errors += 1;
	    return 0;
      }

      ivl_assert(*this, reg);

      const name_component_t&name_tail = path_.back();

      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (!name_tail.index.empty())
	    use_sel = name_tail.index.back().sel;

	// This is the special case that the l-value is an entire
	// memory. This is, in fact, an error.
      if (reg->array_dimensions() > 0 && name_tail.index.empty()) {
	    cerr << get_fileline() << ": error: Cannot assign to array "
		 << path_ << ". Did you forget a word index?" << endl;
	    des->errors += 1;
	    return 0;
      }

	/* Get the signal referenced by the identifier, and make sure
	   it is a register. Wires are not allows in this context,
	   unless this is the l-value of a force. */
      if ((reg->type() != NetNet::REG) && !is_force) {
	    cerr << get_fileline() << ": error: " << path_ <<
		  " is not a valid l-value in " << scope_path(scope) <<
		  "." << endl;
	    cerr << reg->get_fileline() << ":      : " << path_ <<
		  " is declared here as " << reg->type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (reg->array_dimensions() > 0)
	    return elaborate_lval_net_word_(des, scope, reg);

	// This must be after the array word elaboration above!
      if (reg->get_scalar() &&
          use_sel != index_component_t::SEL_NONE) {
	    cerr << get_fileline() << ": error: can not select part of ";
	    if (reg->data_type() == IVL_VT_REAL) cerr << "real: ";
	    else cerr << "scalar: ";
	    cerr << reg->name() << endl;
	    des->errors += 1;
	    return 0;
      }

      if (use_sel == index_component_t::SEL_PART) {
	    NetAssign_*lv = new NetAssign_(reg);
	    elaborate_lval_net_part_(des, scope, lv);
	    return lv;
      }

      if (use_sel == index_component_t::SEL_IDX_UP ||
          use_sel == index_component_t::SEL_IDX_DO) {
	    NetAssign_*lv = new NetAssign_(reg);
	    elaborate_lval_net_idx_(des, scope, lv, use_sel);
	    return lv;
      }


      if (use_sel == index_component_t::SEL_BIT) {
	    NetAssign_*lv = new NetAssign_(reg);
	    elaborate_lval_net_bit_(des, scope, lv);
	    return lv;
      }

      ivl_assert(*this, use_sel == index_component_t::SEL_NONE);

	/* No select expressions. */

      NetAssign_*lv = new NetAssign_(reg);

      return lv;
}

NetAssign_* PEIdent::elaborate_lval_net_word_(Design*des,
					      NetScope*scope,
					      NetNet*reg) const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_head = name_tail.index.front();
      if (index_head.sel == index_component_t::SEL_PART) {
	    cerr << get_fileline() << ": error: cannot perform a part "
	         << "select on array " << reg->name() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      ivl_assert(*this, index_head.sel == index_component_t::SEL_BIT);
      ivl_assert(*this, index_head.msb != 0);
      ivl_assert(*this, index_head.lsb == 0);

	// These are not used, but they need to have a default value.
      ivl_variable_type_t expr_type_tmp = IVL_VT_NO_TYPE;
      bool unsized_flag_tmp = false;
      index_head.msb->test_width(des, scope, integer_width, integer_width,
                                 expr_type_tmp, unsized_flag_tmp);

      NetExpr*word = elab_and_eval(des, scope, index_head.msb, -1);

	// If there is a non-zero base to the memory, then build an
	// expression to calculate the canonical address.
      if (long base = reg->array_first()) {

	    word = normalize_variable_array_base(word, base,
	                                         reg->array_count());
	    eval_expr(word);
      }

      NetAssign_*lv = new NetAssign_(reg);
      lv->set_word(word);

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: Set array word=" << *word << endl;

	// Test for the case that the index is a constant, and is out
	// of bounds. The "word" expression is the word index already
	// converted to canonical address, so this just needs to check
	// that the address is not too big.
      if (NetEConst*word_const = dynamic_cast<NetEConst*>(word)) {
	    verinum word_val = word_const->value();
	    long index = word_val.as_long();

	    if (index < 0 || index >= (long) reg->array_count()) {
		  cerr << get_fileline() << ": warning: Constant array index "
		       << (index + reg->array_first())
		       << " is out of range for array "
		       << reg->name() << "." << endl;
	    }
      }

	/* An array word may also have part selects applied to them. */

      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (name_tail.index.size() > 1)
	    use_sel = name_tail.index.back().sel;

      if (reg->get_scalar() &&
          use_sel != index_component_t::SEL_NONE) {
	    cerr << get_fileline() << ": error: can not select part of ";
	    if (reg->data_type() == IVL_VT_REAL) cerr << "real";
	    else cerr << "scalar";
	    cerr << " array word: " << reg->name()
	         << "[" << *word << "]" << endl;
	    des->errors += 1;
	    return 0;
      }

      if (use_sel == index_component_t::SEL_BIT)
	    elaborate_lval_net_bit_(des, scope, lv);

      if (use_sel == index_component_t::SEL_PART)
	    elaborate_lval_net_part_(des, scope, lv);

      if (use_sel == index_component_t::SEL_IDX_UP ||
          use_sel == index_component_t::SEL_IDX_DO)
	    elaborate_lval_net_idx_(des, scope, lv, use_sel);

      return lv;
}

bool PEIdent::elaborate_lval_net_bit_(Design*des,
				      NetScope*scope,
				      NetAssign_*lv) const
{
      const name_component_t&name_tail = path_.back();
      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.msb != 0);
      ivl_assert(*this, index_tail.lsb == 0);

      NetNet*reg = lv->sig();

	// These are not used, but they need to have a default value.
      ivl_variable_type_t expr_type_tmp = IVL_VT_NO_TYPE;
      bool unsized_flag_tmp = false;
      index_tail.msb->test_width(des, scope, integer_width, integer_width,
			         expr_type_tmp, unsized_flag_tmp);

	// Bit selects have a single select expression. Evaluate the
	// constant value and treat it as a part select with a bit
	// width of 1.
      NetExpr*mux = elab_and_eval(des, scope, index_tail.msb, -1);
      long lsb = 0;

      if (NetEConst*index_con = dynamic_cast<NetEConst*> (mux)) {
	    lsb = index_con->value().as_long();
	    mux = 0;
      }

      if (mux) {
	      // Non-constant bit mux. Correct the mux for the range
	      // of the vector, then set the l-value part select expression.
	    mux = normalize_variable_base(mux, reg->msb(), reg->lsb(), 1, true);

	    lv->set_part(mux, 1);

      } else if (lsb == reg->msb() && lsb == reg->lsb()) {
	      // Constant bit mux that happens to select the only bit
	      // of the l-value. Don't bother with any select at all.

      } else {
	      // Constant bit select that does something useful.
	    long loff = reg->sb_to_idx(lsb);

	    if (loff < 0 || loff >= (long)reg->vector_width()) {
		  cerr << get_fileline() << ": error: bit select "
		       << reg->name() << "[" <<lsb<<"]"
		       << " is out of range." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    lv->set_part(new NetEConst(verinum(loff)), 1);
      }

      return true;
}

bool PEIdent::elaborate_lval_net_part_(Design*des,
				       NetScope*scope,
				       NetAssign_*lv) const
{
	// The range expressions of a part select must be
	// constant. The calculate_parts_ function calculates the
	// values into msb and lsb.
      long msb, lsb;
      bool parts_defined_flag;
      bool flag = calculate_parts_(des, scope, msb, lsb, parts_defined_flag);
      if (!flag)
	    return false;

      ivl_assert(*this, parts_defined_flag);

      NetNet*reg = lv->sig();
      assert(reg);

      if (msb == reg->msb() && lsb == reg->lsb()) {

	      /* Part select covers the entire vector. Simplest case. */

      } else {

	      /* Get the canonical offsets into the vector. */
	    long loff = reg->sb_to_idx(lsb);
	    long moff = reg->sb_to_idx(msb);
	    long wid = moff - loff + 1;

	    if (moff < loff) {
		  cerr << get_fileline() << ": error: part select "
		       << reg->name() << "[" << msb<<":"<<lsb<<"]"
		       << " is reversed." << endl;
		  des->errors += 1;
		  return false;
	    }

	      /* If the part select extends beyond the extremes of the
		 variable, then report an error. Note that loff is
		 converted to normalized form so is relative the
		 variable pins. */

	    if (loff < 0 || moff >= (signed)reg->vector_width()) {
		  cerr << get_fileline() << ": warning: Part select "
		       << reg->name() << "[" << msb<<":"<<lsb<<"]"
		       << " is out of range." << endl;
	    }

	    lv->set_part(new NetEConst(verinum(loff)), wid);
      }

      return true;
}

bool PEIdent::elaborate_lval_net_idx_(Design*des,
				      NetScope*scope,
				      NetAssign_*lv,
				      index_component_t::ctype_t use_sel) const
{
      const name_component_t&name_tail = path_.back();;
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.msb != 0);
      ivl_assert(*this, index_tail.lsb != 0);

      NetNet*reg = lv->sig();
      assert(reg);

      if (reg->type() != NetNet::REG) {
	    cerr << get_fileline() << ": error: " << path_ <<
		  " is not a reg/integer/time in " << scope_path(scope) <<
		  "." << endl;
	    cerr << reg->get_fileline() << ":      : " << path_ <<
		  " is declared here as " << reg->type() << "." << endl;
	    des->errors += 1;
	    return false;
      }

      unsigned long wid;
      calculate_up_do_width_(des, scope, wid);

	// These are not used, but they need to have a default value.
      ivl_variable_type_t expr_type_tmp = IVL_VT_NO_TYPE;
      bool unsized_flag_tmp = false;
      index_tail.msb->test_width(des, scope, integer_width, integer_width,
			         expr_type_tmp, unsized_flag_tmp);

      NetExpr*base = elab_and_eval(des, scope, index_tail.msb, -1);

	// Handle the special case that the base is constant. For this
	// case we can reduce the expression.
      if (NetEConst*base_c = dynamic_cast<NetEConst*> (base)) {
	      // For the undefined case just let the constant pass and
	      // we will handle it in the code generator.
	    if (base_c->value().is_defined()) {
		  long lsv = base_c->value().as_long();
		  long offset = 0;
		  if (((reg->msb() < reg->lsb()) &&
                       use_sel == index_component_t::SEL_IDX_UP) ||
		      ((reg->msb() > reg->lsb()) &&
		       use_sel == index_component_t::SEL_IDX_DO)) {
			offset = -wid + 1;
		  }
		  delete base;
		  long rel_base = reg->sb_to_idx(lsv) + offset;
		    /* If we cover the entire lvalue just skip the select. */
		  if (rel_base == 0 && wid == reg->vector_width()) return true;
		  base = new NetEConst(verinum(rel_base));
		  if (warn_ob_select) {
			if (rel_base < 0) {
			      cerr << get_fileline() << ": warning: " << reg->name();
			      if (reg->array_dimensions() > 0) cerr << "[]";
			      cerr << "[" << lsv;
			      if (use_sel == index_component_t::SEL_IDX_UP) {
				    cerr << "+:";
			      } else {
				    cerr << "-:";
			      }
			      cerr << wid << "] is selecting before vector." << endl;
			}
			if (rel_base + wid > reg->vector_width()) {
			      cerr << get_fileline() << ": warning: " << reg->name();
			      if (reg->array_dimensions() > 0) cerr << "[]";
			      cerr << "[" << lsv;
			      if (use_sel == index_component_t::SEL_IDX_UP) {
				    cerr << "+:";
			      } else {
				    cerr << "-:";
			      }
			      cerr << wid << "] is selecting after vector." << endl;
			}
		  }
	    } else {
		  if (warn_ob_select) {
			cerr << get_fileline() << ": warning: " << reg->name();
			if (reg->array_dimensions() > 0) cerr << "[]";
			cerr << "['bx";
			if (use_sel == index_component_t::SEL_IDX_UP) {
			      cerr << "+:";
			} else {
			      cerr << "-:";
			}
			cerr << wid << "] is always outside vector." << endl;
		  }
	    }
      } else {
	      /* Correct the mux for the range of the vector. */
	    if (use_sel == index_component_t::SEL_IDX_UP) {
		  base = normalize_variable_base(base, reg->msb(), reg->lsb(),
		                                 wid, true);
	    } else {
		    // This is assumed to be a SEL_IDX_DO.
		  base = normalize_variable_base(base, reg->msb(), reg->lsb(),
		                                 wid, false);
	    }
      }

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: Set part select width="
		 << wid << ", base=" << *base << endl;

      lv->set_part(base, wid);

      return true;
}

NetAssign_* PENumber::elaborate_lval(Design*des, NetScope*, bool) const
{
      cerr << get_fileline() << ": error: Constant values not allowed "
	   << "in l-value expressions." << endl;
      des->errors += 1;
      return 0;
}
