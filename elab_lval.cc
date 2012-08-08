/*
 * Copyright (c) 2000-2012 Stephen Williams (steve@icarus.com)
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
# include  "netstruct.h"
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
 * rather than create a NetAssign_ for each item in the concatenation,
 * elaboration makes a single NetAssign_ and connects it up properly.
 */


/*
 * The default interpretation of an l-value to a procedural assignment
 * is to try to make a net elaboration, and see if the result is
 * suitable for assignment.
 */
NetAssign_* PExpr::elaborate_lval(Design*, NetScope*, bool) const
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

      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {

	    if (parms_[idx] == 0) {
		  cerr << get_fileline() << ": error: Empty expressions "
		       << "not allowed in concatenations." << endl;
		  des->errors += 1;
		  continue;
	    }

	    NetAssign_*tmp = parms_[idx]->elaborate_lval(des, scope, is_force);

	      /* If the l-value doesn't elaborate, the error was
		 already detected and printed. We just skip it and let
		 the compiler catch more errors. */
	    if (tmp == 0) continue;

	    if (tmp->expr_type() == IVL_VT_REAL) {
		  cerr << parms_[idx]->get_fileline() << ": error: "
		       << "concatenation operand can not be real: "
		       << *parms_[idx] << endl;
		  des->errors += 1;
		  continue;
	    }

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
      perm_string   method_name;

      symbol_search(this, des, scope, path_, reg, par, eve);

	/* If the signal is not found, check to see if this is a
	   member of a struct. Take the name of the form "a.b.member",
	   remove the member and store it into method_name, and retry
	   the search with "a.b". */
      if (reg == 0 && path_.size() >= 2) {
	    pform_name_t use_path = path_;
	    method_name = peek_tail_name(use_path);
	    use_path.pop_back();
	    symbol_search(this, des, scope, use_path, reg, par, eve);

	    if (reg && reg->struct_type() == 0) {
		  method_name = perm_string();
		  reg = 0;
	    }
      }

      if (reg == 0) {
	    cerr << get_fileline() << ": error: Could not find variable ``"
		 << path_ << "'' in ``" << scope_path(scope) <<
		  "''" << endl;

	    des->errors += 1;
	    return 0;
      }

      ivl_assert(*this, reg);
	// We are processing the tail of a string of names. For
	// example, the verilog may be "a.b.c", so we are processing
	// "c" at this point.
      const name_component_t&name_tail = path_.back();

	// Use the last index to determine what kind of select
	// (bit/part/etc) we are processing. For example, the verilog
	// may be "a.b.c[1][2][<index>]". All but the last index must
	// be simple expressions, only the <index> may be a part
	// select etc., so look at it to determine how we will be
	// proceeding.
      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (!name_tail.index.empty())
	    use_sel = name_tail.index.back().sel;

	// Special case: The l-value is an entire memory, or array
	// slice. This is, in fact, an error in l-values. Detect the
	// situation by noting if the index count is less than the
	// array dimensions (unpacked).
      if (reg->unpacked_dimensions() > name_tail.index.size()) {
	    cerr << get_fileline() << ": error: Cannot assign to array "
		 << path_ << ". Did you forget a word index?" << endl;
	    des->errors += 1;
	    return 0;
      }

	/* Get the signal referenced by the identifier, and make sure
	   it is a register. Wires are not allowed in this context,
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

      if (reg->struct_type() && !method_name.nil()) {
	    NetAssign_*lv = new NetAssign_(reg);
	    elaborate_lval_net_packed_member_(des, scope, lv, method_name);
	    return lv;
      }

      if (reg->unpacked_dimensions() > 0)
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
	    if (reg->darray_type()) {
		  NetAssign_*lv = new NetAssign_(reg);
		  elaborate_lval_darray_bit_(des, scope, lv);
		  return lv;
	    } else {
		  NetAssign_*lv = new NetAssign_(reg);
		  elaborate_lval_net_bit_(des, scope, lv);
		  return lv;
	    }
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

      if (name_tail.index.size() < reg->unpacked_dimensions()) {
	    cerr << get_fileline() << ": error: Array " << reg->name()
		 << " needs " << reg->unpacked_dimensions() << " indices,"
		 << " but got only " << name_tail.index.size() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

	// Make sure there are enough indices to address an array element.
      const index_component_t&index_head = name_tail.index.front();
      if (index_head.sel == index_component_t::SEL_PART) {
	    cerr << get_fileline() << ": error: cannot perform a part "
	         << "select on array " << reg->name() << "." << endl;
	    des->errors += 1;
	    return 0;
      }


	// Evaluate all the index expressions into an
	// "unpacked_indices" array.
      list<NetExpr*>unpacked_indices;
      list<long> unpacked_indices_const;
      bool flag = indices_to_expressions(des, scope, this,
					 name_tail.index, reg->unpacked_dimensions(),
					 false,
					 unpacked_indices,
					 unpacked_indices_const);

      NetExpr*canon_index = 0;
      if (flag) {
	    ivl_assert(*this, unpacked_indices_const.size() == reg->unpacked_dimensions());
	    canon_index = normalize_variable_unpacked(reg, unpacked_indices_const);
	    if (canon_index == 0) {
		  cerr << get_fileline() << ": warning: "
		       << "ignoring out of bounds l-value array access " << reg->name();
		  for (list<long>::const_iterator cur = unpacked_indices_const.begin()
			     ; cur != unpacked_indices_const.end() ; ++cur) {
			cerr << "[" << *cur << "]";
		  }
		  cerr << "." << endl;
	    }
      } else {
	    ivl_assert(*this, unpacked_indices.size() == reg->unpacked_dimensions());
	    canon_index = normalize_variable_unpacked(reg, unpacked_indices);
      }


      NetAssign_*lv = new NetAssign_(reg);
      lv->set_word(canon_index);

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: Set array word=" << *canon_index << endl;


	/* An array word may also have part selects applied to them. */

      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (name_tail.index.size() > reg->unpacked_dimensions())
	    use_sel = name_tail.index.back().sel;

      if (reg->get_scalar() &&
          use_sel != index_component_t::SEL_NONE) {
	    cerr << get_fileline() << ": error: can not select part of ";
	    if (reg->data_type() == IVL_VT_REAL) cerr << "real";
	    else cerr << "scalar";
	    cerr << " array word: " << reg->name()
	         << "[" << *canon_index << "]" << endl;
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
      list<long>prefix_indices;
      bool rc = calculate_packed_indices_(des, scope, lv->sig(), prefix_indices);
      if (!rc) return false;

      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.msb != 0);
      ivl_assert(*this, index_tail.lsb == 0);

      NetNet*reg = lv->sig();

	// Bit selects have a single select expression. Evaluate the
	// constant value and treat it as a part select with a bit
	// width of 1.
      NetExpr*mux = elab_and_eval(des, scope, index_tail.msb, -1);
      long lsb = 0;

      if (NetEConst*index_con = dynamic_cast<NetEConst*> (mux)) {
	    lsb = index_con->value().as_long();
	    mux = 0;
      }


      if (prefix_indices.size()+2 <= reg->packed_dims().size()) {
	      // Special case: this is a slice of a multi-dimensional
	      // packed array. For example:
	      //   reg [3:0][7:0] x;
	      //   x[2] = ...
	      // This shows up as the prefix_indices being too short
	      // for the packed dimensions of the vector. What we do
	      // here is convert to a "slice" of the vector.
	    if (mux == 0) {
		  long loff;
		  unsigned long lwid;
		  bool rcl = reg->sb_to_slice(prefix_indices, lsb, loff, lwid);
		  ivl_assert(*this, rcl);

		  lv->set_part(new NetEConst(verinum(loff)), lwid);
	    } else {
		  unsigned long lwid;
		  mux = normalize_variable_slice_base(prefix_indices, mux,
						      reg, lwid);
		  lv->set_part(mux, lwid);
	    }

      } else if (reg->data_type() == IVL_VT_STRING) {
	      // Special case: This is a select of a string
	      // variable. The target of the assignment is a character
	      // select of a string. Force the r-value to be an 8bit
	      // vector and set the "part" to be the character select
	      // expression. The code generator knows what to do with
	      // this.
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: "
		       << "Bit select of string becomes character select." << endl;
	    }
	    if (mux)
		  lv->set_part(mux, 8);
	    else
		  lv->set_part(new NetEConst(verinum(lsb)), 8);

      } else if (mux) {
	      // Non-constant bit mux. Correct the mux for the range
	      // of the vector, then set the l-value part select
	      // expression.
	    mux = normalize_variable_bit_base(prefix_indices, mux, reg);
	    lv->set_part(mux, 1);

      } else if (reg->vector_width() == 1 && reg->sb_is_valid(prefix_indices,lsb)) {
	      // Constant bit mux that happens to select the only bit
	      // of the l-value. Don't bother with any select at all.

      } else {
	      // Constant bit select that does something useful.
	    long loff = reg->sb_to_idx(prefix_indices,lsb);

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

bool PEIdent::elaborate_lval_darray_bit_(Design*des, NetScope*scope, NetAssign_*lv)const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

	// For now, only support single-dimension dynamic arrays.
      ivl_assert(*this, name_tail.index.size() == 1);

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.msb != 0);
      ivl_assert(*this, index_tail.lsb == 0);

	// Evaluate the select expression...
      NetExpr*mux = elab_and_eval(des, scope, index_tail.msb, -1);

      lv->set_word(mux);

      return true;
}

bool PEIdent::elaborate_lval_net_part_(Design*des,
				       NetScope*scope,
				       NetAssign_*lv) const
{
      list<long> prefix_indices;
      bool rc = calculate_packed_indices_(des, scope, lv->sig(), prefix_indices);
      ivl_assert(*this, rc);

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
      ivl_assert(*this, reg);

      const list<netrange_t>&packed = reg->packed_dims();

	// Part selects cannot select slices. So there must be enough
	// prefix_indices to get all the way to the final dimension.
      if (prefix_indices.size()+1 < packed.size()) {
	    cerr << get_fileline() << ": error: Cannot select a range "
		 << "of slices from a packed array." << endl;
	    des->errors += 1;
	    return false;
      }

      long loff = reg->sb_to_idx(prefix_indices,lsb);
      long moff = reg->sb_to_idx(prefix_indices,msb);
      long wid = moff - loff + 1;

      if (moff < loff) {
	    cerr << get_fileline() << ": error: part select "
		 << reg->name() << "[" << msb<<":"<<lsb<<"]"
		 << " is reversed." << endl;
	    des->errors += 1;
	    return false;
      }

	// Special case: The range winds up selecting the entire
	// vector. Treat this as no part select at all.
      if (loff == 0 && moff == (long)(reg->vector_width()-1)) {
	    return true;
      }

	/* If the part select extends beyond the extremes of the
	   variable, then report an error. Note that loff is
	   converted to normalized form so is relative the
	   variable pins. */

      if (loff < 0 || moff >= (long)reg->vector_width()) {
	    cerr << get_fileline() << ": warning: Part select "
		 << reg->name() << "[" << msb<<":"<<lsb<<"]"
		 << " is out of range." << endl;
      }

      lv->set_part(new NetEConst(verinum(loff)), wid);

      return true;
}

bool PEIdent::elaborate_lval_net_idx_(Design*des,
				      NetScope*scope,
				      NetAssign_*lv,
				      index_component_t::ctype_t use_sel) const
{
      list<long>prefix_indices;
      bool rc = calculate_packed_indices_(des, scope, lv->sig(), prefix_indices);
      ivl_assert(*this, rc);

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

      NetExpr*base = elab_and_eval(des, scope, index_tail.msb, -1);
      ivl_select_type_t sel_type = IVL_SEL_OTHER;

	// Handle the special case that the base is constant. For this
	// case we can reduce the expression.
      if (NetEConst*base_c = dynamic_cast<NetEConst*> (base)) {
	      // For the undefined case just let the constant pass and
	      // we will handle it in the code generator.
	    if (base_c->value().is_defined()) {
		  long lsv = base_c->value().as_long();
		  long offset = 0;
		    // Get the signal range.
		  const list<netrange_t>&packed = reg->packed_dims();
		  ivl_assert(*this, packed.size() == prefix_indices.size()+1);

		    // We want the last range, which is where we work.
		  const netrange_t&rng = packed.back();
		  if (((rng.get_msb() < rng.get_lsb()) &&
                       use_sel == index_component_t::SEL_IDX_UP) ||
		      ((rng.get_msb() > rng.get_lsb()) &&
		       use_sel == index_component_t::SEL_IDX_DO)) {
			offset = -wid + 1;
		  }
		  delete base;
		  long rel_base = reg->sb_to_idx(prefix_indices,lsv) + offset;
		    /* If we cover the entire lvalue just skip the select. */
		  if (rel_base == 0 && wid == reg->vector_width()) return true;
		  base = new NetEConst(verinum(rel_base));
		  if (warn_ob_select) {
			if (rel_base < 0) {
			      cerr << get_fileline() << ": warning: " << reg->name();
			      if (reg->unpacked_dimensions() > 0) cerr << "[]";
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
			      if (reg->unpacked_dimensions() > 0) cerr << "[]";
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
			if (reg->unpacked_dimensions() > 0) cerr << "[]";
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
	    ivl_assert(*this, prefix_indices.size()+1 == reg->packed_dims().size());
	      /* Correct the mux for the range of the vector. */
	    if (use_sel == index_component_t::SEL_IDX_UP) {
		  base = normalize_variable_part_base(prefix_indices, base,
						      reg, wid, true);
		  sel_type = IVL_SEL_IDX_UP;
	    } else {
		    // This is assumed to be a SEL_IDX_DO.
		  base = normalize_variable_part_base(prefix_indices, base,
						      reg, wid, false);
		  sel_type = IVL_SEL_IDX_DOWN;
	    }
      }

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: Set part select width="
		 << wid << ", base=" << *base << endl;

      lv->set_part(base, wid, sel_type);

      return true;
}

bool PEIdent::elaborate_lval_net_packed_member_(Design*des, NetScope*scope,
						NetAssign_*lv,
						const perm_string&member_name) const
{
      NetNet*reg = lv->sig();
      ivl_assert(*this, reg);

      netstruct_t*struct_type = reg->struct_type();
      ivl_assert(*this, struct_type);

      if (! struct_type->packed()) {
	    cerr << get_fileline() << ": sorry: Only packed structures "
		 << "are supported in l-value." << endl;
	    des->errors += 1;
	    return false;
      }

      unsigned long off;
      const netstruct_t::member_t* member = struct_type->packed_member(member_name, off);

      if (member == 0) {
	    cerr << get_fileline() << ": error: Member " << member_name
		 << " is not a member of variable " << reg->name() << endl;
	    des->errors += 1;
	    return false;
      }

      unsigned long use_width = member->width();

	// We are processing the tail of a string of names. For
	// example, the verilog may be "a.b.c", so we are processing
	// "c" at this point. Of course, "c" is the name of the member
	// we are working on and "a.b" is the name of reg.
      const name_component_t&name_tail = path_.back();

      if (name_tail.index.size() > member->packed_dims.size()) {
	    cerr << get_fileline() << ": error: Too make index expressions for member." << endl;
	    des->errors += 1;
	    return false;
      }

	// Get the index component type. At this point, we only
	// support bit select or none.
      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (!name_tail.index.empty())
	    use_sel = name_tail.index.back().sel;

      ivl_assert(*this, use_sel == index_component_t::SEL_NONE || use_sel == index_component_t::SEL_BIT);

      if (! name_tail.index.empty()) {
	      // Evaluate all but the last index expression, into prefix_indices.
	    list<long>prefix_indices;
	    bool rc = evaluate_index_prefix(des, scope, prefix_indices, name_tail.index);
	    ivl_assert(*this, rc);

	      // Evaluate the last index expression into a constant long.
	    NetExpr*texpr = elab_and_eval(des, scope, name_tail.index.back().msb, -1, true);
	    long tmp;
	    if (texpr == 0 || !eval_as_long(tmp, texpr)) {
		  cerr << get_fileline() << ": error: "
			"Array index expressions must be constant here." << endl;
		  des->errors += 1;
		  return false;
	    }

	    delete texpr;

	      // Now use the prefix_to_slice function to calculate the
	      // offset and width of the addressed slice of the member.
	    long loff;
	    unsigned long lwid;
	    prefix_to_slice(member->packed_dims, prefix_indices, tmp, loff, lwid);

	    off += loff;
	    use_width = lwid;
      }

      lv->set_part(new NetEConst(verinum(off)), use_width);
      return true;
}

NetAssign_* PENumber::elaborate_lval(Design*des, NetScope*, bool) const
{
      cerr << get_fileline() << ": error: Constant values not allowed "
	   << "in l-value expressions." << endl;
      des->errors += 1;
      return 0;
}
