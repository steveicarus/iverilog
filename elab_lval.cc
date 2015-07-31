/*
 * Copyright (c) 2000-2014 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012-2013 / Stephen Williams (steve@icarus.com)
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

# include  "PExpr.h"
# include  "PPackage.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "netstruct.h"
# include  "netclass.h"
# include  "netdarray.h"
# include  "netparray.h"
# include  "netvector.h"
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
NetAssign_* PExpr::elaborate_lval(Design*, NetScope*, bool, bool) const
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
				     bool is_cassign,
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

	    NetAssign_*tmp = parms_[idx]->elaborate_lval(des, scope,
							 is_cassign, is_force);

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

NetAssign_*PEIdent::scan_lname_for_nested_members_(Design*des, NetScope*scope,
						   const pform_name_t&cur_path) const
{
      if (cur_path.size() == 1)
	    return 0;

      pform_name_t use_path = cur_path;
      name_component_t tail = use_path.back();
      use_path.pop_back();

      NetNet*       reg = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;
      symbol_search(this, des, scope, use_path, reg, par, eve);

      if (reg == 0) {
	    NetAssign_*tmp = scan_lname_for_nested_members_(des, scope, use_path);
	    if (tmp == 0)
		  return 0;

	    tmp = new NetAssign_(tmp);
	    tmp->set_property(tail.name);
	    return tmp;
      }

      if (reg->struct_type() && reg->struct_type()->packed()) {
	    NetAssign_*tmp = new NetAssign_(reg);
	    elaborate_lval_net_packed_member_(des, scope, tmp, tail);
	    return tmp;
      }
#if 0
      if (reg->struct_type() && reg->struct_type()->packed()) {
	    cerr << get_fileline() << ": sorry: "
		 << "I don't know what to do with packed struct " << use_path
		 << " with member " << tail << "." << endl;
	    return 0;
      }
#endif
      if (reg->struct_type() && !reg->struct_type()->packed()) {
	    cerr << get_fileline() << ": sorry: "
		 << "I don't know what to do with unpacked struct " << use_path
		 << " with member " << tail << "." << endl;
	    return 0;
      }

      if (reg->class_type()) {
	    return elaborate_lval_net_class_member_(des, scope, reg, tail.name);
      }

      return 0;
}

/*
 * Handle the ident as an l-value. This includes bit and part selects
 * of that ident.
 */
NetAssign_* PEIdent::elaborate_lval(Design*des,
				    NetScope*scope,
				    bool is_cassign,
				    bool is_force) const
{
      NetNet*       reg = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;
      perm_string   method_name;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_lval: "
		 << "Elaborate l-value ident expression: " << *this << endl;
      }

	/* Try to detect the special case that we are in a method and
	   the identifier is a member of the class. */
      if (NetAssign_*tmp = elaborate_lval_method_class_member_(des, scope))
	    return tmp;

	/* Normally find the name in the passed scope. But if this is
	   imported from a package, then located the variable from the
	   package scope. */
      NetScope*use_scope = scope;
      if (package_) {
	    use_scope = des->find_package(package_->pscope_name());
	    ivl_assert(*this, use_scope);
      }

      symbol_search(this, des, use_scope, path_, reg, par, eve);

	/* If the signal is not found, check to see if this is a
	   member of a struct. Take the name of the form "a.b.member",
	   remove the member and store it into method_name, and retry
	   the search with "a.b". */
      if (reg == 0 && path_.size() >= 2) {
	    pform_name_t use_path = path_;
	    perm_string tmp_name = peek_tail_name(use_path);
	    use_path.pop_back();
	    symbol_search(this, des, use_scope, use_path, reg, par, eve);

	    if (reg && reg->struct_type()) {
		  method_name = tmp_name;

	    } else if (reg && reg->class_type()) {
		  method_name = tmp_name;

	    } else if (NetAssign_*subl = scan_lname_for_nested_members_(des, use_scope, path_)) {
		  return subl;

	    } else {
		  reg = 0;
	    }
      }

      if (reg == 0) {
	    if (use_scope->type()==NetScope::FUNC
		&& use_scope->func_def()->return_sig()==0
		&& use_scope->basename()==peek_tail_name(path_)) {
		  cerr << get_fileline() << ": error: "
		       << "Cannot assign to " << path_
		       << " because function " << scope_path(use_scope)
		       << " is void." << endl;
	    } else {
		  cerr << get_fileline() << ": error: Could not find variable ``"
		       << path_ << "'' in ``" << scope_path(use_scope) <<
			"''" << endl;
	    }
	    des->errors += 1;
	    return 0;
      }

      ivl_assert(*this, reg);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_lval: "
		 << "Found l-value as reg."
		 << " unpacked_dimensions()=" << reg->unpacked_dimensions() << endl;
      }

	// We are processing the tail of a string of names. For
	// example, the verilog may be "a.b.c", so we are processing
	// "c" at this point. (Note that if method_name is not nil,
	// then this is "a.b.c.method" and "a.b.c" is a struct or class.)
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
      if ((reg->type() != NetNet::REG)
	  && (reg->type() != NetNet::UNRESOLVED_WIRE)
	  && !is_force) {
	    cerr << get_fileline() << ": error: " << path_ <<
		  " is not a valid l-value in " << scope_path(use_scope) <<
		  "." << endl;
	    cerr << reg->get_fileline() << ":      : " << path_ <<
		  " is declared here as " << reg->type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (reg->struct_type() && !method_name.nil()) {
	    NetAssign_*lv = new NetAssign_(reg);
	    name_component_t tmp_name (method_name);
	    elaborate_lval_net_packed_member_(des, use_scope, lv, tmp_name);
	    return lv;
      }

      if (reg->class_type() && !method_name.nil() && gn_system_verilog()) {
	    NetAssign_*lv = elaborate_lval_net_class_member_(des, use_scope, reg, method_name);
	    return lv;
      }

	// Past this point, we should have taken care of the cases
	// where the name is a member/method of a struct/class.
      ivl_assert(*this, method_name.nil());

      bool need_const_idx = is_cassign || is_force || (reg->type()==NetNet::UNRESOLVED_WIRE);

      if (reg->unpacked_dimensions() > 0)
	    return elaborate_lval_net_word_(des, scope, reg, need_const_idx);

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
	    elaborate_lval_net_idx_(des, scope, lv, use_sel, need_const_idx);
	    return lv;
      }


      if (use_sel == index_component_t::SEL_BIT) {
	    if (reg->darray_type()) {
		  NetAssign_*lv = new NetAssign_(reg);
		  elaborate_lval_darray_bit_(des, scope, lv);
		  return lv;
	    } else {
		  NetAssign_*lv = new NetAssign_(reg);
		  elaborate_lval_net_bit_(des, scope, lv, need_const_idx);
		  return lv;
	    }
      }

      ivl_assert(*this, use_sel == index_component_t::SEL_NONE);

      if (reg->type()==NetNet::UNRESOLVED_WIRE && !is_force) {
	    cerr << get_fileline() << ": error: "
		 << path_ << " Unable to assign to unresolved wires."
		 << endl;
	    des->errors += 1;
	    return 0;
      }

	/* No select expressions. */

      NetAssign_*lv = new NetAssign_(reg);

      return lv;
}

NetAssign_* PEIdent::elaborate_lval_method_class_member_(Design*des,
							 NetScope*scope) const
{
      if (!gn_system_verilog())
	    return 0;
      if (scope->parent() == 0)
	    return 0;
      if (path_.size() != 1)
	    return 0;

      const netclass_t*class_type = find_class_containing_scope(*this, scope);
      if (class_type == 0)
	    return 0;

      const name_component_t&name_comp = path_.back();

      perm_string member_name = name_comp.name;
      int pidx = class_type->property_idx_from_name(member_name);
      if (pidx < 0)
	    return 0;

      NetScope*scope_method = find_method_containing_scope(*this, scope);
      ivl_assert(*this, scope_method);

      NetNet*this_net = scope_method->find_signal(perm_string::literal("@"));
      if (this_net == 0) {
	    cerr << get_fileline() << ": internal error: "
		 << "Unable to find 'this' port of " << scope_path(scope_method)
		 << "." << endl;
	    return 0;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_lval_method_class_member_: "
		 << "Ident " << member_name
		 << " is a property of class " << class_type->get_name() << endl;
      }

      NetExpr*canon_index = 0;
      if (! name_comp.index.empty()) {
	    ivl_type_t property_type = class_type->get_prop_type(pidx);

	    if (const netsarray_t* stype = dynamic_cast<const netsarray_t*> (property_type)) {
		  canon_index = make_canonical_index(des, scope, this,
						     name_comp.index, stype, false);

	    } else {
		  cerr << get_fileline() << ": error: "
		       << "Index expressions don't apply to this type of property." << endl;
		  des->errors += 1;
	    }
      }

	// Detect assignment to constant properties. Note that the
	// initializer constructor MAY assign to constant properties,
	// as this is how the property gets its value.
      property_qualifier_t qual = class_type->get_prop_qual(pidx);
      if (qual.test_const()) {
	    if (class_type->get_prop_initialized(pidx)) {
		  cerr << get_fileline() << ": error: "
		       << "Property " << class_type->get_prop_name(pidx)
		       << " is constant in this method."
		       << " (scope=" << scope_path(scope) << ")" << endl;
		  des->errors += 1;

	    } else if (scope->basename()!="new" && scope->basename()!="new@") {
		  cerr << get_fileline() << ": error: "
		       << "Property " << class_type->get_prop_name(pidx)
		       << " is constant in this method."
		       << " (scope=" << scope_path(scope) << ")" << endl;
		  des->errors += 1;

	    } else {

		    // Mark this property as initialized. This is used
		    // to know that we have initialized the constant
		    // object so the next assignment will be marked as
		    // illegal.
		  class_type->set_prop_initialized(pidx);

		  if (debug_elaborate) {
			cerr << get_fileline() << ": PEIdent::elaborate_lval_method_class_member_: "
			     << "Found initializers for property " << class_type->get_prop_name(pidx) << endl;
		  }
	    }
      }

      ivl_type_t tmp_type = class_type->get_prop_type(pidx);
      if (const netuarray_t*tmp_ua = dynamic_cast<const netuarray_t*>(tmp_type)) {

	    const std::vector<netrange_t>&dims = tmp_ua->static_dimensions();

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_lval_method_class_member_: "
		       << "Property " << class_type->get_prop_name(pidx)
		       << " has " << dims.size() << " dimensions, "
		       << " got " << name_comp.index.size() << " indices." << endl;
		  if (canon_index) {
			cerr << get_fileline() << ": PEIdent::elaborate_lval_method_class_member_: "
			     << "Canonical index is:" << *canon_index << endl;
		  };
	    }

	    if (dims.size() != name_comp.index.size()) {
		  cerr << get_fileline() << ": error: "
		       << "Got " << name_comp.index.size() << " indices, "
		       << "expecting " << dims.size()
		       << " to index the property " << class_type->get_prop_name(pidx) << "." << endl;
		  des->errors += 1;
	    }
      }

      NetAssign_*this_lval = new NetAssign_(this_net);
      this_lval->set_property(member_name);
      if (canon_index) this_lval->set_word(canon_index);

      return this_lval;
}

NetAssign_* PEIdent::elaborate_lval_net_word_(Design*des,
					      NetScope*scope,
					      NetNet*reg,
					      bool need_const_idx) const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_lval_net_word_: "
		 << "Handle as n-dimensional array." << endl;
      }

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
      indices_flags flags;
      indices_to_expressions(des, scope, this,
			     name_tail.index, reg->unpacked_dimensions(),
			     false,
			     flags,
			     unpacked_indices,
			     unpacked_indices_const);

      NetExpr*canon_index = 0;
      if (flags.invalid) {
	    // Nothing to do.

      } else if (flags.undefined) {
	    cerr << get_fileline() << ": warning: "
		 << "ignoring undefined l-value array access "
		 << reg->name() << as_indices(unpacked_indices)
		 << "." << endl;

      } else if (flags.variable) {
	    if (need_const_idx) {
		  cerr << get_fileline() << ": error: array '" << reg->name()
		       << "' index must be a constant in this context." << endl;
		  des->errors += 1;
		  return 0;
	    }
	    ivl_assert(*this, unpacked_indices.size() == reg->unpacked_dimensions());
	    canon_index = normalize_variable_unpacked(reg, unpacked_indices);

      } else {
	    ivl_assert(*this, unpacked_indices_const.size() == reg->unpacked_dimensions());
	    canon_index = normalize_variable_unpacked(reg, unpacked_indices_const);

	    if (canon_index == 0) {
		  cerr << get_fileline() << ": warning: "
		       << "ignoring out of bounds l-value array access "
		       << reg->name() << as_indices(unpacked_indices_const)
		       << "." << endl;
	    }
      }

	// Ensure invalid array accesses are ignored.
      if (canon_index == 0)
	    canon_index = new NetEConst(verinum(verinum::Vx));
      canon_index->set_line(*this);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_lval_net_word_: "
		 << "canon_index=" << *canon_index << endl;
      }

      if (reg->type()==NetNet::UNRESOLVED_WIRE) {
	    cerr << get_fileline() << ": error: "
		 << "Unable to assign words of unresolved wire array." << endl;
	    des->errors += 1;
	    return 0;
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
	         << as_indices(unpacked_indices) << endl;
	    des->errors += 1;
	    return 0;
      }

      if (use_sel == index_component_t::SEL_BIT)
	    elaborate_lval_net_bit_(des, scope, lv, need_const_idx);

      if (use_sel == index_component_t::SEL_PART)
	    elaborate_lval_net_part_(des, scope, lv);

      if (use_sel == index_component_t::SEL_IDX_UP ||
          use_sel == index_component_t::SEL_IDX_DO)
	    elaborate_lval_net_idx_(des, scope, lv, use_sel, need_const_idx);

      return lv;
}

bool PEIdent::elaborate_lval_net_bit_(Design*des,
				      NetScope*scope,
				      NetAssign_*lv,
				      bool need_const_idx) const
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
      ivl_assert(*this, reg);

	// Bit selects have a single select expression. Evaluate the
	// constant value and treat it as a part select with a bit
	// width of 1.
      NetExpr*mux = elab_and_eval(des, scope, index_tail.msb, -1);
      long lsb = 0;

      if (NetEConst*index_con = dynamic_cast<NetEConst*> (mux)) {
	      // The index has a constant defined value.
	    if (index_con->value().is_defined()) {
		  lsb = index_con->value().as_long();
		  mux = 0;
	      // The index is undefined and this is a packed array.
	    } else if (prefix_indices.size()+2 <= reg->packed_dims().size()) {
		  long loff;
		  unsigned long lwid;
		  bool rcl = reg->sb_to_slice(prefix_indices, lsb, loff, lwid);
		  ivl_assert(*this, rcl);
		  cerr << get_fileline() << ": warning: L-value packed array "
		       << "select of " << reg->name();
		  if (reg->unpacked_dimensions() > 0) cerr << "[]";
		  cerr << " has an undefined index." << endl;

		  lv->set_part(new NetEConst(verinum(verinum::Vx)), lwid);
		  return true;
	      // The index is undefined and this is a bit select.
	    } else {
		  cerr << get_fileline() << ": warning: L-value bit select of "
		       << reg->name();
		  if (reg->unpacked_dimensions() > 0) cerr << "[]";
		  cerr << " has an undefined index." << endl;

		  lv->set_part(new NetEConst(verinum(verinum::Vx)), 1);
		  return true;
	    }
      }

      if (debug_elaborate && (reg->type()==NetNet::UNRESOLVED_WIRE)) {
	    cerr << get_fileline() << ": PEIdent::elaborate_lval_net_bit_: "
		 << "Try to assign bits of unresolved wire."
		 << endl;
      }

	// Notice that we might be assigning to an unresolved wire. This
	// can happen if we are actually assigning to a variable that
	// has a partial continuous assignment to it. If that is the
	// case, then the bit select must be constant.
      ivl_assert(*this, need_const_idx || (reg->type()!=NetNet::UNRESOLVED_WIRE));


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

		  if (reg->type()==NetNet::UNRESOLVED_WIRE) {
			bool rct = reg->test_and_set_part_driver(loff+lwid-1, loff);
			if (rct) {
			      cerr << get_fileline() << ": error: "
				   << "These bits are already driven." << endl;
			      des->errors += 1;
			}
		  }

		  lv->set_part(new NetEConst(verinum(loff)), lwid);

	    } else {
		  ivl_assert(*this, reg->type()!=NetNet::UNRESOLVED_WIRE);
		  unsigned long lwid;
		  mux = normalize_variable_slice_base(prefix_indices, mux,
						      reg, lwid);
		  lv->set_part(mux, lwid);
	    }

      } else if (reg->data_type() == IVL_VT_STRING) {
	    ivl_assert(*this, reg->type()!=NetNet::UNRESOLVED_WIRE);
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
	    ivl_assert(*this, reg->type()!=NetNet::UNRESOLVED_WIRE);

	      // Non-constant bit mux. Correct the mux for the range
	      // of the vector, then set the l-value part select
	      // expression.
	    if (need_const_idx) {
		  cerr << get_fileline() << ": error: '" << reg->name()
		       << "' bit select must be a constant in this context."
		       << endl;
		  des->errors += 1;
		  return false;
	    }
	    mux = normalize_variable_bit_base(prefix_indices, mux, reg);
	    lv->set_part(mux, 1);

      } else if (reg->vector_width() == 1 && reg->sb_is_valid(prefix_indices,lsb)) {
	      // Constant bit mux that happens to select the only bit
	      // of the l-value. Don't bother with any select at all.

	      // NOTE: Don't know what to do about unresolved wires
	      // here, but they are probably wrong.
	    ivl_assert(*this, reg->type()!=NetNet::UNRESOLVED_WIRE);

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

	    if (reg->type()==NetNet::UNRESOLVED_WIRE) {
		  bool rct = reg->test_and_set_part_driver(loff, loff);
		  if (rct) {
			cerr << get_fileline() << ": error: "
			     << "Bit " << loff << " is already driven." << endl;
			des->errors += 1;
		  }
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

      if (lv->sig()->type()==NetNet::UNRESOLVED_WIRE) {
	    cerr << get_fileline() << ": error: "
		 << path_ << " Unable to darray word select unresolved wires."
		 << endl;
	    des->errors += 1;
	    return false;
      }

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
      if (!flag) return false;

      NetNet*reg = lv->sig();
      ivl_assert(*this, reg);

      if (! parts_defined_flag) {
	    cerr << get_fileline() << ": warning: L-value part select of "
	         << reg->name();
	    if (reg->unpacked_dimensions() > 0) cerr << "[]";
	    cerr << " has an undefined index." << endl;
	      // Use a width of two here so we can distinguish between an
	      // undefined bit or part select.
	    lv->set_part(new NetEConst(verinum(verinum::Vx)), 2);
	    return true;
      }

      if (reg->type()==NetNet::UNRESOLVED_WIRE) {
	    bool rct = reg->test_and_set_part_driver(msb, lsb);
	    if (rct) {
		  cerr << get_fileline() << ": error: "
		       << path_ << "Part select is double-driving unresolved wire."
		       << endl;
		  des->errors += 1;
		  return false;
	    }
      }

      const vector<netrange_t>&packed = reg->packed_dims();

      long loff, moff;
      long wid;
      if (prefix_indices.size()+1 < packed.size()) {
	      // If there are fewer indices then there are packed
	      // dimensions, then this is a range of slices. Calculate
	      // it into a big slice.
	    bool lrc;
	    unsigned long tmp_lwid, tmp_mwid;
	    lrc = reg->sb_to_slice(prefix_indices,lsb, loff, tmp_lwid);
	    ivl_assert(*this, lrc);
	    lrc = reg->sb_to_slice(prefix_indices,msb, moff, tmp_mwid);
	    ivl_assert(*this, lrc);

	    if (loff < moff) {
		  moff = moff + tmp_mwid - 1;
	    } else {
		  long ltmp = moff;
		  moff = loff + tmp_lwid - 1;
		  loff = ltmp;
	    }
	    wid = moff - loff + 1;

      } else {
	    loff = reg->sb_to_idx(prefix_indices,lsb);
	    moff = reg->sb_to_idx(prefix_indices,msb);
	    wid = moff - loff + 1;

	    if (moff < loff) {
		  cerr << get_fileline() << ": error: part select "
		       << reg->name() << "[" << msb<<":"<<lsb<<"]"
		       << " is reversed." << endl;
		  des->errors += 1;
		  return false;
	    }
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
				      index_component_t::ctype_t use_sel,
				      bool need_const_idx) const
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
		  const vector<netrange_t>&packed = reg->packed_dims();
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
		  cerr << get_fileline() << ": warning: L-value indexed part "
		       << "select of " << reg->name();
		  if (reg->unpacked_dimensions() > 0) cerr << "[]";
		  cerr << " has an undefined base." << endl;
	    }
      } else {
	    if (need_const_idx) {
		  cerr << get_fileline() << ": error: '" << reg->name()
		       << "' base index must be a constant in this context."
		       << endl;
		  des->errors += 1;
		  return false;
	    }
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

NetAssign_* PEIdent::elaborate_lval_net_class_member_(Design*des, NetScope*scope,
				    NetNet*sig, const perm_string&method_name) const
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": elaborate_lval_net_class_member_: "
		 << "l-value is property " << method_name
		 << " of " << sig->name() << "." << endl;
      }

      const netclass_t*class_type = sig->class_type();
      ivl_assert(*this, class_type);

	/* Make sure the property is really present in the class. If
	   not, then generate an error message and return an error. */
      int pidx = class_type->property_idx_from_name(method_name);
      if (pidx < 0) {
	    cerr << get_fileline() << ": error: Class " << class_type->get_name()
		 << " does not have a property " << method_name << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      property_qualifier_t qual = class_type->get_prop_qual(pidx);
      if (qual.test_local() && ! class_type->test_scope_is_method(scope)) {
	    cerr << get_fileline() << ": error: "
		 << "Local property " << class_type->get_prop_name(pidx)
		 << " is not accessible (l-value) in this context."
		 << " (scope=" << scope_path(scope) << ")" << endl;
	    des->errors += 1;

      } else if (qual.test_static()) {

	      // Special case: this is a static property. Ignore the
	      // "this" sig and use the property itself, which is not
	      // part of the sig, as the l-value.
	    NetNet*psig = class_type->find_static_property(method_name);
	    ivl_assert(*this, psig);

	    NetAssign_*lv = new NetAssign_(psig);
	    return lv;

      } else if (qual.test_const()) {
	    cerr << get_fileline() << ": error: "
		 << "Property " << class_type->get_prop_name(pidx)
		 << " is constant in this context." << endl;
	    des->errors += 1;
      }

      NetAssign_*lv = new NetAssign_(sig);
      lv->set_property(method_name);

      ivl_type_t ptype = class_type->get_prop_type(pidx);
      const netdarray_t*mtype = dynamic_cast<const netdarray_t*> (ptype);
      if (mtype) {
	    const name_component_t&name_tail = path_.back();
	    if (! name_tail.index.empty()) {
		  cerr << get_fileline() << ": sorry: "
		       << "Array index of array properties not supported."
		       << endl;
		  des->errors += 1;
	    }
      }

      return lv;
}


bool PEIdent::elaborate_lval_net_packed_member_(Design*des, NetScope*scope,
						NetAssign_*lv,
						const name_component_t&member_comp) const
{
      const perm_string&member_name = member_comp.name;
      NetNet*reg = lv->sig();
      ivl_assert(*this, reg);

      const netstruct_t*struct_type = reg->struct_type();
      ivl_assert(*this, struct_type);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: elaborate lval packed member: "
		 << "path_=" << path_ << endl;
      }

      if (! struct_type->packed()) {
	    cerr << get_fileline() << ": sorry: Only packed structures "
		 << "are supported in l-value." << endl;
	    des->errors += 1;
	    return false;
      }

	// Shouldn't be seeing unpacked arrays of packed structs...
      ivl_assert(*this, reg->unpacked_dimensions() == 0);

	// This is a packed member, so the name is of the form
	// "a.b[...].c[...]" which means that the path_ must have at
	// least 2 components. We are processing "c[...]" at that
	// point (otherwise known as member_name) so we'll save a
	// reference to it in name_tail. We are also processing "b[]"
	// so save that as name_base.

      ivl_assert(*this, path_.size() >= 2);

      pform_name_t::const_reverse_iterator name_idx = path_.rbegin();
      ivl_assert(*this, name_idx->name == member_name);
      const name_component_t&name_tail = *name_idx;
      ++ name_idx;
      const name_component_t&name_base = *name_idx;

	// Calculate the offset within the packed structure of the
	// member, and any indices. We will add in the offset of the
	// struct into the packed array later. Note that this works
	// for packed unions as well (although the offset will be 0
	// for union members).
      unsigned long off;
      const netstruct_t::member_t* member = struct_type->packed_member(member_name, off);

      if (member == 0) {
	    cerr << get_fileline() << ": error: Member " << member_name
		 << " is not a member of variable " << reg->name() << endl;
	    des->errors += 1;
	    return false;
      }

      unsigned long use_width = member->net_type->packed_width();

	// Get the index component type. At this point, we only
	// support bit select or none.
      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (!name_tail.index.empty())
	    use_sel = name_tail.index.back().sel;

      ivl_assert(*this, use_sel == index_component_t::SEL_NONE || use_sel == index_component_t::SEL_BIT);

      if (! name_tail.index.empty()) {

	      // If there are index expressions in this l-value
	      // expression, then the implicit assumption is that the
	      // member is a vector type with packed dimensions. For
	      // example, if the l-value expression is "foo.member[1][2]",
	      // then the member should be something like:
	      //    ... logic [h:l][m:n] foo;
	      // Get the dimensions from the netvector_t that this implies.
	    const netvector_t*mem_vec = dynamic_cast<const netvector_t*>(member->net_type);
	    ivl_assert(*this, mem_vec);
	    const vector<netrange_t>&mem_packed_dims = mem_vec->packed_dims();

	    if (name_tail.index.size() > mem_packed_dims.size()) {
		  cerr << get_fileline() << ": error: "
		       << "Too many index expressions for member." << endl;
		  des->errors += 1;
		  return false;
	    }

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
	    prefix_to_slice(mem_packed_dims, prefix_indices, tmp, loff, lwid);

	    off += loff;
	    use_width = lwid;
      }

	// The dimensions in the expression must match the packed
	// dimensions that are declared for the variable. For example,
	// if foo is a packed array of struct, then this expression
	// must be "b[n][m]" with the right number of dimensions to
	// match the declaration of "b".
	// Note that one of the packed dimensions is the packed struct
	// itself.
      ivl_assert(*this, name_base.index.size()+1 == reg->packed_dimensions());

	// Generate an expression that takes the input array of
	// expressions and generates a canonical offset into the
	// packed array.
      NetExpr*packed_base = 0;
      if (reg->packed_dimensions() > 1) {
	    list<index_component_t>tmp_index = name_base.index;
	    index_component_t member_select;
	    member_select.sel = index_component_t::SEL_BIT;
	    member_select.msb = new PENumber(new verinum(off));
	    tmp_index.push_back(member_select);
	    packed_base = collapse_array_indices(des, scope, reg, tmp_index);
      }

      long tmp;
      if (packed_base && eval_as_long(tmp, packed_base)) {
	    off = tmp;
	    delete packed_base;
	    packed_base = 0;
      }

      if (reg->type()==NetNet::UNRESOLVED_WIRE) {
	    cerr << get_fileline() << ": error: "
		 << path_ << " Unable to member-select unresolved wires."
		 << endl;
	    des->errors += 1;
	    return false;
      }

      if (packed_base == 0) {
	    lv->set_part(new NetEConst(verinum(off)), use_width);
	    return true;
      }

	// Oops, packed_base is not fully evaluated, so I don't know
	// yet what to do with it.
      cerr << get_fileline() << ": internal error: "
	   << "I don't know how to handle this index expression? " << *packed_base << endl;
      ivl_assert(*this, 0);
      return false;
}

NetAssign_* PENumber::elaborate_lval(Design*des, NetScope*, bool, bool) const
{
      cerr << get_fileline() << ": error: Constant values not allowed "
	   << "in l-value expressions." << endl;
      des->errors += 1;
      return 0;
}
