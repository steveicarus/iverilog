/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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

# include  <cstdlib>
# include  <climits>
# include  "netlist.h"
# include  "netparray.h"
# include  "netvector.h"
# include  "netmisc.h"
# include  "PExpr.h"
# include  "pform_types.h"
# include  "compiler.h"
# include  "ivl_assert.h"


NetNet* sub_net_from(Design*des, NetScope*scope, long val, NetNet*sig)
{
      netvector_t*zero_vec = new netvector_t(sig->data_type(),
					     sig->vector_width()-1, 0);
      NetNet*zero_net = new NetNet(scope, scope->local_symbol(),
				   NetNet::WIRE, zero_vec);
      zero_net->set_line(*sig);
      zero_net->local_flag(true);

      if (sig->data_type() == IVL_VT_REAL) {
	    verireal zero (val);
	    NetLiteral*zero_obj = new NetLiteral(scope, scope->local_symbol(), zero);
	    zero_obj->set_line(*sig);
	    des->add_node(zero_obj);

	    connect(zero_net->pin(0), zero_obj->pin(0));

      } else {
	    verinum zero ((int64_t)val);
	    zero = cast_to_width(zero, sig->vector_width());
	    zero.has_sign(sig->get_signed());
	    NetConst*zero_obj = new NetConst(scope, scope->local_symbol(), zero);
	    zero_obj->set_line(*sig);
	    des->add_node(zero_obj);

	    connect(zero_net->pin(0), zero_obj->pin(0));
      }

      NetAddSub*adder = new NetAddSub(scope, scope->local_symbol(), sig->vector_width());
      adder->set_line(*sig);
      des->add_node(adder);
      adder->attribute(perm_string::literal("LPM_Direction"), verinum("SUB"));

      connect(zero_net->pin(0), adder->pin_DataA());
      connect(adder->pin_DataB(), sig->pin(0));

      netvector_t*tmp_vec = new netvector_t(sig->data_type(),
					    sig->vector_width()-1, 0);
      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, tmp_vec);
      tmp->set_line(*sig);
      tmp->local_flag(true);

      connect(adder->pin_Result(), tmp->pin(0));

      return tmp;
}

NetNet* cast_to_int2(Design*des, NetScope*scope, NetNet*src, unsigned wid)
{
      if (src->data_type() == IVL_VT_BOOL)
	    return src;

      netvector_t*tmp_vec = new netvector_t(IVL_VT_BOOL, wid-1, 0,
					    src->get_signed());
      NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, tmp_vec);
      tmp->set_line(*src);
      tmp->local_flag(true);

      NetCastInt2*cast = new NetCastInt2(scope, scope->local_symbol(), wid);
      cast->set_line(*src);
      des->add_node(cast);

      connect(cast->pin(0), tmp->pin(0));
      connect(cast->pin(1), src->pin(0));

      return tmp;
}

NetNet* cast_to_int4(Design*des, NetScope*scope, NetNet*src, unsigned wid)
{
      if (src->data_type() != IVL_VT_REAL)
	    return src;

      netvector_t*tmp_vec = new netvector_t(IVL_VT_LOGIC, wid-1, 0);
      NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, tmp_vec);
      tmp->set_line(*src);
      tmp->local_flag(true);

      NetCastInt4*cast = new NetCastInt4(scope, scope->local_symbol(), wid);
      cast->set_line(*src);
      des->add_node(cast);

      connect(cast->pin(0), tmp->pin(0));
      connect(cast->pin(1), src->pin(0));

      return tmp;
}

NetNet* cast_to_real(Design*des, NetScope*scope, NetNet*src)
{
      if (src->data_type() == IVL_VT_REAL)
	    return src;

      netvector_t*tmp_vec = new netvector_t(IVL_VT_REAL);
      NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, tmp_vec);
      tmp->set_line(*src);
      tmp->local_flag(true);

      NetCastReal*cast = new NetCastReal(scope, scope->local_symbol(), src->get_signed());
      cast->set_line(*src);
      des->add_node(cast);

      connect(cast->pin(0), tmp->pin(0));
      connect(cast->pin(1), src->pin(0));

      return tmp;
}

NetExpr* cast_to_int2(NetExpr*expr, unsigned width)
{
	// Special case: The expression is already BOOL
      if (expr->expr_type() == IVL_VT_BOOL)
	    return expr;

      if (debug_elaborate)
	    cerr << expr->get_fileline() << ": debug: "
		 << "Cast expression to int2, width=" << width << "." << endl;

      NetECast*cast = new NetECast('2', expr, width, expr->has_sign());
      cast->set_line(*expr);
      return cast;
}

NetExpr* cast_to_int4(NetExpr*expr, unsigned width)
{
	// Special case: The expression is already LOGIC or BOOL
      if (expr->expr_type() != IVL_VT_REAL)
	    return expr;

      if (debug_elaborate)
	    cerr << expr->get_fileline() << ": debug: "
		 << "Cast expression to int4, width=" << width << "." << endl;

      NetECast*cast = new NetECast('v', expr, width, expr->has_sign());
      cast->set_line(*expr);
      return cast;
}

NetExpr* cast_to_real(NetExpr*expr)
{
      if (expr->expr_type() == IVL_VT_REAL)
	    return expr;

      if (debug_elaborate)
	    cerr << expr->get_fileline() << ": debug: "
		 << "Cast expression to real." << endl;

      NetECast*cast = new NetECast('r', expr, 1, true);
      cast->set_line(*expr);
      return cast;
}

/*
 * Add a signed constant to an existing expression. Generate a new
 * NetEBAdd node that has the input expression and an expression made
 * from the constant value.
 */
static NetExpr* make_add_expr(NetExpr*expr, long val)
{
      if (val == 0)
	    return expr;

	// If the value to be added is <0, then instead generate a
	// SUBTRACT node and turn the value positive.
      char add_op = '+';
      if (val < 0) {
	    add_op = '-';
	    val = -val;
      }

      verinum val_v (val, expr->expr_width());
      val_v.has_sign(true);

      NetEConst*val_c = new NetEConst(val_v);
      val_c->set_line(*expr);

      NetEBAdd*res = new NetEBAdd(add_op, expr, val_c, expr->expr_width(),
                                  expr->has_sign());
      res->set_line(*expr);

      return res;
}

static NetExpr* make_add_expr(const LineInfo*loc, NetExpr*expr1, NetExpr*expr2)
{
      bool use_signed = expr1->has_sign() && expr2->has_sign();
      unsigned use_wid = expr1->expr_width();

      if (expr2->expr_width() > use_wid)
	    use_wid = expr2->expr_width();

      expr1 = pad_to_width(expr1, use_wid, *loc);
      expr2 = pad_to_width(expr2, use_wid, *loc);

      NetEBAdd*tmp = new NetEBAdd('+', expr1, expr2, use_wid, use_signed);
      return tmp;
}

/*
 * Subtract an existing expression from a signed constant.
 */
static NetExpr* make_sub_expr(long val, NetExpr*expr)
{
      verinum val_v (val, expr->expr_width());
      val_v.has_sign(true);

      NetEConst*val_c = new NetEConst(val_v);
      val_c->set_line(*expr);

      NetEBAdd*res = new NetEBAdd('-', val_c, expr, expr->expr_width(),
                                  expr->has_sign());
      res->set_line(*expr);

      return res;
}

/*
 * Multiple an existing expression by a signed positive number.
 * This does a lossless multiply, so the arguments will need to be
 * sized to match the output size.
 */
static NetExpr* make_mult_expr(NetExpr*expr, unsigned long val)
{
      const unsigned val_wid = ceil(log2((double)val)) ;
      unsigned use_wid = expr->expr_width() + val_wid;
      verinum val_v (val, use_wid);
      val_v.has_sign(true);

      NetEConst*val_c = new NetEConst(val_v);
      val_c->set_line(*expr);

	// We know by definitions that the expr argument needs to be
	// padded to be the right argument width for this lossless multiply.
      expr = pad_to_width(expr, use_wid, *expr);

      NetEBMult*res = new NetEBMult('*', expr, val_c, use_wid, expr->has_sign());
      res->set_line(*expr);

      return res;
}

/*
 * This routine is used to calculate the number of bits needed to
 * contain the given number.
 */
static unsigned num_bits(long arg)
{
      unsigned res = 0;

	/* For a negative value we have room for one extra value, but
	 * we have a signed result so we need an extra bit for this. */
      if (arg < 0) {
	    arg = -arg - 1;
	    res += 1;
      }

	/* Calculate the number of bits needed here. */
      while (arg) {
	    res += 1;
	    arg >>= 1;
      }

      return res;
}

/*
 * This routine generates the normalization expression needed for a variable
 * bit select or a variable base expression for an indexed part
 * select. This function doesn't actually look at the variable
 * dimensions, it just does the final calculation using msb/lsb of the
 * last slice, and the off of the slice in the variable.
 */
NetExpr *normalize_variable_base(NetExpr *base, long msb, long lsb,
				 unsigned long wid, bool is_up, long soff)
{
      long offset = lsb;

      if (msb < lsb) {
	      /* Correct the offset if needed. */
	    if (is_up) offset -= wid - 1;
	      /* Calculate the space needed for the offset. */
	    unsigned min_wid = num_bits(offset);
	    if (num_bits(soff) > min_wid)
		  min_wid = num_bits(soff);
	      /* We need enough space for the larger of the offset or the
	       * base expression. */
	    if (min_wid < base->expr_width()) min_wid = base->expr_width();
	      /* Now that we have the minimum needed width increase it by
	       * one to make room for the normalization calculation. */
	    min_wid += 2;
	      /* Pad the base expression to the correct width. */
	    base = pad_to_width(base, min_wid, *base);
	      /* If the base expression is unsigned and either the lsb
	       * is negative or it does not fill the width of the base
	       * expression then we could generate negative normalized
	       * values so cast the expression to signed to get the
	       * math correct. */
	    if ((lsb < 0 || num_bits(lsb+1) <= base->expr_width()) &&
	        ! base->has_sign()) {
		    /* We need this extra select to hide the signed
		     * property from the padding above. It will be
		     * removed automatically during code generation. */
		  NetESelect *tmp = new NetESelect(base, 0 , min_wid);
		  tmp->set_line(*base);
		  tmp->cast_signed(true);
                  base = tmp;
	    }
	      /* Normalize the expression. */
	    base = make_sub_expr(offset+soff, base);
      } else {
	      /* Correct the offset if needed. */
	    if (!is_up) offset += wid - 1;
	      /* If the offset is zero then just return the base (index)
	       * expression. */
	    if ((soff-offset) == 0) return base;
	      /* Calculate the space needed for the offset. */
	    unsigned min_wid = num_bits(-offset);
	    if (num_bits(soff) > min_wid)
		  min_wid = num_bits(soff);
	      /* We need enough space for the larger of the offset or the
	       * base expression. */
	    if (min_wid < base->expr_width()) min_wid = base->expr_width();
	      /* Now that we have the minimum needed width increase it by
	       * one to make room for the normalization calculation. */
	    min_wid += 2;
	      /* Pad the base expression to the correct width. */
	    base = pad_to_width(base, min_wid, *base);
	      /* If the offset is greater than zero then we need to do
	       * signed math to get the location value correct. */
	    if (offset > 0 && ! base->has_sign()) {
		    /* We need this extra select to hide the signed
		     * property from the padding above. It will be
		     * removed automatically during code generation. */
		  NetESelect *tmp = new NetESelect(base, 0 , min_wid);
		  tmp->set_line(*base);
		  tmp->cast_signed(true);
                  base = tmp;
	    }
	      /* Normalize the expression. */
	    base = make_add_expr(base, soff-offset);
      }

      return base;
}

/*
 * This method is how indices should work except that the base should
 * be a vector of expressions that matches the size of the dims list,
 * so that we can generate an expression based on the entire packed
 * vector. For now, we assert that there is only one set of dimensions.
 */
NetExpr *normalize_variable_base(NetExpr *base,
				 const list<netrange_t>&dims,
				 unsigned long wid, bool is_up)
{
      ivl_assert(*base, dims.size() == 1);
      const netrange_t&rng = dims.back();
      return normalize_variable_base(base, rng.get_msb(), rng.get_lsb(), wid, is_up);
}

NetExpr *normalize_variable_bit_base(const list<long>&indices, NetExpr*base,
				     const NetNet*reg)
{
      const vector<netrange_t>&packed_dims = reg->packed_dims();
      ivl_assert(*base, indices.size()+1 == packed_dims.size());

	// Get the canonical offset of the slice within which we are
	// addressing. We need that address as a slice offset to
	// calculate the proper complete address
      const netrange_t&rng = packed_dims.back();
      long slice_off = reg->sb_to_idx(indices, rng.get_lsb());

      return normalize_variable_base(base, rng.get_msb(), rng.get_lsb(), 1, true, slice_off);
}

NetExpr *normalize_variable_part_base(const list<long>&indices, NetExpr*base,
				      const NetNet*reg,
				      unsigned long wid, bool is_up)
{
      const vector<netrange_t>&packed_dims = reg->packed_dims();
      ivl_assert(*base, indices.size()+1 == packed_dims.size());

	// Get the canonical offset of the slice within which we are
	// addressing. We need that address as a slice offset to
	// calculate the proper complete address
      const netrange_t&rng = packed_dims.back();
      long slice_off = reg->sb_to_idx(indices, rng.get_lsb());

      return normalize_variable_base(base, rng.get_msb(), rng.get_lsb(), wid, is_up, slice_off);
}

NetExpr *normalize_variable_slice_base(const list<long>&indices, NetExpr*base,
				       const NetNet*reg, unsigned long&lwid)
{
      const vector<netrange_t>&packed_dims = reg->packed_dims();
      ivl_assert(*base, indices.size() < packed_dims.size());

      vector<netrange_t>::const_iterator pcur = packed_dims.end();
      for (size_t idx = indices.size() ; idx < packed_dims.size(); idx += 1) {
	    -- pcur;
      }

      long sb;
      if (pcur->get_msb() >= pcur->get_lsb())
	    sb = pcur->get_lsb();
      else
	    sb = pcur->get_msb();

      long loff;
      reg->sb_to_slice(indices, sb, loff, lwid);

      base = make_mult_expr(base, lwid);
      base = make_add_expr(base, loff);
      return base;
}

ostream& operator << (ostream&o, __IndicesManip<long> val)
{
      for (list<long>::const_iterator cur = val.val.begin()
		 ; cur != val.val.end() ; ++cur) {
	    o << "[" << *cur << "]";
      }
      return o;
}

ostream& operator << (ostream&o, __IndicesManip<NetExpr*> val)
{
      for (list<NetExpr*>::const_iterator cur = val.val.begin()
		 ; cur != val.val.end() ; ++cur) {
	    o << "[" << *(*cur) << "]";
      }
      return o;
}

/*
 * The src is the input index expression list from the expression, and
 * the count is the number that are to be elaborated into the indices
 * list. At the same time, create a indices_const list that contains
 * the evaluated values for the expression, if they can be evaluated.
 */
void indices_to_expressions(Design*des, NetScope*scope,
			      // loc is for error messages.
			    const LineInfo*loc,
			      // src is the index list, and count is
			      // the number of items in the list to use.
			    const list<index_component_t>&src, unsigned count,
			      // True if the expression MUST be constant.
			    bool need_const,
			      // These are the outputs.
			    indices_flags&flags,
			    list<NetExpr*>&indices, list<long>&indices_const)
{
      ivl_assert(*loc, count <= src.size());

      flags.invalid   = false;
      flags.variable  = false;
      flags.undefined = false;
      for (list<index_component_t>::const_iterator cur = src.begin()
		 ; count > 0 ;  ++cur, --count) {
	    ivl_assert(*loc, cur->sel != index_component_t::SEL_NONE);

	    if (cur->sel != index_component_t::SEL_BIT) {
		  cerr << loc->get_fileline() << ": error: "
		       << "Array cannot be indexed by a range." << endl;
		  des->errors += 1;
	    }
	    ivl_assert(*loc, cur->msb);

	    NetExpr*word_index = elab_and_eval_lossless(des, scope, cur->msb, -2, need_const);

	    if (word_index == 0)
		  flags.invalid = true;

	      // Track if we detect any non-constant expressions
	      // here. This may allow for a special case.
	    NetEConst*word_const = dynamic_cast<NetEConst*> (word_index);
	    if (word_const == 0)
		  flags.variable = true;
	    else if (!word_const->value().is_defined())
		  flags.undefined = true;
	    else if (!flags.variable && !flags.undefined)
		  indices_const.push_back(word_const->value().as_long());

	    indices.push_back(word_index);
      }
}

static void make_strides(const vector<netrange_t>&dims,
			 vector<long>&stride)
{
      stride[dims.size()-1] = 1;
      for (size_t idx = stride.size()-1 ; idx > 0 ; --idx) {
	    long tmp = dims[idx].width();
	    if (idx < stride.size())
		  tmp *= stride[idx];
	    stride[idx-1] = tmp;
      }
}

/*
 * Take in a vector of constant indices and convert them to a single
 * number that is the canonical address (zero based, 1-d) of the
 * word. If any of the indices are out of bounds, return nil instead
 * of an expression.
 */
static NetExpr* normalize_variable_unpacked(const vector<netrange_t>&dims, list<long>&indices)
{
	// Make strides for each index. The stride is the distance (in
	// words) to the next element in the canonical array.
      vector<long> stride (dims.size());
      make_strides(dims, stride);

      int64_t canonical_addr = 0;

      int idx = 0;
      for (list<long>::const_iterator cur = indices.begin()
		 ; cur != indices.end() ; ++cur, ++idx) {
	    long tmp = *cur;

	    if (dims[idx].get_lsb() <= dims[idx].get_msb())
		  tmp -= dims[idx].get_lsb();
	    else
		  tmp -= dims[idx].get_msb();

	      // Notice of this index is out of range.
	    if (tmp < 0 || tmp >= (long)dims[idx].width()) {
		  return 0;
	    }

	    canonical_addr += tmp * stride[idx];
      }

      NetEConst*canonical_expr = new NetEConst(verinum(canonical_addr));
      return canonical_expr;
}

NetExpr* normalize_variable_unpacked(const NetNet*net, list<long>&indices)
{
      const vector<netrange_t>&dims = net->unpacked_dims();
      return normalize_variable_unpacked(dims, indices);
}

NetExpr* normalize_variable_unpacked(const netsarray_t*stype, list<long>&indices)
{
      const vector<netrange_t>&dims = stype->static_dimensions();
      return normalize_variable_unpacked(dims, indices);
}

NetExpr* normalize_variable_unpacked(const LineInfo&loc, const vector<netrange_t>&dims, list<NetExpr*>&indices)
{
	// Make strides for each index. The stride is the distance (in
	// words) to the next element in the canonical array.
      vector<long> stride (dims.size());
      make_strides(dims, stride);

      NetExpr*canonical_expr = 0;

      int idx = 0;
      for (list<NetExpr*>::const_iterator cur = indices.begin()
		 ; cur != indices.end() ; ++cur, ++idx) {
	    NetExpr*tmp = *cur;
	      // If the expression elaboration generated errors, then
	      // give up. Presumably, the error during expression
	      // elaboration already generated the error message.
	    if (tmp == 0)
		  return 0;

	    int64_t use_base;
	    if (! dims[idx].defined())
		  use_base = 0;
	    else if (dims[idx].get_lsb() <= dims[idx].get_msb())
		  use_base = dims[idx].get_lsb();
	    else
		  use_base = dims[idx].get_msb();

	    int64_t use_stride = stride[idx];

	      // Account for that we are doing arithmetic and should
	      // have a proper width to make sure there are no
	      // losses. So calculate a min_wid width.
	    unsigned tmp_wid;
	    unsigned min_wid = tmp->expr_width();
	    if (use_base != 0 && ((tmp_wid = num_bits(use_base)) >= min_wid))
		  min_wid = tmp_wid + 1;
	    if ((tmp_wid = num_bits(dims[idx].width()+1)) >= min_wid)
		  min_wid = tmp_wid + 1;
	    if (use_stride != 1)
		  min_wid += num_bits(use_stride);

	    tmp = pad_to_width(tmp, min_wid, loc);

	      // Now generate the math to calculate the canonical address.
	    NetExpr*tmp_scaled = 0;
	    if (NetEConst*tmp_const = dynamic_cast<NetEConst*> (tmp)) {
		    // Special case: the index is constant, so this
		    // iteration can be replaced with a constant
		    // expression.
		  int64_t val = tmp_const->value().as_long();
		  val -= use_base;
		  val *= use_stride;
		    // Very special case: the index is zero, so we can
		    // skip this iteration
		  if (val == 0)
			continue;
		  tmp_scaled = new NetEConst(verinum(val));

	    } else {
		  tmp_scaled = tmp;
		  if (use_base != 0)
			tmp_scaled = make_add_expr(tmp_scaled, -use_base);
		  if (use_stride != 1)
			tmp_scaled = make_mult_expr(tmp_scaled, use_stride);
	    }

	    if (canonical_expr == 0) {
		  canonical_expr = tmp_scaled;
	    } else {
		  bool expr_has_sign = canonical_expr->has_sign() &&
		                        tmp_scaled->has_sign();
		  canonical_expr = new NetEBAdd('+', canonical_expr, tmp_scaled,
						canonical_expr->expr_width()+1,
		                                expr_has_sign);
	    }
      }

	// If we don't have an expression at this point, all the indices were
	// constant zero. But this variant of normalize_variable_unpacked()
	// is only used when at least one index is not a constant.
	ivl_assert(loc, canonical_expr);

      return canonical_expr;
}

NetExpr* normalize_variable_unpacked(const NetNet*net, list<NetExpr*>&indices)
{
      const vector<netrange_t>&dims = net->unpacked_dims();
      return normalize_variable_unpacked(*net, dims, indices);
}

NetExpr* normalize_variable_unpacked(const LineInfo&loc, const netsarray_t*stype, list<NetExpr*>&indices)
{
      const vector<netrange_t>&dims = stype->static_dimensions();
      return normalize_variable_unpacked(loc, dims, indices);
}

NetExpr* make_canonical_index(Design*des, NetScope*scope,
			      const LineInfo*loc,
			      const std::list<index_component_t>&src,
			      const netsarray_t*stype,
			      bool need_const)
{
      NetExpr*canon_index = 0;

      list<long> indices_const;
      list<NetExpr*> indices_expr;
      indices_flags flags;
      indices_to_expressions(des, scope, loc,
			     src, src.size(),
			     need_const,
			     flags,
			     indices_expr, indices_const);

      if (flags.undefined) {
	    cerr << loc->get_fileline() << ": warning: "
		 << "ignoring undefined value array access." << endl;

      } else if (flags.variable) {
	    canon_index = normalize_variable_unpacked(*loc, stype, indices_expr);

      } else {
	    canon_index = normalize_variable_unpacked(stype, indices_const);
      }

      return canon_index;
}

NetEConst* make_const_x(unsigned long wid)
{
      verinum xxx (verinum::Vx, wid);
      NetEConst*resx = new NetEConst(xxx);
      return resx;
}

NetEConst* make_const_0(unsigned long wid)
{
      verinum xxx (verinum::V0, wid);
      NetEConst*resx = new NetEConst(xxx);
      return resx;
}

NetEConst* make_const_val(unsigned long value)
{
      verinum tmp (value, integer_width);
      NetEConst*res = new NetEConst(tmp);
      return res;
}

NetEConst* make_const_val_s(long value)
{
      verinum tmp (value, integer_width);
      tmp.has_sign(true);
      NetEConst*res = new NetEConst(tmp);
      return res;
}

NetNet* make_const_x(Design*des, NetScope*scope, unsigned long wid)
{
      verinum xxx (verinum::Vx, wid);
      NetConst*res = new NetConst(scope, scope->local_symbol(), xxx);
      des->add_node(res);

      netvector_t*sig_vec = new netvector_t(IVL_VT_LOGIC, wid-1, 0);
      NetNet*sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, sig_vec);
      sig->local_flag(true);

      connect(sig->pin(0), res->pin(0));
      return sig;
}

NetNet* make_const_z(Design*des, NetScope*scope, unsigned long wid)
{
      verinum xxx (verinum::Vz, wid);
      NetConst*res = new NetConst(scope, scope->local_symbol(), xxx);
      des->add_node(res);

      netvector_t*sig_vec = new netvector_t(IVL_VT_LOGIC, wid-1, 0);
      NetNet*sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, sig_vec);
      sig->local_flag(true);

      connect(sig->pin(0), res->pin(0));
      return sig;
}

NetExpr* condition_reduce(NetExpr*expr)
{
      if (expr->expr_type() == IVL_VT_REAL) {
	    if (NetECReal *tmp = dynamic_cast<NetECReal*>(expr)) {
		  verinum::V res;
		  if (tmp->value().as_double() == 0.0) res = verinum::V0;
		  else res = verinum::V1;
		  verinum vres (res, 1, true);
		  NetExpr *rtn = new NetEConst(vres);
		  rtn->set_line(*expr);
		  delete expr;
		  return rtn;
	    }

	    NetExpr *rtn = new NetEBComp('n', expr,
	                                 new NetECReal(verireal(0.0)));
	    rtn->set_line(*expr);
	    return rtn;
      }

      if (expr->expr_width() == 1)
	    return expr;

      verinum zero (verinum::V0, expr->expr_width());
      zero.has_sign(expr->has_sign());

      NetEConst*ezero = new NetEConst(zero);
      ezero->set_line(*expr);

      NetEBComp*cmp = new NetEBComp('n', expr, ezero);
      cmp->set_line(*expr);
      cmp->cast_signed(false);

      return cmp;
}

static NetExpr* do_elab_and_eval(Design*des, NetScope*scope, PExpr*pe,
				 int context_width, bool need_const,
				 bool annotatable, bool force_expand,
				 ivl_variable_type_t cast_type,
				 bool force_unsigned)
{
      PExpr::width_mode_t mode = PExpr::SIZED;
      if ((context_width == -2) && !gn_strict_expr_width_flag)
            mode = PExpr::EXPAND;
      if (force_expand)
	    mode = PExpr::EXPAND;

      pe->test_width(des, scope, mode);

        // Get the final expression width. If the expression is unsized,
        // this may be different from the value returned by test_width().
      unsigned expr_width = pe->expr_width();

        // If context_width is positive, this is the RHS of an assignment,
        // so the LHS width must also be included in the width calculation.
      unsigned pos_context_width = context_width > 0 ? context_width : 0;
      if ((pe->expr_type() != IVL_VT_REAL) && (expr_width < pos_context_width))
            expr_width = pos_context_width;

	// If this is the RHS of a compressed assignment, the LHS also
	// affects the expression type (signed/unsigned).
      if (force_unsigned)
	    pe->cast_signed(false);

      if (debug_elaborate) {
            cerr << pe->get_fileline() << ": elab_and_eval: test_width of "
                 << *pe << endl;
            cerr << pe->get_fileline() << ":              : "
                 << "returns type=" << pe->expr_type()
		 << ", context_width=" << context_width
                 << ", signed=" << pe->has_sign()
		 << ", force_expand=" << force_expand
                 << ", expr_width=" << expr_width
                 << ", mode=" << PExpr::width_mode_name(mode) << endl;
	    cerr << pe->get_fileline() << ":              : "
		 << "cast_type=" << cast_type << endl;
      }

        // If we can get the same result using a smaller expression
        // width, do so.

      unsigned min_width = pe->min_width();
      if ((min_width != UINT_MAX) && (pe->expr_type() != IVL_VT_REAL)
          && (pos_context_width > 0) && (expr_width > pos_context_width)) {
            expr_width = max(min_width, pos_context_width);

            if (debug_elaborate) {
                  cerr << pe->get_fileline() << ":              : "
                       << "pruned to width=" << expr_width << endl;
            }
      }

      if ((mode >= PExpr::LOSSLESS) && (expr_width > width_cap)
          && (expr_width > pos_context_width)) {
            cerr << pe->get_fileline() << ": warning: excessive unsized "
                 << "expression width detected." << endl;
            cerr << pe->get_fileline() << ":        : The expression width "
                 << "is capped at " << width_cap << " bits." << endl;
	    expr_width = width_cap;
      }

      unsigned flags = PExpr::NO_FLAGS;
      if (need_const)
            flags |= PExpr::NEED_CONST;
      if (annotatable)
            flags |= PExpr::ANNOTATABLE;

      if (debug_elaborate) {
	    cerr << pe->get_fileline() << ": elab_and_eval: "
		 << "Calculated width is " << expr_width << "." << endl;
      }

      NetExpr*tmp = pe->elaborate_expr(des, scope, expr_width, flags);
      if (tmp == 0) return 0;

      if ((cast_type != IVL_VT_NO_TYPE) && (cast_type != tmp->expr_type())) {
            switch (cast_type) {
                case IVL_VT_REAL:
                  tmp = cast_to_real(tmp);
                  break;
                case IVL_VT_BOOL:
                  tmp = cast_to_int2(tmp, pos_context_width);
                  break;
                case IVL_VT_LOGIC:
                  tmp = cast_to_int4(tmp, pos_context_width);
                  break;
                default:
                  break;
            }
      }

	// If the context_width sent is is actually the minimum width,
	// then raise the context_width to be big enough for the
	// lossless expression.
      if (force_expand && context_width > 0) {
	    context_width = max(context_width, (int)expr_width);
      }

      eval_expr(tmp, context_width);

      if (NetEConst*ce = dynamic_cast<NetEConst*>(tmp)) {
            if ((mode >= PExpr::LOSSLESS) && (context_width < 0))
                  ce->trim();
      }

      return tmp;
}

NetExpr* elab_and_eval(Design*des, NetScope*scope, PExpr*pe,
		       int context_width, bool need_const, bool annotatable,
		       ivl_variable_type_t cast_type, bool force_unsigned)
{
      return do_elab_and_eval(des, scope, pe, context_width,
			      need_const, annotatable, false,
			      cast_type, force_unsigned);
}

/*
 * This variant of elab_and_eval does the expression losslessly, no
 * matter what the generation of verilog. This is in support of
 * certain special contexts, notably index expressions.
 */
NetExpr* elab_and_eval_lossless(Design*des, NetScope*scope, PExpr*pe,
				 int context_width, bool need_const, bool annotatable,
				 ivl_variable_type_t cast_type)
{
      return do_elab_and_eval(des, scope, pe, context_width,
			      need_const, annotatable, true,
			      cast_type, false);
}

NetExpr* elab_and_eval(Design*des, NetScope*scope, PExpr*pe,
		       ivl_type_t lv_net_type, bool need_const)
{
      if (debug_elaborate) {
	    cerr << pe->get_fileline() << ": elab_and_eval: "
		 << "pe=" << *pe
		 << ", lv_net_type=" << *lv_net_type << endl;
      }

	// Elaborate the expression using the more general
	// elaborate_expr method.
      unsigned flags = PExpr::NO_FLAGS;
      if (need_const)
            flags |= PExpr::NEED_CONST;

      NetExpr*tmp = pe->elaborate_expr(des, scope, lv_net_type, flags);

      return tmp;
}

NetExpr* elab_sys_task_arg(Design*des, NetScope*scope, perm_string name,
                           unsigned arg_idx, PExpr*pe, bool need_const)
{
      PExpr::width_mode_t mode = PExpr::SIZED;
      pe->test_width(des, scope, mode);

      if (debug_elaborate) {
            cerr << pe->get_fileline() << ": debug: test_width of "
                 << name << " argument " << (arg_idx+1) << " " << *pe << endl;
            cerr << pe->get_fileline() << ":        "
                 << "returns type=" << pe->expr_type()
                 << ", width=" << pe->expr_width()
                 << ", signed=" << pe->has_sign()
                 << ", mode=" << PExpr::width_mode_name(mode) << endl;
      }

      unsigned flags = PExpr::SYS_TASK_ARG;
      if (need_const)
            flags |= PExpr::NEED_CONST;

      NetExpr*tmp = pe->elaborate_expr(des, scope, pe->expr_width(), flags);
      if (tmp == 0) return 0;

      eval_expr(tmp, -1);

      if (NetEConst*ce = dynamic_cast<NetEConst*>(tmp)) {
              // For lossless/unsized constant expressions, we can now
              // determine the exact width required to hold the result.
              // But leave literal numbers exactly as the user supplied
              // them.
            if ((mode >= PExpr::LOSSLESS) && !dynamic_cast<PENumber*>(pe) && tmp->expr_width()>32)
                  ce->trim();
      }

      return tmp;
}

bool evaluate_ranges(Design*des, NetScope*scope,
		     vector<netrange_t>&llist,
		     const list<pform_range_t>&rlist)
{
      bool bad_msb = false, bad_lsb = false;

      for (list<pform_range_t>::const_iterator cur = rlist.begin()
		 ; cur != rlist.end() ; ++cur) {
	    long use_msb, use_lsb;

	    NetExpr*texpr = elab_and_eval(des, scope, cur->first, -1, true);
	    if (! eval_as_long(use_msb, texpr)) {
		  cerr << cur->first->get_fileline() << ": error: "
			"Range expressions must be constant." << endl;
		  cerr << cur->first->get_fileline() << "       : "
			"This MSB expression violates the rule: "
		       << *cur->first << endl;
		  des->errors += 1;
		  bad_msb = true;
	    }

	    delete texpr;

	    texpr = elab_and_eval(des, scope, cur->second, -1, true);
	    if (! eval_as_long(use_lsb, texpr)) {
		  cerr << cur->second->get_fileline() << ": error: "
			"Range expressions must be constant." << endl;
		  cerr << cur->second->get_fileline() << "       : "
			"This LSB expression violates the rule: "
		       << *cur->second << endl;
		  des->errors += 1;
		  bad_lsb = true;
	    }

	    delete texpr;

	      /* Error recovery */
	    if (bad_lsb) use_lsb = 0;
	    if (bad_msb) use_msb = use_lsb;

	    llist.push_back(netrange_t(use_msb, use_lsb));
      }

      return bad_msb | bad_lsb;
}

void eval_expr(NetExpr*&expr, int context_width)
{
      assert(expr);
      if (dynamic_cast<NetECReal*>(expr)) return;

      NetExpr*tmp = expr->eval_tree();
      if (tmp != 0) {
	    tmp->set_line(*expr);
	    delete expr;
	    expr = tmp;
      }

      if (context_width <= 0) return;

      NetEConst *ce = dynamic_cast<NetEConst*>(expr);
      if (ce == 0) return;

        // The expression is a constant, so resize it if needed.
      if (ce->expr_width() < (unsigned)context_width) {
            expr = pad_to_width(expr, context_width, *expr);
      } else if (ce->expr_width() > (unsigned)context_width) {
            verinum value(ce->value(), context_width);
            ce = new NetEConst(value);
            ce->set_line(*expr);
            delete expr;
            expr = ce;
      }
}

bool eval_as_long(long&value, const NetExpr*expr)
{
      if (const NetEConst*tmp = dynamic_cast<const NetEConst*>(expr) ) {
	    value = tmp->value().as_long();
	    return true;
      }

      if (const NetECReal*rtmp = dynamic_cast<const NetECReal*>(expr)) {
	    value = rtmp->value().as_long();
	    return true;
      }

      return false;
}

bool eval_as_double(double&value, NetExpr*expr)
{
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(expr) ) {
	    value = tmp->value().as_double();
	    return true;
      }

      if (NetECReal*rtmp = dynamic_cast<NetECReal*>(expr)) {
	    value = rtmp->value().as_double();
	    return true;
      }

      return false;
}

/*
 * At the parser level, a name component is a name with a collection
 * of expressions. For example foo[N] is the name "foo" and the index
 * expression "N". This function takes as input the name component and
 * returns the path component name. It will evaluate the index
 * expression if it is present.
 */
hname_t eval_path_component(Design*des, NetScope*scope,
			    const name_component_t&comp,
			    bool&error_flag)
{
	// No index expression, so the path component is an undecorated
	// name, for example "foo".
      if (comp.index.empty())
	    return hname_t(comp.name);

      vector<int> index_values;

      for (list<index_component_t>::const_iterator cur = comp.index.begin()
		 ; cur != comp.index.end() ; ++cur) {
	    const index_component_t&index = *cur;

	    if (index.sel != index_component_t::SEL_BIT) {
		  cerr << index.msb->get_fileline() << ": error: "
		       << "Part select is not valid for this kind of object." << endl;
		  des->errors += 1;
		  return hname_t(comp.name, 0);
	    }

	      // The parser will assure that path components will have only
	      // bit select index expressions. For example, "foo[n]" is OK,
	      // but "foo[n:m]" is not.
	    assert(index.sel == index_component_t::SEL_BIT);

	      // Evaluate the bit select to get a number.
	    NetExpr*tmp = elab_and_eval(des, scope, index.msb, -1);
	    ivl_assert(*index.msb, tmp);

	    if (NetEConst*ctmp = dynamic_cast<NetEConst*>(tmp)) {
		  index_values.push_back(ctmp->value().as_long());
		  delete ctmp;
		  continue;
	    }
#if 1
	      // Darn, the expression doesn't evaluate to a constant. That's
	      // an error to be reported. And make up a fake index value to
	      // return to the caller.
	    cerr << index.msb->get_fileline() << ": error: "
		 << "Scope index expression is not constant: "
		 << *index.msb << endl;
	    des->errors += 1;
#endif
	    error_flag = true;

	    delete tmp;
      }

      return hname_t(comp.name, index_values);
}

std::list<hname_t> eval_scope_path(Design*des, NetScope*scope,
				   const pform_name_t&path)
{
      bool path_error_flag = false;
      list<hname_t> res;

      typedef pform_name_t::const_iterator pform_path_it;

      for (pform_path_it cur = path.begin() ; cur != path.end(); ++ cur ) {
	    const name_component_t&comp = *cur;
	    res.push_back( eval_path_component(des,scope,comp,path_error_flag) );
      }
#if 0
      if (path_error_flag) {
	    cerr << "XXXXX: Errors evaluating path " << path << endl;
      }
#endif
      return res;
}

/*
 * Human readable version of op. Used in elaboration error messages.
 */
const char *human_readable_op(const char op, bool unary)
{
	const char *type;
	switch (op) {
	    case '~': type = "~";  break;  // Negation

	    case '+': type = "+";  break;
	    case '-': type = "-";  break;
	    case '*': type = "*";  break;
	    case '/': type = "/";  break;
	    case '%': type = "%";  break;

	    case '<': type = "<";  break;
	    case '>': type = ">";  break;
	    case 'L': type = "<="; break;
	    case 'G': type = ">="; break;

	    case '^': type = "^";  break;  // XOR
	    case 'X': type = "~^"; break;  // XNOR
	    case '&': type = "&";  break;  // Bitwise AND
	    case 'A': type = "~&"; break;  // NAND (~&)
	    case '|': type = "|";  break;  // Bitwise OR
	    case 'O': type = "~|"; break;  // NOR

	    case '!': type = "!"; break;   // Logical NOT
	    case 'a': type = "&&"; break;  // Logical AND
	    case 'o': type = "||"; break;  // Logical OR

	    case 'e': type = "==";  break;
	    case 'n': type = "!=";  break;
	    case 'E': type = "==="; break;  // Case equality
	    case 'N':
		if (unary) type = "~|";     // NOR
		else type = "!==";          // Case inequality
		break;

	    case 'l': type = "<<(<)"; break;  // Left shifts
	    case 'r': type = ">>";    break;  // Logical right shift
	    case 'R': type = ">>>";   break;  // Arithmetic right shift

	    case 'p': type = "**"; break; // Power

	    case 'i':
	    case 'I': type = "++"; break; /* increment */
	    case 'd':
	    case 'D': type = "--"; break; /* decrement */

	    default:
	      type = "???";
	      assert(0);
	}
	return type;
}

const_bool const_logical(const NetExpr*expr)
{
      switch (expr->expr_type()) {
	  case IVL_VT_REAL: {
	    const NetECReal*val = dynamic_cast<const NetECReal*> (expr);
	    if (val == 0) return C_NON;
	    if (val->value().as_double() == 0.0) return C_0;
	    else return C_1;
	  }

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC: {
	    const NetEConst*val = dynamic_cast<const NetEConst*> (expr);
	    if (val == 0) return C_NON;
	    verinum cval = val->value();
	    const_bool res = C_0;
	    for (unsigned idx = 0; idx < cval.len(); idx += 1) {
		  switch (cval.get(idx)) {
		      case verinum::V1:
			return C_1;
			break;

		      case verinum::V0:
			break;

		      default:
			if (res == C_0) res = C_X;
			break;
		  }
	    }
	    return res;
	  }

	  default:
	    break;
      }

      return C_NON;
}

uint64_t get_scaled_time_from_real(Design*des, NetScope*scope, NetECReal*val)
{
      verireal fn = val->value();

      int shift = scope->time_unit() - scope->time_precision();
      assert(shift >= 0);
      int64_t delay = fn.as_long64(shift);


      shift = scope->time_precision() - des->get_precision();
      assert(shift >= 0);
      for (int lp = 0; lp < shift; lp += 1) delay *= 10;

      return delay;
}

/*
 * This function looks at the NetNet signal to see if there are any
 * NetPartSelect::PV nodes driving this signal. If so, See if they can
 * be collapsed into a single concatenation.
 */
void collapse_partselect_pv_to_concat(Design*des, NetNet*sig)
{
      NetScope*scope = sig->scope();
      vector<NetPartSelect*> ps_map (sig->vector_width());

      Nexus*nex = sig->pin(0).nexus();

      for (Link*cur = nex->first_nlink(); cur ; cur = cur->next_nlink()) {
	    NetPins*obj;
	    unsigned obj_pin;
	    cur->cur_link(obj, obj_pin);

	      // Look for NetPartSelect devices, where this signal is
	      // connected to pin 1 of a NetPartSelect::PV.
	    NetPartSelect*ps_obj = dynamic_cast<NetPartSelect*> (obj);
	    if (ps_obj == 0)
		  continue;
	    if (ps_obj->dir() != NetPartSelect::PV)
		  continue;
	    if (obj_pin != 1)
		  continue;

	      // Don't support overrun selects here.
	    if (ps_obj->base()+ps_obj->width() > ps_map.size())
		  continue;

	    ivl_assert(*ps_obj, ps_obj->base() < ps_map.size());
	    ps_map[ps_obj->base()] = ps_obj;
      }

	// Check the collected NetPartSelect::PV objects to see if
	// they cover the vector.
      unsigned idx = 0;
      unsigned device_count = 0;
      while (idx < ps_map.size()) {
	    NetPartSelect*ps_obj = ps_map[idx];
	    if (ps_obj == 0)
		  return;

	    idx += ps_obj->width();
	    device_count += 1;
      }

      ivl_assert(*sig, idx == ps_map.size());

	/* The vlog95 and possibly other code generators do not want
	 * to have a group of part selects turned into a transparent
	 * concatenation. */
      if (disable_concatz_generation) {
// HERE: If the part selects have matching strengths then we can use
//       a normal concat with a buf-Z after if the strengths are not
//       both strong. We would ideally delete any buf-Z driving the
//       concat, but that is not required for the vlog95 generator.
	    return;
      }

	// Ah HAH! The NetPartSelect::PV objects exactly cover the
	// target signal. We can replace all of them with a single
	// concatenation.

      if (debug_elaborate) {
	    cerr << sig->get_fileline() << ": debug: "
		 << "Collapse " << device_count
		 << " NetPartSelect::PV devices into a concatenation." << endl;
      }

      NetConcat*cat = new NetConcat(scope, scope->local_symbol(),
				    ps_map.size(), device_count,
				    true);
      des->add_node(cat);
      cat->set_line(*sig);

      connect(cat->pin(0), sig->pin(0));

      idx = 0;
      unsigned concat_position = 1;
      while (idx < ps_map.size()) {
	    assert(ps_map[idx]);
	    NetPartSelect*ps_obj = ps_map[idx];
	    connect(cat->pin(concat_position), ps_obj->pin(0));
	    concat_position += 1;
	    idx += ps_obj->width();
	    delete ps_obj;
      }
}

/*
 * Evaluate the prefix indices. All but the final index in a
 * chain of indices must be a single value and must evaluate
 * to constants at compile time. For example:
 *    [x]          - OK
 *    [1][2][x]    - OK
 *    [1][x:y]     - OK
 *    [2:0][x]     - BAD
 *    [y][x]       - BAD
 * Leave the last index for special handling.
 */
bool evaluate_index_prefix(Design*des, NetScope*scope,
			   list<long>&prefix_indices,
			   const list<index_component_t>&indices)
{
      list<index_component_t>::const_iterator icur = indices.begin();
      for (size_t idx = 0 ; (idx+1) < indices.size() ; idx += 1, ++icur) {
	    assert(icur != indices.end());
	    assert(icur->sel == index_component_t::SEL_BIT);
	    NetExpr*texpr = elab_and_eval(des, scope, icur->msb, -1, true);

	    long tmp;
	    if (texpr == 0 || !eval_as_long(tmp, texpr)) {
		  cerr << icur->msb->get_fileline() << ": error: "
			"Array index expressions must be constant here." << endl;
		  des->errors += 1;
		  return false;
	    }

	    prefix_indices.push_back(tmp);
	    delete texpr;
      }

      return true;
}

/*
 * Evaluate the indices. The chain of indices are applied to the
 * packed indices of a NetNet to generate a canonical expression to
 * replace the exprs.
 */
NetExpr*collapse_array_exprs(Design*des, NetScope*scope,
			     const LineInfo*loc, NetNet*net,
			     const list<index_component_t>&indices)
{
	// First elaborate all the expressions as far as possible.
      list<NetExpr*> exprs;
      list<long> exprs_const;
      indices_flags flags;
      indices_to_expressions(des, scope, loc, indices,
                             net->packed_dimensions(),
                             false, flags, exprs, exprs_const);
      ivl_assert(*loc, exprs.size() == net->packed_dimensions());

	// Special Case: there is only 1 packed dimension, so the
	// single expression should already be naturally canonical.
      if (net->slice_width(1) == 1) {
	    return *exprs.begin();
      }

      const std::vector<netrange_t>&pdims = net->packed_dims();
      std::vector<netrange_t>::const_iterator pcur = pdims.begin();

      list<NetExpr*>::iterator ecur = exprs.begin();
      NetExpr* base = 0;
      for (size_t idx = 0 ; idx < net->packed_dimensions() ; idx += 1, ++pcur, ++ecur) {
	    unsigned cur_slice_width = net->slice_width(idx+1);
	    long lsb = pcur->get_lsb();
	    long msb = pcur->get_msb();
	      // This normalizes the expression of this index based on
	      // the msb/lsb values.
	    NetExpr*tmp = normalize_variable_base(*ecur, msb, lsb,
						  cur_slice_width, msb > lsb);

	      // If this slice has width, then scale it.
	    if (net->slice_width(idx+1) != 1) {
		  unsigned min_wid = tmp->expr_width();
		  if (num_bits(cur_slice_width) >= min_wid) {
			min_wid = num_bits(cur_slice_width)+1;
			tmp = pad_to_width(tmp, min_wid, *loc);
		  }

		  tmp = make_mult_expr(tmp, cur_slice_width);
	    }

	      // Now add it to the position we've accumulated so far.
	    if (base) {
		  base = make_add_expr(loc, base, tmp);
	    } else {
		  base = tmp;
	    }
      }

      return base;
}

/*
 * Given a list of indices, treat them as packed indices and convert
 * them to an expression that normalizes the list to a single index
 * expression over a canonical equivalent 1-dimensional array.
 */
NetExpr*collapse_array_indices(Design*des, NetScope*scope, NetNet*net,
			       const list<index_component_t>&indices)
{
      list<long>prefix_indices;
      bool rc = evaluate_index_prefix(des, scope, prefix_indices, indices);
      assert(rc);

      const index_component_t&back_index = indices.back();
      assert(back_index.sel == index_component_t::SEL_BIT);
      assert(back_index.msb && !back_index.lsb);

      NetExpr*base = elab_and_eval(des, scope, back_index.msb, -1, true);

      NetExpr*res = normalize_variable_bit_base(prefix_indices, base, net);

      eval_expr(res, -1);
      return res;
}

void assign_unpacked_with_bufz(Design*des, NetScope*scope,
			       const LineInfo*loc,
			       NetNet*lval, NetNet*rval)
{
      ivl_assert(*loc, lval->pin_count()==rval->pin_count());

      for (unsigned idx = 0 ; idx < lval->pin_count() ; idx += 1) {
	    NetBUFZ*driver = new NetBUFZ(scope, scope->local_symbol(),
					 lval->vector_width(), false);
	    driver->set_line(*loc);
	    des->add_node(driver);

	    connect(lval->pin(idx), driver->pin(0));
	    connect(driver->pin(1), rval->pin(idx));
      }
}

/*
 * synthesis sometimes needs to unpack assignment to a part
 * select. That looks like this:
 *
 *    foo[N] <= <expr> ;
 *
 * The NetAssignBase::synth_async() method will turn that into a
 * netlist like this:
 *
 *   NetAssignBase(PV) --> base()==<N>
 *    (0)      (1)
 *     |        |
 *     v        v
 *   <expr>    foo
 *
 * This search will return a pointer to the NetAssignBase(PV) object,
 * but only if it matches this pattern.
 */
NetPartSelect* detect_partselect_lval(Link&pin)
{
      NetPartSelect*found_ps = 0;

      Nexus*nex = pin.nexus();
      for (Link*cur = nex->first_nlink() ; cur ; cur = cur->next_nlink()) {
	    NetPins*obj;
	    unsigned obj_pin;
	    cur->cur_link(obj, obj_pin);

	      // Skip NexusSet objects.
	    if (obj == 0)
		  continue;

	      // NetNet pins have no effect on this search.
	    if (dynamic_cast<NetNet*> (obj))
		  continue;

	    if (NetPartSelect*ps = dynamic_cast<NetPartSelect*> (obj)) {

		    // If this is the input side of a NetPartSelect, skip.
		  if (ps->pin(obj_pin).get_dir()==Link::INPUT)
			continue;

		    // Oops, driven by the wrong size of a
		    // NetPartSelect, so this is not going to work out.
		  if (ps->dir()==NetPartSelect::VP)
			return 0;

		    // So now we know this is a NetPartSelect::PV. It
		    // is a candidate for our part-select assign. If
		    // we already have a candidate, then give up.
		  if (found_ps)
			return 0;

		    // This is our candidate. Carry on.
		  found_ps = ps;
		  continue;

	    }

	      // If this is a driver to the Nexus that is not a
	      // NetPartSelect device. This cannot happen to
	      // part selected lval nets, so quit now.
	    if (obj->pin(obj_pin).get_dir() == Link::OUTPUT)
		  return 0;

      }

      return found_ps;
}

const netclass_t* find_class_containing_scope(const LineInfo&loc, const NetScope*scope)
{
      while (scope && scope->type() != NetScope::CLASS)
	    scope = scope->parent();

      if (scope == 0)
	    return 0;

      const netclass_t*found_in = scope->class_def();
      ivl_assert(loc, found_in);
      return found_in;
}
/*
 * Find the scope that contains this scope, that is the method for a
 * class scope. Look for the scope whose PARENT is the scope for a
 * class. This is going to be a method.
 */
NetScope* find_method_containing_scope(const LineInfo&, NetScope*scope)
{
      NetScope*up = scope->parent();

      while (up && up->type() != NetScope::CLASS) {
	    scope = up;
	    up = up->parent();
      }

      if (up == 0) return 0;

	// Should I check if this scope is a TASK or FUNC?

      return scope;
}
