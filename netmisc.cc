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

NetNet* sub_net_from(Design*des, NetScope*scope, long val, NetNet*sig)
{
      NetNet*zero_net = new NetNet(scope, scope->local_symbol(),
				   NetNet::WIRE, sig->vector_width());
      zero_net->data_type(sig->data_type());
      zero_net->local_flag(true);

      if (sig->data_type() == IVL_VT_REAL) {
	    verireal zero (val);
	    NetLiteral*zero_obj = new NetLiteral(scope, scope->local_symbol(), zero);
	    des->add_node(zero_obj);

	    connect(zero_net->pin(0), zero_obj->pin(0));

      } else {
	    verinum zero ((int64_t)val);
	    zero = pad_to_width(zero, sig->vector_width());
	    NetConst*zero_obj = new NetConst(scope, scope->local_symbol(), zero);
	    des->add_node(zero_obj);

	    connect(zero_net->pin(0), zero_obj->pin(0));
      }

      NetAddSub*adder = new NetAddSub(scope, scope->local_symbol(), sig->vector_width());
      des->add_node(adder);
      adder->attribute(perm_string::literal("LPM_Direction"), verinum("SUB"));

      connect(zero_net->pin(0), adder->pin_DataA());
      connect(adder->pin_DataB(), sig->pin(0));

      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, sig->vector_width());
      tmp->data_type(sig->data_type());
      tmp->local_flag(true);

      connect(adder->pin_Result(), tmp->pin(0));

      return tmp;
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

NetEConst* make_const_0(unsigned long wid)
{
      verinum xxx (verinum::V0, wid);
      NetEConst*resx = new NetEConst(xxx);
      return resx;
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

void probe_expr_width(Design*des, NetScope*scope, PExpr*pe)
{
      ivl_variable_type_t expr_type = IVL_VT_NO_TYPE;
      bool flag = false;
      pe->test_width(des, scope, 0, 0, expr_type, flag);
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

      for (pform_path_it cur = path.begin() ; cur != path.end(); cur++) {
	    const name_component_t&comp = *cur;
	    res.push_back( eval_path_component(des,scope,comp) );
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

	    case '+': type = "+";  break;
	    case '-': type = "-";  break;
	    case '*': type = "*";  break;
	    case '/': type = "/";  break;

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
	    case 'N': type = "!=="; break;  // Case inequality

	    case 'l': type = "<<(<)"; break;  // Left shifts
	    case 'r': type = ">>";    break;  // Logical right shift
	    case 'R': type = ">>>";   break;  // Arithmetic right shift

	    case 'p': type = "**"; break; // Power
	    default:
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
