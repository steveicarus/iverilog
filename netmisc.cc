/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
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

# include  <cstdlib>
# include  "netlist.h"
# include  "netmisc.h"
# include  "PExpr.h"
# include  "pform_types.h"
# include  "compiler.h"
# include  "ivl_assert.h"


NetNet* sub_net_from(Design*des, NetScope*scope, long val, NetNet*sig)
{
      NetNet*zero_net = new NetNet(scope, scope->local_symbol(),
				   NetNet::WIRE, sig->vector_width());
      zero_net->set_line(*sig);
      zero_net->data_type(sig->data_type());
      zero_net->local_flag(true);

      if (sig->data_type() == IVL_VT_REAL) {
	    verireal zero (val);
	    NetLiteral*zero_obj = new NetLiteral(scope, scope->local_symbol(), zero);
	    zero_obj->set_line(*sig);
	    des->add_node(zero_obj);

	    connect(zero_net->pin(0), zero_obj->pin(0));

      } else {
	    verinum zero ((int64_t)val);
	    zero = pad_to_width(zero, sig->vector_width());
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

      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, sig->vector_width());
      tmp->set_line(*sig);
      tmp->data_type(sig->data_type());
      tmp->local_flag(true);

      connect(adder->pin_Result(), tmp->pin(0));

      return tmp;
}

NetNet* cast_to_int2(Design*des, NetScope*scope, NetNet*src, unsigned wid)
{
      if (src->data_type() == IVL_VT_BOOL)
	    return src;

      NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, wid);
      tmp->set_line(*src);
      tmp->data_type(IVL_VT_BOOL);
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

      NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, wid);
      tmp->set_line(*src);
      tmp->data_type(IVL_VT_LOGIC);
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

      NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::WIRE);
      tmp->set_line(*src);
      tmp->data_type(IVL_VT_REAL);
      tmp->local_flag(true);

      NetCastReal*cast = new NetCastReal(scope, scope->local_symbol(), src->get_signed());
      cast->set_line(*src);
      des->add_node(cast);

      connect(cast->pin(0), tmp->pin(0));
      connect(cast->pin(1), src->pin(0));

      return tmp;
}

NetExpr* cast_to_int2(NetExpr*expr)
{
	// Special case: The expression is alreadt BOOL
      if (expr->expr_type() == IVL_VT_BOOL)
	    return expr;

      unsigned use_width = expr->expr_width();
      if (expr->expr_type() == IVL_VT_REAL)
	    use_width = 64;

      NetECast*cast = new NetECast('2', expr, use_width,
                                   expr->has_sign());
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

static NetExpr* make_mult_expr(NetExpr*expr, unsigned long val)
{
      verinum val_v (val, expr->expr_width());
      val_v.has_sign(true);

      NetEConst*val_c = new NetEConst(val_v);
      val_c->set_line(*expr);

      NetEBMult*res = new NetEBMult('*', expr, val_c, expr->expr_width(),
                                  expr->has_sign());
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
 * bit select or a variable base expression for an indexed part select.
 */
NetExpr *normalize_variable_base(NetExpr *base, long msb, long lsb,
                                 unsigned long wid, bool is_up)
{
      long offset = lsb;

      if (msb < lsb) {
	      /* Correct the offset if needed. */
	    if (is_up) offset -= wid - 1;
	      /* Calculate the space needed for the offset. */
	    unsigned min_wid = num_bits(offset);
	      /* We need enough space for the larger of the offset or the
	       * base expression. */
	    if (min_wid < base->expr_width()) min_wid = base->expr_width();
	      /* Now that we have the minimum needed width increase it by
	       * one to make room for the normalization calculation. */
	    min_wid += 1;
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
	    base = make_sub_expr(offset, base);
      } else {
	      /* Correct the offset if needed. */
	    if (!is_up) offset += wid - 1;
	      /* If the offset is zero then just return the base (index)
	       * expression. */
	    if (offset == 0) return base;
	      /* Calculate the space needed for the offset. */
	    unsigned min_wid = num_bits(-offset);
	      /* We need enough space for the larger of the offset or the
	       * base expression. */
	    if (min_wid < base->expr_width()) min_wid = base->expr_width();
	      /* Now that we have the minimum needed width increase it by
	       * one to make room for the normalization calculation. */
	    min_wid += 1;
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
	    base = make_add_expr(base, -offset);
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
				 const list<NetNet::range_t>&dims,
				 unsigned long wid, bool is_up)
{
      ivl_assert(*base, dims.size() == 1);
      const NetNet::range_t&rng = dims.back();
      return normalize_variable_base(base, rng.msb, rng.lsb, wid, is_up);
}

NetExpr *normalize_variable_slice_base(const list<long>&indices, NetExpr*base,
				       const NetNet*reg, unsigned long&lwid)
{
      const list<NetNet::range_t>&packed_dims = reg->packed_dims();
      ivl_assert(*base, indices.size() < packed_dims.size());

      list<NetNet::range_t>::const_iterator pcur = packed_dims.end();
      for (size_t idx = indices.size() ; idx < packed_dims.size(); idx += 1) {
	    -- pcur;
      }

      long sb;
      if (pcur->msb >= pcur->lsb)
	    sb = pcur->lsb;
      else
	    sb = pcur->msb;

      long loff;
      reg->sb_to_slice(indices, sb, loff, lwid);

      base = make_mult_expr(base, lwid);
      base = make_add_expr(base, loff);
      return base;
}

/*
 * This routine generates the normalization expression needed for a variable
 * array word select.
 */
NetExpr *normalize_variable_array_base(NetExpr *base, long offset,
                                       unsigned count)
{
      assert(offset != 0);
	/* Calculate the space needed for the offset. */
      unsigned min_wid = num_bits(-offset);
	/* We need enough space for the larger of the offset or the base
	 * expression. */
      if (min_wid < base->expr_width()) min_wid = base->expr_width();
	/* Now that we have the minimum needed width increase it by one
	 * to make room for the normalization calculation. */
      min_wid += 1;
	/* Pad the base expression to the correct width. */
      base = pad_to_width(base, min_wid, *base);
	/* If the offset is greater than zero then we need to do signed
	 * math to get the location value correct. */
      if (offset > 0 && ! base->has_sign()) {
	      /* We need this extra select to hide the signed property
	       * from the padding above. It will be removed automatically
	       * during code generation. */
	    NetESelect *tmp = new NetESelect(base, 0 , min_wid);
	    tmp->set_line(*base);
	    tmp->cast_signed(true);
	    base = tmp;
      }
	/* Normalize the expression. */
      base = make_add_expr(base, -offset);

	/* We should not need to do this, but .array/port does not
	 * handle a small signed index correctly and it is a major
	 * effort to fix it. For now we will just pad the expression
	 * enough so that any negative value when converted to
	 * unsigned is larger than the maximum array word. */
      if (base->has_sign()) {
	    unsigned range_wid = num_bits(count-1) + 1;
	    if (min_wid < range_wid) {
		  base = pad_to_width(base, range_wid, *base);
	    }
      }

      return base;
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

NetNet* make_const_x(Design*des, NetScope*scope, unsigned long wid)
{
      verinum xxx (verinum::Vx, wid);
      NetConst*res = new NetConst(scope, scope->local_symbol(), xxx);
      des->add_node(res);

      NetNet*sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, wid);
      sig->local_flag(true);
      sig->data_type(IVL_VT_LOGIC);

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

static const char*width_mode_name(PExpr::width_mode_t mode)
{
      switch (mode) {
          case PExpr::SIZED:
            return "sized";
          case PExpr::EXPAND:
            return "expand";
          case PExpr::LOSSLESS:
            return "lossless";
          case PExpr::UNSIZED:
            return "unsized";
          default:
            return "??";
      }
}

NetExpr* elab_and_eval(Design*des, NetScope*scope, PExpr*pe,
                       int context_width, bool need_const)
{
      PExpr::width_mode_t mode = PExpr::SIZED;
      if ((context_width == -2) && !gn_strict_expr_width_flag)
            mode = PExpr::EXPAND;

      pe->test_width(des, scope, mode);

        // Get the final expression width. If the expression is unsized,
        // this may be different from the value returned by test_width().
      unsigned expr_width = pe->expr_width();

        // If context_width is positive, this is the RHS of an assignment,
        // so the LHS width must also be included in the width calculation.
      if ((context_width > 0) && (pe->expr_type() != IVL_VT_REAL)
          && (expr_width < (unsigned)context_width))
            expr_width = context_width;

      if (debug_elaborate) {
            cerr << pe->get_fileline() << ": debug: test_width of "
                 << *pe << endl;
            cerr << pe->get_fileline() << ":        "
                 << "returns type=" << pe->expr_type()
                 << ", width=" << expr_width
                 << ", signed=" << pe->has_sign()
                 << ", mode=" << width_mode_name(mode) << endl;
      }

        // If we can get the same result using a smaller expression
        // width, do so.
      if ((context_width > 0) && (pe->expr_type() != IVL_VT_REAL)
          && (expr_width > (unsigned)context_width)) {
            expr_width = max(pe->min_width(), (unsigned)context_width);

            if (debug_elaborate) {
                  cerr << pe->get_fileline() << ":        "
                       << "pruned to width=" << expr_width << endl;
            }
      }

      unsigned flags = PExpr::NO_FLAGS;
      if (need_const)
            flags |= PExpr::NEED_CONST;

      NetExpr*tmp = pe->elaborate_expr(des, scope, expr_width, flags);
      if (tmp == 0) return 0;

      eval_expr(tmp, context_width);

      if (NetEConst*ce = dynamic_cast<NetEConst*>(tmp)) {
            if ((mode >= PExpr::LOSSLESS) && (context_width < 0))
                  ce->trim();
      }

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
                 << ", mode=" << width_mode_name(mode) << endl;
      }

      unsigned flags = PExpr::SYS_TASK_ARG;
      if (need_const)
            flags |= PExpr::NEED_CONST;

      NetExpr*tmp = pe->elaborate_expr(des, scope, pe->expr_width(), flags);
      if (tmp == 0) return 0;

      eval_expr(tmp, -1);

      if (NetEConst*ce = dynamic_cast<NetEConst*>(tmp)) {
            if (mode != PExpr::SIZED)
                  ce->trim();
      }

      return tmp;
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

bool eval_as_long(long&value, NetExpr*expr)
{
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(expr) ) {
	    value = tmp->value().as_long();
	    return true;
      }

      if (NetECReal*rtmp = dynamic_cast<NetECReal*>(expr)) {
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
			    const name_component_t&comp)
{
	// No index expression, so the path component is an undecorated
	// name, for example "foo".
      if (comp.index.empty())
	    return hname_t(comp.name);

	// The parser will assure that path components will have only
	// one index. For example, foo[N] is one index, foo[n][m] is two.
      assert(comp.index.size() == 1);

      const index_component_t&index = comp.index.front();

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

	// Now we should have a constant value for the bit select
	// expression, and we can use it to make the final hname_t
	// value, for example "foo[5]".
      if (NetEConst*ctmp = dynamic_cast<NetEConst*>(tmp)) {
	    hname_t res(comp.name, ctmp->value().as_long());
	    delete ctmp;
	    return res;
      }

	// Darn, the expression doesn't evaluate to a constant. That's
	// an error to be reported. And make up a fake index value to
	// return to the caller.
      cerr << index.msb->get_fileline() << ": error: "
	   << "Scope index expression is not constant: "
	   << *index.msb << endl;
      des->errors += 1;

      delete tmp;
      return hname_t (comp.name, 0);
}

std::list<hname_t> eval_scope_path(Design*des, NetScope*scope,
				   const pform_name_t&path)
{
      list<hname_t> res;

      typedef pform_name_t::const_iterator pform_path_it;

      for (pform_path_it cur = path.begin() ; cur != path.end(); ++ cur ) {
	    const name_component_t&comp = *cur;
	    res.push_back( eval_path_component(des,scope,comp) );
      }

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

	// Ah HAH! The NetPartSelect::PV objects exactly cover the
	// target signal. We can replace all of them with a single
	// concatenation.

      if (debug_elaborate) {
	    cerr << sig->get_fileline() << ": debug: "
		 << "Collapse " << device_count
		 << " NetPartSelect::PV devices into a concatenation." << endl;
      }

      NetConcat*cat = new NetConcat(scope, scope->local_symbol(),
				    ps_map.size(), device_count);
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
	    ivl_assert(*icur->msb, texpr);
	    long tmp;
	    if (!eval_as_long(tmp, texpr)) {
		  cerr << icur->msb->get_fileline() << ": error: "
			"Array index expressions must be constant." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    prefix_indices .push_back(tmp);
	    delete texpr;
      }

      return true;
}
