/*
 * Copyright (c) 2001-2008 Stephen Williams (steve@icarus.com)
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
# include  "ivl_assert.h"

NetNet* add_to_net(Design*des, NetNet*sig, long val)
{
      if (val == 0)
	    return sig;
#if 0
      NetScope*scope = sig->scope();
      unsigned long abs_val = (val >= 0)? val : (-val);
      unsigned width = sig->pin_count();

      verinum val_v (abs_val, width);

      NetConst*val_c = new NetConst(scope, scope->local_symbol(), val_v);

      NetNet*val_s = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, width);
      val_s->local_flag(true);

      NetNet*res = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, width);
      res->local_flag(true);

      NetAddSub*add = new NetAddSub(scope, scope->local_symbol(), width);

      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(sig->pin(idx), add->pin_DataA(idx));

      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(val_c->pin(idx), add->pin_DataB(idx));

      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(val_s->pin(idx), add->pin_DataB(idx));

      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(res->pin(idx), add->pin_Result(idx));

      if (val < 0)
	    add->attribute(perm_string::literal("LPM_Direction"), verinum("SUB"));
      else
	    add->attribute(perm_string::literal("LPM_Direction"), verinum("ADD"));

      des->add_node(add);
      des->add_node(val_c);

      return res;
#else
      cerr << sig->get_fileline() << ": XXXX: Forgot how to implement add_to_net" << endl;
      return 0;
#endif
}

NetNet* cast_to_int(Design*des, NetScope*scope, NetNet*src, unsigned wid)
{
      if (src->data_type() != IVL_VT_REAL)
	    return src;

      NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, wid);
      tmp->data_type(IVL_VT_LOGIC);
      tmp->set_line(*src);
      tmp->local_flag(true);

      NetCastInt*cast = new NetCastInt(scope, scope->local_symbol(), wid);
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
      tmp->data_type(IVL_VT_REAL);
      tmp->set_line(*src);
      tmp->local_flag(true);

      NetCastReal*cast = new NetCastReal(scope, scope->local_symbol(), src->get_signed());
      cast->set_line(*src);
      des->add_node(cast);

      connect(cast->pin(0), tmp->pin(0));
      connect(cast->pin(1), src->pin(0));

      return tmp;
}

/*
 * Add a signed constant to an existing expression. Generate a new
 * NetEBAdd node that has the input expression and an expression made
 * from the constant value.
 */
NetExpr* make_add_expr(NetExpr*expr, long val)
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

      verinum val_v (val);
      val_v.has_sign(true);

      if (expr->has_width()) {
	    val_v = verinum(val_v, expr->expr_width());
      }

      NetEConst*val_c = new NetEConst(val_v);
      val_c->set_line(*expr);

      NetEBAdd*res = new NetEBAdd(add_op, expr, val_c);
      res->set_line(*expr);

      return res;
}

NetExpr* make_sub_expr(long val, NetExpr*expr)
{
      verinum val_v (val, expr->expr_width());
      val_v.has_sign(true);
      NetEConst*val_c = new NetEConst(val_v);
      val_c->set_line(*expr);

      NetEBAdd*res = new NetEBAdd('-', val_c, expr);
      res->set_line(*expr);

      return res;
}

NetEConst* make_const_x(unsigned long wid)
{
      verinum xxx (verinum::Vx, wid);
      NetEConst*resx = new NetEConst(xxx);
      return resx;
}

NetExpr* condition_reduce(NetExpr*expr)
{
      if (expr->expr_width() == 1)
	    return expr;

      verinum zero (verinum::V0, expr->expr_width());

      NetEConst*ezero = new NetEConst(zero);
      ezero->cast_signed(expr->has_sign());
      ezero->set_line(*expr);
      ezero->set_width(expr->expr_width());

      NetEBComp*cmp = new NetEBComp('n', expr, ezero);
      cmp->cast_signed(false);
      cmp->set_line(*expr);

      return cmp;
}

NetExpr* elab_and_eval(Design*des, NetScope*scope,
		       const PExpr*pe, int expr_wid, int prune_width)
{
      NetExpr*tmp = pe->elaborate_expr(des, scope, expr_wid, false);
      if (tmp == 0) return 0;

      eval_expr(tmp, prune_width);

      return tmp;
}

void eval_expr(NetExpr*&expr, int prune_width)
{
      assert(expr);
      if (dynamic_cast<NetEConst*>(expr)) return;
      if (dynamic_cast<NetECReal*>(expr)) return;

      NetExpr*tmp = expr->eval_tree(prune_width);
      if (tmp != 0) {
	    tmp->set_line(*expr);
	    delete expr;
	    expr = tmp;
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
	    value = tmp->value().as_long();
	    return true;
      }

      if (NetECReal*rtmp = dynamic_cast<NetECReal*>(expr)) {
	    value = rtmp->value().as_double();
	    return true;
      }

      return false;
}

std::list<hname_t> eval_scope_path(Design*des, NetScope*scope,
				   const pform_name_t&path)
{
      list<hname_t> res;

      typedef pform_name_t::const_iterator pform_path_it;

      for (pform_path_it cur = path.begin() ; cur != path.end(); cur++) {
	    const name_component_t&comp = *cur;
	    if (comp.index.empty()) {
		  res.push_back(hname_t(comp.name));
		  continue;
	    }

	    assert(comp.index.size() == 1);
	    const index_component_t&index = comp.index.front();
	    assert(index.sel == index_component_t::SEL_BIT);

	    NetExpr*tmp = elab_and_eval(des, scope, index.msb, -1);
	    ivl_assert(*index.msb, tmp);

	    if (NetEConst*ctmp = dynamic_cast<NetEConst*>(tmp)) {
		  res.push_back(hname_t(comp.name, ctmp->value().as_long()));
		  delete ctmp;
		  continue;
	    } else {
		  cerr << index.msb->get_fileline() << ": error: "
		       << "Scope index expression is not constant: "
		       << *index.msb << endl;
		  des->errors += 1;
	    }

	    return res;
      }

      return res;
}

/*
 * Human readable version of op. Used in elaboration error messages.
 */
const char *human_readable_op(const char op)
{
	const char *type;
	switch (op) {
	        case '~': type = "~";  break;  // Negation

	        case '^': type = "^";  break;  // XOR
	        case 'X': type = "~^"; break;  // XNOR
	        case '&': type = "&";  break;  // Bitwise AND
	        case 'A': type = "~&"; break;  // NAND (~&)
	        case '|': type = "|";  break;  // Bitwise OR
	        case 'O': type = "~|"; break;  // NOR

	        case 'a': type = "&&"; break;  // Logical AND
	        case 'o': type = "||"; break;  // Logical OR

	        case 'E': type = "==="; break;  // Case equality
	        case 'N': type = "!=="; break;  // Case inequality

	        case 'l': type = "<<(<)"; break;  // Left shifts
	        case 'r': type = ">>";    break;  // Logical right shift
	        case 'R': type = ">>>";   break;  // Arithmetic right shift

		default: assert(0);
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
			res = C_1;
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
