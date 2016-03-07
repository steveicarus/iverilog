/*
 * Copyright (c) 1999-2014 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012 / Stephen Williams (steve@icarus.com)
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
# include  "netlist.h"
# include  "netmisc.h"
# include  "netstruct.h"
# include  "netvector.h"
# include  "compiler.h"

# include  <cstdlib>
# include  <cstring>
# include  <iostream>
# include  "ivl_assert.h"

/*
 * The concatenation is also OK an an l-value. This method elaborates
 * it as a structural l-value. The return values is the *input* net of
 * the l-value, which may feed via part selects to the final
 * destination. The caller can connect gate outputs to this signal to
 * make the l-value connections.
 */
NetNet* PEConcat::elaborate_lnet_common_(Design*des, NetScope*scope,
					 bool bidirectional_flag) const
{
      assert(scope);

      svector<NetNet*>nets (parms_.size());
      unsigned width = 0;
      unsigned errors = 0;

      if (repeat_) {
	    cerr << get_fileline() << ": sorry: I do not know how to"
		  " elaborate repeat concatenation nets." << endl;
	    return 0;
      }

	/* Elaborate the operands of the concatenation. */
      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Elaborate subexpression "
		       << idx << " of " << nets.count() << " l-values: "
		       << *parms_[idx] << endl;
	    }

	    if (parms_[idx] == 0) {
		  cerr << get_fileline() << ": error: Empty expressions "
		       << "not allowed in concatenations." << endl;
		  errors += 1;
		  continue;
	    }

	    if (bidirectional_flag) {
		  nets[idx] = parms_[idx]->elaborate_bi_net(des, scope);
	    } else {
		  nets[idx] = parms_[idx]->elaborate_lnet(des, scope);
	    }

	    if (nets[idx] == 0) {
                  errors += 1;
            } else if (nets[idx]->data_type() == IVL_VT_REAL) {
		  cerr << parms_[idx]->get_fileline() << ": error: "
		       << "concatenation operand can no be real: "
		       << *parms_[idx] << endl;
		  errors += 1;
		  continue;
	    } else {
                  width += nets[idx]->vector_width();
            }
      }

      if (errors) {
	    des->errors += errors;
	    return 0;
      }

	/* Make the temporary signal that connects to all the
	   operands, and connect it up. Scan the operands of the
	   concat operator from most significant to least significant,
	   which is the order they are given in the concat list. */

      netvector_t*tmp2_vec = new netvector_t(nets[0]->data_type(),width-1,0);
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, tmp2_vec);

	/* Assume that the data types of the nets are all the same, so
	   we can take the data type of any, the first will do. */
      osig->local_flag(true);
      osig->set_line(*this);

      if (bidirectional_flag) {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Generating tran(VP) "
		       << "to connect input l-value to subexpressions."
		       << endl;
	    }

	    for (unsigned idx = 0 ; idx < nets.count() ; idx += 1) {
		  unsigned wid = nets[idx]->vector_width();
		  unsigned off = width - wid;
		  NetTran*ps = new NetTran(scope, scope->local_symbol(),
					   osig->vector_width(), wid, off);
		  des->add_node(ps);
		  ps->set_line(*this);

		  connect(ps->pin(0), osig->pin(0));
		  connect(ps->pin(1), nets[idx]->pin(0));

		  ivl_assert(*this, wid <= width);
		  width -= wid;
	    }

      } else {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Generating part selects "
		       << "to connect input l-value to subexpressions."
		       << endl;
	    }

	    NetPartSelect::dir_t part_dir = NetPartSelect::VP;

	    for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
		  unsigned wid = nets[idx]->vector_width();
		  unsigned off = width - wid;
		  NetPartSelect*ps = new NetPartSelect(osig, off, wid, part_dir);
		  des->add_node(ps);
		  ps->set_line(*this);

		  connect(ps->pin(1), osig->pin(0));
		  connect(ps->pin(0), nets[idx]->pin(0));

		  assert(wid <= width);
		  width -= wid;
	    }
	    assert(width == 0);
      }

      return osig;
}

NetNet* PEConcat::elaborate_lnet(Design*des, NetScope*scope) const
{
      return elaborate_lnet_common_(des, scope, false);
}

NetNet* PEConcat::elaborate_bi_net(Design*des, NetScope*scope) const
{
      return elaborate_lnet_common_(des, scope, true);
}

bool PEConcat::is_collapsible_net(Design*des, NetScope*scope) const
{
      assert(scope);

        // Repeat concatenations are not currently supported.
      if (repeat_)
            return false;

	// Test the operands of the concatenation.
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {

              // Empty expressions are not allowed in concatenations
	    if (parms_[idx] == 0)
                  return false;

	    if (!parms_[idx]->is_collapsible_net(des, scope))
                  return false;
      }

      return true;
}

/*
 * This private method evaluates the part selects (if any) for the
 * signal. The sig argument is the NetNet already located for the
 * PEIdent name. The midx and lidx arguments are loaded with the
 * results, which may be the whole vector, or a single bit, or
 * anything in between. The values are in canonical indices.
 */
bool PEIdent::eval_part_select_(Design*des, NetScope*scope, NetNet*sig,
				long&midx, long&lidx) const
{
      list<long> prefix_indices;
      bool rc = calculate_packed_indices_(des, scope, sig, prefix_indices);
      ivl_assert(*this, rc);

      const name_component_t&name_tail = path_.back();
	// Only treat as part/bit selects any index that is beyond the
	// word selects for an array. This is not an array, then
	// dimensions==0 and any index is treated as a select.
      if (name_tail.index.size() <= sig->unpacked_dimensions()) {
	    midx = sig->vector_width()-1;
	    lidx = 0;
	    return true;
      }

      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();

      switch (index_tail.sel) {
	  default:
	    cerr << get_fileline() << ": internal error: "
		 << "Unexpected sel_ value = " << index_tail.sel << endl;
	    ivl_assert(*this, 0);
	    break;

	  case index_component_t::SEL_IDX_DO:
	  case index_component_t::SEL_IDX_UP: {
		NetExpr*tmp_ex = elab_and_eval(des, scope, index_tail.msb, -1, true);
		NetEConst*tmp = dynamic_cast<NetEConst*>(tmp_ex);
		if (!tmp) {
		      cerr << get_fileline() << ": error: indexed part select of "
		           << sig->name()
		           << " must be a constant in this context." << endl;
		      des->errors += 1;
		      return 0;
		}

		  /* The width (a constant) is calculated here. */
		unsigned long wid = 0;
		bool flag = calculate_up_do_width_(des, scope, wid);
		if (! flag) return false;

		  /* We have an undefined index and that is out of range. */
		if (! tmp->value().is_defined()) {
		      if (warn_ob_select) {
			    cerr << get_fileline() << ": warning: "
			         << sig->name();
			    if (sig->unpacked_dimensions() > 0) cerr << "[]";
			    cerr << "['bx";
			    if (index_tail.sel ==
			        index_component_t::SEL_IDX_UP) {
				  cerr << "+:";
			    } else {
				  cerr << "-:";
			    }
			    cerr << wid << "] is always outside the vector."
			         << endl;
		      }
		      return false;
		}

		long midx_val = tmp->value().as_long();
		midx = sig->sb_to_idx(prefix_indices, midx_val);
		delete tmp_ex;

		if (index_tail.sel == index_component_t::SEL_IDX_UP)
		      lidx = sig->sb_to_idx(prefix_indices, midx_val+wid-1);
		else
		      lidx = sig->sb_to_idx(prefix_indices, midx_val-wid+1);

		if (midx < lidx) {
		      long tmpx = midx;
		      midx = lidx;
		      lidx = tmpx;
		}

		  /* Warn about an indexed part select that is out of range. */
		if (warn_ob_select && (lidx < 0)) {
		      cerr << get_fileline() << ": warning: " << sig->name();
		      if (sig->unpacked_dimensions() > 0) cerr << "[]";
		      cerr << "[" << midx_val;
		      if (index_tail.sel == index_component_t::SEL_IDX_UP) {
			    cerr << "+:";
		      } else {
			    cerr << "-:";
		      }
		      cerr << wid << "] is selecting before vector." << endl;
		}
		if (warn_ob_select && (midx >= (long)sig->vector_width())) {
		      cerr << get_fileline() << ": warning: " << sig->name();
		      if (sig->unpacked_dimensions() > 0) {
			    cerr << "[]";
		      }
		      cerr << "[" << midx_val;
		      if (index_tail.sel == index_component_t::SEL_IDX_UP) {
			    cerr << "+:";
		      } else {
			    cerr << "-:";
		      }
		      cerr << wid << "] is selecting after vector." << endl;
		}

		  /* This is completely out side the signal so just skip it. */
		if (lidx >= (long)sig->vector_width() || midx < 0) {
		      return false;
		}

		break;
	  }

	  case index_component_t::SEL_PART: {

		long msb, lsb;
		bool part_defined_flag;
		/* bool flag = */ calculate_parts_(des, scope, msb, lsb, part_defined_flag);

		  /* We have an undefined index and that is out of range. */
		if (!part_defined_flag) {
		      if (warn_ob_select) {
			    cerr << get_fileline() << ": warning: "
			         << sig->name();
			    if (sig->unpacked_dimensions() > 0) cerr << "[]";
			    cerr << "['bx] is always outside the vector."
			         << endl;
		      }
		      return false;
		}

		if (prefix_indices.size()+1 < sig->packed_dims().size()) {
		      // Here we have a slice that doesn't have enough indices
		      // to get to a single slice. For example:
		      //    wire [9:0][5:1] foo
		      //      ... foo[4:3] ...
		      // Make this work by finding the indexed slices and
		      // creating a generated slice that spans the whole
		      // range.
		      long loff, moff;
		      unsigned long lwid, mwid;
		      bool lrc;
		      lrc = sig->sb_to_slice(prefix_indices, lsb, loff, lwid);
		      ivl_assert(*this, lrc);
		      lrc = sig->sb_to_slice(prefix_indices, msb, moff, mwid);
		      ivl_assert(*this, lrc);
		      ivl_assert(*this, lwid == mwid);

		      if (moff > loff) {
			    lidx = loff;
			    midx = moff + mwid - 1;
		      } else {
			    lidx = moff;
			    midx = loff + lwid - 1;
		      }
		} else {
		      long lidx_tmp = sig->sb_to_idx(prefix_indices, lsb);
		      long midx_tmp = sig->sb_to_idx(prefix_indices, msb);

		        /* Detect reversed indices of a part select. */
		      if (lidx_tmp > midx_tmp) {
			    cerr << get_fileline() << ": error: Part select "
			        << sig->name() << "[" << msb << ":"
			        << lsb << "] indices reversed." << endl;
			    cerr << get_fileline() << ":      : Did you mean "
			        << sig->name() << "[" << lsb << ":"
			        << msb << "]?" << endl;
			    long tmp = midx_tmp;
			    midx_tmp = lidx_tmp;
			    lidx_tmp = tmp;
			    des->errors += 1;
		      }

		        /* Warn about a part select that is out of range. */
		      if (midx_tmp >= (long)sig->vector_width() || lidx_tmp < 0) {
			    cerr << get_fileline() << ": warning: Part select "
			         << sig->name();
			    if (sig->unpacked_dimensions() > 0) {
				cerr << "[]";
			    }
			    cerr << "[" << msb << ":" << lsb
			         << "] is out of range." << endl;
		}
		        /* This is completely out side the signal so just skip it. */
		      if (lidx_tmp >= (long)sig->vector_width() || midx_tmp < 0) {
			    return false;
		      }

		      midx = midx_tmp;
		      lidx = lidx_tmp;
		}
		break;
	  }

	  case index_component_t::SEL_BIT:
	    if (name_tail.index.size() > sig->unpacked_dimensions()) {
		  long msb;
		  bool bit_defined_flag;
		  /* bool flag = */ calculate_bits_(des, scope, msb, bit_defined_flag);

		  /* We have an undefined index and that is out of range. */
		  if (!bit_defined_flag) {
			if (warn_ob_select) {
			      cerr << get_fileline() << ": warning: "
			           << sig->name();
			      if (sig->unpacked_dimensions() > 0) cerr << "[]";
			      cerr << "['bx] is always outside the vector."
			           << endl;
			}
			return false;
		  }


		  if (prefix_indices.size()+2 <= sig->packed_dims().size()) {
			long tmp_loff;
			unsigned long tmp_lwid;
			bool rcl = sig->sb_to_slice(prefix_indices, msb,
						    tmp_loff, tmp_lwid);
			ivl_assert(*this, rcl);
			midx = tmp_loff + tmp_lwid - 1;
			lidx = tmp_loff;
		  } else {
			midx = sig->sb_to_idx(prefix_indices, msb);
			if (midx >= (long)sig->vector_width()) {
			      cerr << get_fileline() << ": error: Index " << sig->name()
				   << "[" << msb << "] is out of range."
				   << endl;
			      des->errors += 1;
			      midx = 0;
			}
			lidx = midx;
		  }

	    } else {
		  cerr << get_fileline() << ": internal error: "
		       << "Bit select " << path_ << endl;
		  ivl_assert(*this, 0);
		  midx = sig->vector_width() - 1;
		  lidx = 0;
	    }
	    break;
      }

      return true;
}

/*
 * This is the common code for l-value nets and bi-directional
 * nets. There is very little that is different between the two cases,
 * so most of the work for both is done here.
 */
NetNet* PEIdent::elaborate_lnet_common_(Design*des, NetScope*scope,
					bool bidirectional_flag) const
{
      assert(scope);

      NetNet*       sig = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;
      perm_string method_name;

      symbol_search(this, des, scope, path_, sig, par, eve);

      if (eve != 0) {
	    cerr << get_fileline() << ": error: named events (" << path_
		 << ") cannot be l-values in continuous "
		 << "assignments." << endl;
	    des->errors += 1;
	    return 0;
      }

	// Break the path_ into the tail name and the prefix. For
	// example, a name "a.b.c" is broken into name_tail="c" and
	// path_prefix="a.b".
      const name_component_t&path_tail = path_.back();
      pform_name_t path_prefix = path_;
      path_prefix.pop_back();

	/* If the signal is not found, check to see if this is a
	   member of a struct. Take the name of the form "a.b.member",
	   remove the member and store it into method_name, and retry
	   the search with "a.b". */
      if (sig == 0 && path_.size() >= 2) {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_lnet_common_: "
			"Symbol not found, try again with path_prefix=" << path_prefix
		       << " and method_name=" << path_tail.name << endl;
	    }
	    method_name = path_tail.name;
	    symbol_search(this, des, scope, path_prefix, sig, par, eve);

	      // Whoops, not a struct signal, so give up on this avenue.
	    if (sig && sig->struct_type() == 0) {
		  cerr << get_fileline() << ": XXXXX: sig=" << sig->name()
		       << " is found, but not a struct with member " << method_name << endl;
		  method_name = perm_string();
		  sig = 0;
	    }
      }

      if (sig == 0) {
	    cerr << get_fileline() << ": error: Net " << path_
		 << " is not defined in this context." << endl;
	    des->errors += 1;
	    return 0;
      }

      assert(sig);

	/* If this is SystemVerilog and the variable is not yet
	   assigned by anything, then convert it to an unresolved
	   wire. */
      if (gn_var_can_be_uwire()
	  && (sig->type() == NetNet::REG)
	  && (sig->peek_lref() == 0) ) {
	    sig->type(NetNet::UNRESOLVED_WIRE);
      }

	/* Don't allow registers as assign l-values. */
      if (sig->type() == NetNet::REG) {
	    cerr << get_fileline() << ": error: reg " << sig->name()
		 << "; cannot be driven by primitives"
		 << " or continuous assignment." << endl;
	    des->errors += 1;
	    return 0;
      }

	// Default part select is the entire word.
      unsigned midx = sig->vector_width()-1, lidx = 0;
	// The default word select is the first.
      long widx = 0;
	// Set this to true if we calculate the word index. This is
	// used to distinguish between unpacked array assignment and
	// array word assignment.
      bool widx_flag = false;

      list<long> unpacked_indices_const;

      const netstruct_t*struct_type = 0;
      if ((struct_type = sig->struct_type()) && !method_name.nil()) {

	      // Detect the variable is a structure and there was a
	      // method name detected. We've already found that
	      // the path_ is <>.sig.method_name and signal
	      // (NetNet). We also know that sig is struct_type(), so
	      // look for a method named method_name.
	    if (debug_elaborate)
		  cerr << get_fileline() << ": PEIdent::elaborate_lnet_common_: "
		       << "Signal " << sig->name() << " is a structure, "
		       << "try to match member " << method_name << endl;

	    unsigned long member_off = 0;
	    const struct netstruct_t::member_t*member = struct_type->packed_member(method_name, member_off);
	    ivl_assert(*this, member);

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_lnet_common_: "
		       << "Member " << method_name
		       << " has type " << *member->net_type << "." << endl;
		  cerr << get_fileline() << ":                                : "
		       << "Tail name has " << path_tail.index.size() << " indices." << endl;
	    }

	      // Rewrite a member select of a packed structure as a
	      // part select of the base variable.
	    lidx = member_off;
	    midx = lidx + member->net_type->packed_width() - 1;

	      // The dimensions of the tail of the prefix must match
	      // the dimensions of the signal at this point. (The sig
	      // has a packed dimension for the packed struct size.)
	      // For example, if the path_=a[<m>][<n>].member, then
	      // sig must have 3 packed dimensions: one for the struct
	      // members and two actual packed dimensions.
	    ivl_assert(*this, path_prefix.back().index.size()+1 == sig->packed_dimensions());

	      // Elaborate an expression from the packed indices and
	      // the member offset (into the structure) to get a
	      // canonical expression into the packed signal vector.
	    NetExpr*packed_base = 0;
	    if (sig->packed_dimensions() > 1) {
		  list<index_component_t>tmp_index = path_prefix.back().index;
		  index_component_t member_select;
		  member_select.sel = index_component_t::SEL_BIT;
		  member_select.msb = new PENumber(new verinum(member_off));
		  tmp_index.push_back(member_select);
		  packed_base = collapse_array_indices(des, scope, sig, tmp_index);

		  if (debug_elaborate) {
			cerr << get_fileline() << ": PEIdent::elaborate_lnet_common_: "
			     << "packed_base=" << *packed_base
			     << ", member_off=" << member_off << endl;
		  }
	    }

	    long tmp;
	    if (packed_base && eval_as_long(tmp, packed_base)) {
		  lidx = tmp;
		  midx = lidx + member->net_type->packed_width() - 1;
		  delete packed_base;
		  packed_base = 0;
	    }

	      // Currently, only support const dimensions here.
	    ivl_assert(*this, packed_base == 0);

	      // Now the lidx/midx values get us to the member. Next
	      // up, deal with bit/part selects from the member
	      // itself.
	      //XXXXivl_assert(*this, member->packed_dims.size() <= 1);
	    ivl_assert(*this, path_tail.index.size() <= 1);
	    if (! path_tail.index.empty()) {
		  long tmp_off;
		  unsigned long tmp_wid;
		  const index_component_t&tail_sel = path_tail.index.back();
		  ivl_assert(*this, tail_sel.sel == index_component_t::SEL_PART || tail_sel.sel == index_component_t::SEL_BIT);
		  bool rc = calculate_part(this, des, scope, tail_sel, tmp_off, tmp_wid);
		  ivl_assert(*this, rc);
		  if (debug_elaborate)
			cerr << get_fileline() << ": PEIdent::elaborate_lnet_common_: "
			     << "tmp_off=" << tmp_off << ", tmp_wid=" << tmp_wid << endl;
		  lidx += tmp_off;
		  midx = lidx + tmp_wid - 1;
	    }

      } else if (gn_system_verilog() && sig->unpacked_dimensions() > 0 && path_tail.index.empty()) {

	      // In this case, we are doing a continuous assignment to
	      // an unpacked array. The NetNet representation is a
	      // NetNet with a pin for each array element, so there is
	      // nothing more needed here.
	      //
	      // This can come up from code like this:
	      //   logic [...] data [0:3];
	      //   assign data = ...;
	      // In this case, "sig" is "data", and sig->pin_count()
	      // is 4 to account for the unpacked size.
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_lnet_common_: "
		       << "Net assign to unpacked array \"" << sig->name()
		       << "\" with " << sig->pin_count() << " elements." << endl;
	    }

      } else if (sig->unpacked_dimensions() > 0) {

	      // Make sure there are enough indices to address an array element.
	    if (path_tail.index.size() < sig->unpacked_dimensions()) {
		  cerr << get_fileline() << ": error: Array " << path()
		       << " needs " << sig->unpacked_dimensions() << " indices,"
		       << " but got only " << path_tail.index.size() << ". (net)" << endl;
		  des->errors += 1;
		  return 0;
	    }

	      // Evaluate all the index expressions into an
	      // "unpacked_indices" array.
	    list<NetExpr*>unpacked_indices;
	    indices_flags flags;
	    indices_to_expressions(des, scope, this,
				   path_tail.index, sig->unpacked_dimensions(),
				   true,
				   flags,
				   unpacked_indices,
				   unpacked_indices_const);

	    if (flags.invalid) {
		  return 0;

	    } else if (flags.variable) {
		  cerr << get_fileline() << ": error: array '" << sig->name()
		       << "' index must be a constant in this context." << endl;
		  des->errors += 1;
		  return 0;

	    } else if (flags.undefined) {
		  cerr << get_fileline() << ": warning: "
		       << "ignoring undefined l-value array access "
		       << sig->name() << as_indices(unpacked_indices)
		       << "." << endl;
		  widx = -1;
		  widx_flag = true;

	    } else {
		  NetExpr*canon_index = 0;
		  ivl_assert(*this, unpacked_indices_const.size() == sig->unpacked_dimensions());
		  canon_index = normalize_variable_unpacked(sig, unpacked_indices_const);

		  if (canon_index == 0) {
			cerr << get_fileline() << ": warning: "
			     << "ignoring out of bounds l-value array access "
			     << sig->name() << as_indices(unpacked_indices_const)
			     << "." << endl;
			widx = -1;
			widx_flag = true;

		  } else {
			NetEConst*canon_const = dynamic_cast<NetEConst*>(canon_index);
			ivl_assert(*this, canon_const);

			widx = canon_const->value().as_long();
			widx_flag = true;
			delete canon_index;
		  }
	    }

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: Use [" << widx << "]"
		       << " to index l-value array." << endl;

	      /* The array has a part/bit select at the end. */
	    if (path_tail.index.size() > sig->unpacked_dimensions()) {
		  if (sig->get_scalar()) {
		        cerr << get_fileline() << ": error: "
		             << "can not select part of ";
			if (sig->data_type() == IVL_VT_REAL) cerr << "real";
			else cerr << "scalar";
			cerr << " array word: " << sig->name()
			     << as_indices(unpacked_indices_const) << endl;
		        des->errors += 1;
		        return 0;
		  }

		  long midx_tmp, lidx_tmp;
		  if (! eval_part_select_(des, scope, sig, midx_tmp, lidx_tmp))
		        return 0;

		  if (lidx_tmp < 0) {
		        cerr << get_fileline() << ": sorry: part selects "
		                "straddling the start of signal (" << path_
		             << ") are not currently supported." << endl;
		        des->errors += 1;
		        return 0;
		  }
		  midx = midx_tmp;
		  lidx = lidx_tmp;
	    }

      } else if (!path_tail.index.empty()) {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_lnet_common_: "
		       << "path_tail.index.size()=" << path_tail.index.size()
		       << endl;
	    }

	      // There are index expressions on the name, so this is a
	      // bit/slice select of the name. Calculate a canonical
	      // part select.

	    if (sig->get_scalar()) {
		  cerr << get_fileline() << ": error: "
		       << "can not select part of ";
		  if (sig->data_type() == IVL_VT_REAL) cerr << "real: ";
		  else cerr << "scalar: ";
		  cerr << sig->name() << endl;
		  des->errors += 1;
		  return 0;
	    }

	    long midx_tmp, lidx_tmp;
	    if (! eval_part_select_(des, scope, sig, midx_tmp, lidx_tmp))
		  return 0;

	    if (lidx_tmp < 0) {
		  cerr << get_fileline() << ": sorry: part selects "
		          "straddling the start of signal (" << path_
                       << ") are not currently supported." << endl;
		  des->errors += 1;
		  return 0;
	    }
	    midx = midx_tmp;
	    lidx = lidx_tmp;
      }

      unsigned subnet_wid = midx-lidx+1;

	/* Check if the l-value bits are double-driven. */
      if (sig->type() == NetNet::UNRESOLVED_WIRE && sig->test_and_set_part_driver(midx,lidx, widx_flag? widx : 0)) {
	    cerr << get_fileline() << ": error: Unresolved net/uwire "
	         << sig->name() << " cannot have multiple drivers." << endl;
	    if (debug_elaborate) {
		  cerr << get_fileline() << ":      : Overlap in "
		       << "[" << midx << ":" << lidx << "] (canonical)"
		       << ", widx=" << (widx_flag? widx : 0)
		       << ", vector width=" << sig->vector_width()
		       << endl;
	    }
	    des->errors += 1;
	    return 0;
      }

      if (sig->pin_count() > 1 && widx_flag) {
	    if (widx < 0 || widx >= (long) sig->pin_count())
		  return 0;

	    netvector_t*tmp2_vec = new netvector_t(sig->data_type(),
						   sig->vector_width()-1,0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    sig->type(), tmp2_vec);
	    tmp->set_line(*this);
	    tmp->local_flag(true);
	    connect(sig->pin(widx), tmp->pin(0));
	    sig = tmp;

      } else if (sig->pin_count() > 1) {

	      // If this turns out to be an l-value unpacked array,
	      // then let the caller handle it. It will probably be
	      // converted into an array of assignments.
	    return sig;
      }

	/* If the desired l-value vector is narrower than the
	   signal itself, then use a NetPartSelect node to
	   arrange for connection to the desired bits. All this
	   can be skipped if the desired width matches the
	   original vector. */

      if (subnet_wid != sig->vector_width()) {
	      /* If we are processing a tran or inout, then the
		 partselect is bi-directional. Otherwise, it is a
		 Part-to-Vector select. */

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "Elaborate lnet part select "
		       << sig->name()
		       << "[base=" << lidx
		       << " wid=" << subnet_wid <<"]"
		       << endl;

	    netvector_t*tmp2_vec = new netvector_t(sig->data_type(),
						   subnet_wid-1,0);
	    NetNet*subsig = new NetNet(sig->scope(),
				       sig->scope()->local_symbol(),
				       NetNet::WIRE, tmp2_vec);
	    subsig->local_flag(true);
	    subsig->set_line(*this);

	    if (bidirectional_flag) {
		    // Make a tran(VP)
		  NetTran*sub = new NetTran(scope, scope->local_symbol(),
					    sig->vector_width(),
					    subnet_wid, lidx);
		  sub->set_line(*this);
		  des->add_node(sub);
		  connect(sub->pin(0), sig->pin(0));
		  connect(sub->pin(1), subsig->pin(0));

	    } else {
		  NetPartSelect*sub = new NetPartSelect(sig, lidx, subnet_wid,
							NetPartSelect::PV);
		  des->add_node(sub);
		  sub->set_line(*this);
		  connect(sub->pin(0), subsig->pin(0));
		  collapse_partselect_pv_to_concat(des, sig);
	    }
	    sig = subsig;
      }

      return sig;
}

/*
 * Identifiers in continuous assignment l-values are limited to wires
 * and that ilk. Detect registers and memories here and report errors.
 */
NetNet* PEIdent::elaborate_lnet(Design*des, NetScope*scope) const
{
      return elaborate_lnet_common_(des, scope, false);
}

NetNet* PEIdent::elaborate_bi_net(Design*des, NetScope*scope) const
{
      return elaborate_lnet_common_(des, scope, true);
}

/*
 * This method is used to elaborate identifiers that are ports to a
 * scope. The scope is presumed to be that of the module that has the
 * port. This elaboration is done inside the module, and is only done
 * to PEIdent objects. This method is used by elaboration of a module
 * instantiation (PGModule::elaborate_mod_) to get NetNet objects for
 * the port.
 */
NetNet* PEIdent::elaborate_subport(Design*des, NetScope*scope) const
{
      ivl_assert(*this, scope->type() == NetScope::MODULE);
      NetNet*sig = des->find_signal(scope, path_);
      if (sig == 0) {
	    cerr << get_fileline() << ": error: no wire/reg " << path_
		 << " in module " << scope_path(scope) << "." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* Check the port_type of the signal to make sure it is really
	   a port, and its direction is resolved. */
      switch (sig->port_type()) {
	  case NetNet::PINPUT:
	  case NetNet::POUTPUT:
	  case NetNet::PINOUT:
	  case NetNet::PREF:
	    break;

	      /* If the name matches, but the signal is not a port,
		 then the user declared the object but there is no
		 matching input/output/inout declaration. */

	  case NetNet::NOT_A_PORT:
	    cerr << get_fileline() << ": error: signal " << path_ << " in"
		 << " module " << scope_path(scope) << " is not a port." << endl;
	    cerr << get_fileline() << ":      : Are you missing an input/"
		 << "output/inout declaration?" << endl;
	    des->errors += 1;
	    return 0;

	      /* This should not happen. A PWire can only become
		 PIMPLICIT if this is a UDP reg port, and the make_udp
		 function should turn it into an output.... I think. */

	  case NetNet::PIMPLICIT:
	    cerr << get_fileline() << ": internal error: signal " << path_
		 << " in module " << scope_path(scope) << " is left as "
		 << "port type PIMPLICIT." << endl;
	    des->errors += 1;
	    return 0;
      }

      long midx;
      long lidx;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_subport: "
		 << "path_ = \"" << path_
		 << "\", unpacked_dimensions=" << sig->unpacked_dimensions()
		 << ", port_type()=" << sig->port_type() << endl;
      }

      if (sig->unpacked_dimensions() && !gn_system_verilog()) {
	    cerr << get_fileline() << ": error: "
		 << "Ports cannot be unpacked arrays. Try enabling SystemVerilog support." << endl;
	    des->errors += 1;
	    return 0;
      }

	// There cannot be parts to an unpacked array, so process this
	// simply as an unpacked array.
      if (sig->unpacked_dimensions()) {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_subport: "
		       << "path_=\"" << path_
		       << "\" is an unpacked array with " << sig->pin_count()
		       << " elements." << endl;
	    }
	    scope->add_module_port_net(sig);
	    return sig;
      }

	/* Evaluate the part/bit select expressions, to get the part
	   select of the signal that attaches to the port. Also handle
	   range and direction checking here. */

      if (! eval_part_select_(des, scope, sig, midx, lidx))
	    return 0;

	/* If this is a part select of the entire signal (or no part
	   select at all) then we're done. */
      if ((lidx == 0) && (midx == (long)sig->vector_width()-1)) {
	    scope->add_module_port_net(sig);
	    return sig;
      }

      unsigned swid = abs(midx - lidx) + 1;
      ivl_assert(*this, swid > 0 && swid < sig->vector_width());

      netvector_t*tmp2_vec = new netvector_t(sig->data_type(),swid-1,0);
      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, tmp2_vec);
      tmp->port_type(sig->port_type());
      tmp->set_line(*this);
      tmp->local_flag(true);
      NetNode*ps = 0;
      switch (sig->port_type()) {

	  case NetNet::PINPUT:
	    ps = new NetPartSelect(sig, lidx, swid, NetPartSelect::PV);
	    connect(tmp->pin(0), ps->pin(0));
	    sig = tmp;
	    break;

	  case NetNet::POUTPUT:
	    ps = new NetPartSelect(sig, lidx, swid, NetPartSelect::VP);
	    connect(tmp->pin(0), ps->pin(0));
	    sig = tmp;
	    break;

	  case NetNet::PREF:
	      // For the purposes of module ports, treat ref ports
	      // just like inout ports.
	  case NetNet::PINOUT:
	    ps = new NetTran(scope, scope->local_symbol(), sig->vector_width(),
			     swid, lidx);
	    connect(sig->pin(0), ps->pin(0));
	    connect(tmp->pin(0), ps->pin(1));
	    sig = tmp;
	    break;

	  default:
	    ivl_assert(*this, 0);
	    break;
      }

      ps->set_line(*this);
      des->add_node(ps);

      scope->add_module_port_net(sig);
      return sig;
}

NetNet*PEIdent::elaborate_unpacked_net(Design*des, NetScope*scope) const
{
      NetNet*       sig = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;
      perm_string method_name;

      symbol_search(this, des, scope, path_, sig, par, eve);

      ivl_assert(*this, sig);

      return sig;
}

bool PEIdent::is_collapsible_net(Design*des, NetScope*scope) const
{
      assert(scope);

      NetNet*       sig = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      symbol_search(this, des, scope, path_, sig, par, eve);

      if (eve != 0)
            return false;

      if (sig == 0)
            return false;

      assert(sig);

	/* If this is SystemVerilog and the variable is not yet
	   assigned by anything, then convert it to an unresolved
	   wire. */
      if (gn_var_can_be_uwire()
	  && (sig->type() == NetNet::REG)
	  && (sig->peek_eref() == 0) ) {
	    sig->type(NetNet::UNRESOLVED_WIRE);
      }

      if (sig->type() == NetNet::UNRESOLVED_WIRE && sig->pin(0).is_linked())
            return false;

      if (sig->type() == NetNet::REG)
            return false;

      return true;
}
