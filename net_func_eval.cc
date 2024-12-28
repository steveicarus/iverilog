/*
 * Copyright (c) 2012-2024 Stephen Williams (steve@icarus.com)
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

# include  "netlist.h"
# include  "netmisc.h"
# include  "compiler.h"
# include  <typeinfo>
# include  "ivl_assert.h"

using namespace std;

/*
 * We only evaluate one function at a time, so to support the disable
 * statement, we just need to record the target block and then early
 * terminate each enclosing block or loop statement until we get back
 * to the target block.
 */
static const NetScope*disable = 0;
static bool loop_break;
static bool loop_continue;

static NetExpr* fix_assign_value(const NetNet*lhs, NetExpr*rhs)
{
      NetEConst*ce = dynamic_cast<NetEConst*>(rhs);
      if (ce == 0) return rhs;

      unsigned lhs_width = lhs->vector_width();
      unsigned rhs_width = rhs->expr_width();
      if (rhs_width < lhs_width) {
            rhs = pad_to_width(rhs, lhs_width, *rhs);
      } else if (rhs_width > lhs_width) {
            verinum value(ce->value(), lhs_width);
            ce = new NetEConst(value);
            ce->set_line(*rhs);
            delete rhs;
	    rhs = ce;
      }
      rhs->cast_signed(lhs->get_signed());
      return rhs;
}

NetExpr* NetFuncDef::evaluate_function(const LineInfo&loc, const std::vector<NetExpr*>&args) const
{
	// Make the context map.
      map<perm_string,LocalVar>::iterator ptr;
      map<perm_string,LocalVar>context_map;

      if (debug_eval_tree) {
	    cerr << loc.get_fileline() << ": NetFuncDef::evaluate_function: "
		 << "Evaluate function " << scope()->basename() << endl;
      }

	// Put the return value into the map...
      LocalVar&return_var = context_map[scope()->basename()];
      return_var.nwords = 0;
      return_var.value  = 0;

	// Load the input ports into the map...
      ivl_assert(loc, port_count() == args.size());
      for (size_t idx = 0 ; idx < port_count() ; idx += 1) {
	    const NetNet*pnet = port(idx);
	    perm_string aname = pnet->name();
	    LocalVar&input_var = context_map[aname];
	    input_var.nwords = 0;
	    input_var.value  = fix_assign_value(pnet, args[idx]);

	    if (debug_eval_tree) {
		  cerr << loc.get_fileline() << ": NetFuncDef::evaluate_function: "
		       << "   input " << aname << " = " << *args[idx] << endl;
	    }
      }

	// Ask the scope to collect definitions for local values. This
	// fills in the context_map with local variables held by the scope.
      scope()->evaluate_function_find_locals(loc, context_map);

	// Execute any variable initialization statements.
      if (const NetProc*init_proc = scope()->var_init())
	    init_proc->evaluate_function(loc, context_map);

      if (debug_eval_tree && proc_==0) {
	    cerr << loc.get_fileline() << ": NetFuncDef::evaluate_function: "
		 << "Function " << scope_path(scope())
		 << " has no statement?" << endl;
      }

	// Perform the evaluation. Note that if there were errors
	// when compiling the function definition, we may not have
	// a valid statement.
      bool flag = proc_ && proc_->evaluate_function(loc, context_map);

      if (debug_eval_tree && !flag) {
	    cerr << loc.get_fileline() << ": NetFuncDef::evaluate_function: "
		 << "Cannot evaluate " << scope_path(scope()) << "." << endl;
      }

	// Extract the result...
      ptr = context_map.find(scope()->basename());
      NetExpr*res = ptr->second.value;
      context_map.erase(ptr);

	// Cleanup the rest of the context.
      for (ptr = context_map.begin() ; ptr != context_map.end() ; ++ptr) {

	    unsigned nwords = ptr->second.nwords;
	    if (nwords > 0) {
		  NetExpr**array = ptr->second.array;
		  for (unsigned idx = 0 ; idx < nwords ; idx += 1) {
			delete array[idx];
		  }
		  delete [] ptr->second.array;
	    } else {
		  delete ptr->second.value;
	    }
      }

      if (disable) {
	    if (debug_eval_tree)
		  cerr << loc.get_fileline() << ": NetFuncDef::evaluate_function: "
		       << "disable of " << scope_path(disable)
		       << " trapped in function " << scope_path(scope())
		       << "." << endl;
	    ivl_assert(loc, disable==scope());
	    disable = 0;
      }

	// Done.
      if (flag) {
	    if (debug_eval_tree) {
		  cerr << loc.get_fileline() << ": NetFuncDef::evaluate_function: "
		       << "Evaluated to ";
		  if (res) cerr << *res;
		  else cerr << "<nil>";
		  cerr << endl;
	    }
	    return res;
      }

      if (debug_eval_tree) {
	    cerr << loc.get_fileline() << ": NetFuncDef::evaluate_function: "
		 << "Evaluation failed." << endl;
      }

      delete res;
      return 0;
}

void NetScope::evaluate_function_find_locals(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      for (map<perm_string,NetNet*>::const_iterator cur = signals_map_.begin()
		 ; cur != signals_map_.end() ; ++cur) {

	    const NetNet*tmp = cur->second;
	      // Skip ports, which are handled elsewhere.
	    if (tmp->port_type() != NetNet::NOT_A_PORT)
		  continue;

	    unsigned nwords = 0;
	    if (tmp->unpacked_dimensions() > 0)
		  nwords = tmp->unpacked_count();

	    LocalVar&local_var = context_map[tmp->name()];
	    local_var.nwords = nwords;

	    if (nwords > 0) {
		  NetExpr**array = new NetExpr*[nwords];
		  for (unsigned idx = 0 ; idx < nwords ; idx += 1) {
			array[idx] = 0;
		  }
		  local_var.array = array;
	    } else {
		  local_var.value = 0;
	    }

	    if (debug_eval_tree) {
		  cerr << loc.get_fileline() << ": debug: "
		       << "   (local) " << tmp->name()
		       << (nwords > 0 ? "[]" : "") << endl;
	    }
      }
}

NetExpr* NetExpr::evaluate_function(const LineInfo&,
				    map<perm_string,LocalVar>&) const
{
      cerr << get_fileline() << ": sorry: I don't know how to evaluate this expression at compile time." << endl;
      cerr << get_fileline() << ":      : Expression type:" << typeid(*this).name() << endl;

      return 0;
}

bool NetProc::evaluate_function(const LineInfo&,
				map<perm_string,LocalVar>&) const
{
      cerr << get_fileline() << ": sorry: I don't know how to evaluate this statement at compile time." << endl;
      cerr << get_fileline() << ":      : Statement type:" << typeid(*this).name() << endl;

      return false;
}

void NetAssign::eval_func_lval_op_real_(const LineInfo&loc,
					verireal&lv, const verireal&rv) const
{
      switch (op_) {
	  case '+':
	    lv = lv + rv;
	    break;
	  case '-':
	    lv = lv - rv;
	    break;
	  case '*':
	    lv = lv * rv;
	    break;
	  case '/':
	    lv = lv / rv;
	    break;
	  case '%':
	    lv = lv % rv;
	    break;
	  default:
	    cerr << "Illegal assignment operator: "
		 << human_readable_op(op_) << endl;
	    ivl_assert(loc, 0);
      }
}

void NetAssign::eval_func_lval_op_(const LineInfo&loc,
				   verinum&lv, verinum&rv) const
{
      unsigned lv_width = lv.len();
      bool lv_sign = lv.has_sign();
      switch (op_) {
	  case 'l':
	  case 'R':
	      // The left operand is self-determined.
	    break;
	  case 'r':
	      // The left operand is self-determined, but we need to
	      // cast it to unsigned to get a logical shift.
	    lv.has_sign(false);
	    break;
          default:
	      // The left operand must be cast to the expression type/size
	    lv.has_sign(rv.has_sign());
	    lv = cast_to_width(lv, rv.len());
      }
      switch (op_) {
	  case '+':
	    lv = lv + rv;
	    break;
	  case '-':
	    lv = lv - rv;
	    break;
	  case '*':
	    lv = lv * rv;
	    break;
	  case '/':
	    lv = lv / rv;
	    break;
	  case '%':
	    lv = lv % rv;
	    break;
	  case '&':
	    for (unsigned idx = 0 ; idx < lv.len() ; idx += 1)
		  lv.set(idx, lv[idx] & rv[idx]);
	    break;
	  case '|':
	    for (unsigned idx = 0 ; idx < lv.len() ; idx += 1)
		  lv.set(idx, lv[idx] | rv[idx]);
	    break;
	  case '^':
	    for (unsigned idx = 0 ; idx < lv.len() ; idx += 1)
		  lv.set(idx, lv[idx] ^ rv[idx]);
	    break;
	  case 'l':
	    lv = lv << rv.as_unsigned();
	    break;
	  case 'r':
	    lv = lv >> rv.as_unsigned();
	    break;
	  case 'R':
	    lv = lv >> rv.as_unsigned();
	    break;
	  default:
	    cerr << "Illegal assignment operator: "
		 << human_readable_op(op_) << endl;
	    ivl_assert(loc, 0);
      }
      lv = cast_to_width(lv, lv_width);
      lv.has_sign(lv_sign);
}

bool NetAssign::eval_func_lval_(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map,
				const NetAssign_*lval, NetExpr*rval_result) const
{
      map<perm_string,LocalVar>::iterator ptr = context_map.find(lval->name());
      ivl_assert(*this, ptr != context_map.end());

      LocalVar*var = & ptr->second;
      while (var->nwords == -1) {
	    ivl_assert(*this, var->ref);
	    var = var->ref;
      }

      NetExpr*old_lval;
      int word = 0;
      if (var->nwords > 0) {
	    NetExpr*word_result = lval->word()->evaluate_function(loc, context_map);
	    if (word_result == 0) {
		  delete rval_result;
		  return false;
	    }

	    NetEConst*word_const = dynamic_cast<NetEConst*>(word_result);
	    ivl_assert(loc, word_const);

	    if (!word_const->value().is_defined())
		  return true;

	    word = word_const->value().as_long();

	    if (word < 0 || word >= var->nwords)
		  return true;

	    old_lval = var->array[word];
      } else {
	    ivl_assert(*this, var->nwords == 0);
	    old_lval = var->value;
      }

      if (const NetExpr*base_expr = lval->get_base()) {
	    NetExpr*base_result = base_expr->evaluate_function(loc, context_map);
	    if (base_result == 0) {
		  delete rval_result;
		  return false;
	    }

	    NetEConst*base_const = dynamic_cast<NetEConst*>(base_result);
	    ivl_assert(loc, base_const);

	    long base = base_const->value().as_long();

	    if (old_lval == 0)
		  old_lval = make_const_x(lval->sig()->vector_width());

	    NetEConst*lval_const = dynamic_cast<NetEConst*>(old_lval);
	    ivl_assert(loc, lval_const);
	    verinum lval_v = lval_const->value();
	    NetEConst*rval_const = dynamic_cast<NetEConst*>(rval_result);
	    ivl_assert(loc, rval_const);
	    verinum rval_v = rval_const->value();

	    verinum lpart(verinum::Vx, lval->lwidth());
	    if (op_) {
		  for (unsigned idx = 0 ; idx < lpart.len() ; idx += 1) {
			long ldx = base + idx;
			if (ldx >= 0 && (unsigned long)ldx < lval_v.len())
			      lpart.set(idx, lval_v[ldx]);
		  }
		  eval_func_lval_op_(loc, lpart, rval_v);
	    } else {
		  lpart = cast_to_width(rval_v, lval->lwidth());
	    }
	    for (unsigned idx = 0 ; idx < lpart.len() ; idx += 1) {
		  long ldx = base + idx;
		  if (ldx >= 0 && (unsigned long)ldx < lval_v.len())
			lval_v.set(idx+base, lpart[idx]);
	    }

	    delete base_result;
	    delete rval_result;
	    rval_result = new NetEConst(lval_v);
      } else {
	    if (op_ == 0) {
		  rval_result = fix_assign_value(lval->sig(), rval_result);
	    } else if (dynamic_cast<NetECReal*>(rval_result)) {
		  NetECReal*lval_const = dynamic_cast<NetECReal*>(old_lval);
		  ivl_assert(loc, lval_const);
		  verireal lval_r = lval_const->value();
		  NetECReal*rval_const = dynamic_cast<NetECReal*>(rval_result);
		  ivl_assert(loc, rval_const);
		  verireal rval_r = rval_const->value();

		  eval_func_lval_op_real_(loc, lval_r, rval_r);

		  delete rval_result;
		  rval_result = new NetECReal(lval_r);
	    } else {
		  NetEConst*lval_const = dynamic_cast<NetEConst*>(old_lval);
		  ivl_assert(loc, lval_const);
		  verinum lval_v = lval_const->value();
		  NetEConst*rval_const = dynamic_cast<NetEConst*>(rval_result);
		  ivl_assert(loc, rval_const);
		  verinum rval_v = rval_const->value();

		  eval_func_lval_op_(loc, lval_v, rval_v);

		  delete rval_result;
		  rval_result = new NetEConst(lval_v);
	    }
      }

      if (old_lval)
	    delete old_lval;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetAssign::evaluate_function: "
		 << lval->name() << " = " << *rval_result << endl;
      }

      if (var->nwords > 0) {
	    var->array[word] = rval_result;
      } else {
	    ivl_assert(*this, var->nwords == 0);
	    var->value = rval_result;
      }

      return true;
}

bool NetAssign::evaluate_function(const LineInfo&loc,
				  map<perm_string,LocalVar>&context_map) const
{
      // Evaluate the r-value expression.
      const NetExpr*use_rval = rval();
      if (use_rval == 0)
	    return false;
      NetExpr*rval_result = use_rval->evaluate_function(loc, context_map);
      if (rval_result == 0)
	    return false;

	// Handle the easy case of a single variable on the LHS.
      if (l_val_count() == 1)
	    return eval_func_lval_(loc, context_map, l_val(0), rval_result);

	// If we get here, the LHS must be a concatenation, so we
	// expect the RHS to be a vector value.
      NetEConst*rval_const = dynamic_cast<NetEConst*>(rval_result);
      ivl_assert(*this, rval_const);

      if (op_) {
	    cerr << get_fileline() << ": sorry: Assignment operators "
		    "inside a constant function are not currently "
		    "supported if the LHS is a concatenation." << endl;
	    return false;
      }

      verinum rval_full = rval_const->value();
      delete rval_result;

      unsigned base = 0;
      for (unsigned ldx = 0 ; ldx < l_val_count() ; ldx += 1) {
	    const NetAssign_*lval = l_val(ldx);

	    verinum rval_part(verinum::Vx, lval->lwidth());
	    for (unsigned idx = 0 ; idx < rval_part.len() ; idx += 1)
		  rval_part.set(idx, rval_full[base+idx]);

	    bool flag = eval_func_lval_(loc, context_map, lval,
					new NetEConst(rval_part));
	    if (!flag) return false;

	    base += lval->lwidth();
      }

      return true;
}

/*
 * Evaluating a NetBlock in a function is a simple matter of
 * evaluating the statements in order.
 */
bool NetBlock::evaluate_function(const LineInfo&loc,
				 map<perm_string,LocalVar>&context_map) const
{
      if (last_ == 0) return true;

	// If we need to make a local scope, then this context map
	// will be filled in and used for statements within this block.
      map<perm_string,LocalVar>local_context_map;
      bool use_local_context_map = false;

      if (subscope_!=0) {
	      // First, copy the containing scope symbols into the new
	      // scope as references.
	    for (map<perm_string,LocalVar>::iterator cur = context_map.begin()
		       ; cur != context_map.end() ; ++cur) {
		  LocalVar&cur_var = local_context_map[cur->first];
		  cur_var.nwords = -1;
		  if (cur->second.nwords == -1)
			cur_var.ref = cur->second.ref;
		  else
			cur_var.ref = &cur->second;
	    }

	      // Now collect the new locals.
	    subscope_->evaluate_function_find_locals(loc, local_context_map);
	    use_local_context_map = true;

	      // Execute any variable initialization statements.
	    if (const NetProc*init_proc = subscope_->var_init())
		  init_proc->evaluate_function(loc, local_context_map);
      }

	// Now use the local context map if there is any local
	// context, or the containing context map.
      map<perm_string,LocalVar>&use_context_map = use_local_context_map? local_context_map : context_map;

      bool flag = true;
      NetProc*cur = last_;
      do {
	    cur = cur->next_;
	    if (debug_eval_tree) {
		  cerr << get_fileline() << ": NetBlock::evaluate_function: "
		       << "Execute statement (" << typeid(*cur).name()
		       << ") at " << cur->get_fileline() << "." << endl;
	    }

	    bool cur_flag = cur->evaluate_function(loc, use_context_map);
	    flag = flag && cur_flag;
      } while (cur != last_ && !disable && !loop_break && !loop_continue);

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetBlock::evaluate_function: "
		 << "subscope_=" << subscope_
		 << ", disable=" << disable
		 << ", flag=" << (flag?"true":"false") << endl;
      }

      if (disable == subscope_) disable = 0;

      return flag;
}

bool NetCase::evaluate_function_vect_(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      NetExpr*case_expr = expr_->evaluate_function(loc, context_map);
      if (case_expr == 0)
	    return false;

      NetEConst*case_const = dynamic_cast<NetEConst*> (case_expr);
      ivl_assert(loc, case_const);

      verinum case_val = case_const->value();
      delete case_expr;

      NetProc*default_statement = 0;

      for (unsigned cnt = 0 ; cnt < items_.size() ; cnt += 1) {
            const Item*item = &items_[cnt];

            if (item->guard == 0) {
                  default_statement = item->statement;
                  continue;
            }

            NetExpr*item_expr = item->guard->evaluate_function(loc, context_map);
            if (item_expr == 0)
                  return false;

            NetEConst*item_const = dynamic_cast<NetEConst*> (item_expr);
            ivl_assert(loc, item_const);

            verinum item_val = item_const->value();
            delete item_expr;

            ivl_assert(loc, item_val.len() == case_val.len());

            bool match = true;
            for (unsigned idx = 0 ; idx < item_val.len() ; idx += 1) {
                  verinum::V bit_a = case_val.get(idx);
                  verinum::V bit_b = item_val.get(idx);

                  if (bit_a == verinum::Vx && type_ == EQX) continue;
                  if (bit_b == verinum::Vx && type_ == EQX) continue;

                  if (bit_a == verinum::Vz && type_ != EQ) continue;
                  if (bit_b == verinum::Vz && type_ != EQ) continue;

                  if (bit_a != bit_b) {
                        match = false;
                        break;
                  }
            }
            if (!match) continue;

            return item->statement->evaluate_function(loc, context_map);
      }

      if (default_statement)
            return default_statement->evaluate_function(loc, context_map);

      return true;
}

bool NetCase::evaluate_function_real_(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      NetExpr*case_expr = expr_->evaluate_function(loc, context_map);
      if (case_expr == 0)
	    return false;

      NetECReal*case_const = dynamic_cast<NetECReal*> (case_expr);
      ivl_assert(loc, case_const);

      double case_val = case_const->value().as_double();
      delete case_expr;

      NetProc*default_statement = 0;

      for (unsigned cnt = 0 ; cnt < items_.size() ; cnt += 1) {
            const Item*item = &items_[cnt];

            if (item->guard == 0) {
                  default_statement = item->statement;
                  continue;
            }

            NetExpr*item_expr = item->guard->evaluate_function(loc, context_map);
            if (item_expr == 0)
                  return false;

            NetECReal*item_const = dynamic_cast<NetECReal*> (item_expr);
            ivl_assert(loc, item_const);

            double item_val = item_const->value().as_double();
            delete item_expr;

            if (item_val != case_val) continue;

            return item->statement->evaluate_function(loc, context_map);
      }

      if (default_statement)
            return default_statement->evaluate_function(loc, context_map);

      return true;
}

bool NetCase::evaluate_function(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      if (expr_->expr_type() == IVL_VT_REAL)
	    return evaluate_function_real_(loc, context_map);
      else
	    return evaluate_function_vect_(loc, context_map);
}

bool NetCondit::evaluate_function(const LineInfo&loc,
				  map<perm_string,LocalVar>&context_map) const
{
      NetExpr*cond = expr_->evaluate_function(loc, context_map);
      if (cond == 0) {
	    if (debug_eval_tree) {
		  cerr << get_fileline() << ": NetCondit::evaluate_function: "
		       << "Unable to evaluate condition (" << *expr_ <<")" << endl;
	    }
	    return false;
      }

      NetEConst*cond_const = dynamic_cast<NetEConst*> (cond);
      ivl_assert(loc, cond_const);

      long val = cond_const->value().as_long();
      delete cond;

      bool flag;

      if (val)
	      // The condition is true, so evaluate the if clause
	    flag = (if_ == 0) || if_->evaluate_function(loc, context_map);
      else
	      // The condition is false, so evaluate the else clause
	    flag = (else_ == 0) || else_->evaluate_function(loc, context_map);

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetCondit::evaluate_function: "
		 << "Finished, flag=" << (flag?"true":"false") << endl;
      }
      return flag;
}

bool NetDisable::evaluate_function(const LineInfo&,
				   map<perm_string,LocalVar>&) const
{
      disable = target_;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetDisable::evaluate_function: "
		 << "disable " << scope_path(disable) << endl;
      }

      return true;
}

bool NetBreak::evaluate_function(const LineInfo&,
			         map<perm_string, LocalVar>&) const
{
      loop_break = true;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetBreak::evaluate_function" << endl;
      }

      return true;
}

bool NetContinue::evaluate_function(const LineInfo&,
				    map<perm_string, LocalVar>&) const
{
      loop_continue = true;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetContinue::evaluate_function" << endl;
      }

      return true;
}

bool NetDoWhile::evaluate_function(const LineInfo&loc,
				   map<perm_string,LocalVar>&context_map) const
{
      bool flag = true;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetDoWhile::evaluate_function: "
		 << "Start loop" << endl;
      }

      while (!disable) {
	      // Evaluate the statement.
	    flag = proc_->evaluate_function(loc, context_map);
	    if (! flag)
		   break;

	    if (loop_break) {
		  loop_break = false;
		  break;
	    }

	    loop_continue = false;

	      // Evaluate the condition expression to try and get the
	      // condition for the loop.
	    NetExpr*cond = cond_->evaluate_function(loc, context_map);
	    if (cond == 0) {
		  flag = false;
		  break;
	    }

	    NetEConst*cond_const = dynamic_cast<NetEConst*> (cond);
	    ivl_assert(loc, cond_const);

	    long val = cond_const->value().as_long();
	    delete cond;

	      // If the condition is false, then the loop is done.
	    if (val == 0)
		  break;
      }

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetDoWhile::evaluate_function: "
		 << "Done loop, flag=" << (flag?"true":"false") << endl;
      }

      return flag;
}

bool NetForever::evaluate_function(const LineInfo&loc,
				   map<perm_string,LocalVar>&context_map) const
{
      bool flag = true;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: NetForever::evaluate_function: "
		 << "Start loop" << endl;
      }

      while (flag && !disable) {
	    flag = flag && statement_->evaluate_function(loc, context_map);

	    if (loop_break) {
		  loop_break = false;
		  break;
	    }

	    loop_continue = false;
      }

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: NetForever::evaluate_function: "
		 << "Done loop" << endl;
      }

      return flag;
}

/*
 * Process the for-loop to generate a value, as if this were in a function.
 */
bool NetForLoop::evaluate_function(const LineInfo&loc,
				   map<perm_string,LocalVar>&context_map) const
{
      bool flag = true;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetForLoop::evaluate_function: "
		<< "Evaluate the for look as a function." << endl;
      }

      if (init_statement_) {
	    bool tmp_flag = init_statement_->evaluate_function(loc, context_map);
	    flag &= tmp_flag;
      }

      while (flag && !disable) {
	    if (condition_) {
		  // Evaluate the condition expression to try and get the
		  // condition for the loop.
		  NetExpr*cond = condition_->evaluate_function(loc, context_map);
		  if (cond == nullptr) {
			flag = false;
			break;
		  }

		  NetEConst*cond_const = dynamic_cast<NetEConst*> (cond);
		  ivl_assert(loc, cond_const);

		  long val = cond_const->value().as_long();
		  delete cond;

		  // If the condition is false, then break;
		  if (val == 0)
			break;
	    }

	    bool tmp_flag = statement_->evaluate_function(loc, context_map);
	    flag &= tmp_flag;

	    if (disable)
		  break;

	    if (loop_break) {
		  loop_break = false;
		  break;
	    }

	    loop_continue = false;

	    if (step_statement_) {
		  tmp_flag = step_statement_->evaluate_function(loc, context_map);
		  flag &= tmp_flag;
	    }
      }

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetForLoop::evaluate_function: "
		<< "Done for-loop, flag=" << (flag?"true":"false") << endl;
      }

      return flag;
}

bool NetRepeat::evaluate_function(const LineInfo&loc,
				  map<perm_string,LocalVar>&context_map) const
{
      bool flag = true;

	// Evaluate the condition expression to try and get the
	// condition for the loop.
      NetExpr*count_expr = expr_->evaluate_function(loc, context_map);
      if (count_expr == 0) return false;

      NetEConst*count_const = dynamic_cast<NetEConst*> (count_expr);
      ivl_assert(loc, count_const);

      long count = count_const->value().as_long();
      delete count_expr;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: NetRepeat::evaluate_function: "
		 << "Repeating " << count << " times." << endl;
      }

      while ((count > 0) && flag && !disable) {
	    flag = flag && statement_->evaluate_function(loc, context_map);
	    count -= 1;

	    if (loop_break) {
		  loop_break = false;
		  break;
	    }

	    loop_continue = false;
      }

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: NetRepeat::evaluate_function: "
		 << "Finished loop" << endl;
      }

      return flag;
}

bool NetSTask::evaluate_function(const LineInfo&,
				 map<perm_string,LocalVar>&) const
{
	// system tasks within a constant function are ignored
      return true;
}

bool NetWhile::evaluate_function(const LineInfo&loc,
				 map<perm_string,LocalVar>&context_map) const
{
      bool flag = true;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetWhile::evaluate_function: "
		 << "Start loop" << endl;
      }

      while (flag && !disable) {
	      // Evaluate the condition expression to try and get the
	      // condition for the loop.
	    NetExpr*cond = cond_->evaluate_function(loc, context_map);
	    if (cond == 0) {
		  flag = false;
		  break;
	    }

	    NetEConst*cond_const = dynamic_cast<NetEConst*> (cond);
	    ivl_assert(loc, cond_const);

	    long val = cond_const->value().as_long();
	    delete cond;

	      // If the condition is false, then break.
	    if (val == 0)
		  break;

	      // The condition is true, so evaluate the statement
	      // another time.
	    bool tmp_flag = proc_->evaluate_function(loc, context_map);
	    if (! tmp_flag)
		  flag = false;

	    if (loop_break) {
		  loop_break = false;
		  break;
	    }

	    loop_continue = false;
      }

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": NetWhile::evaluate_function: "
		 << "Done loop, flag=" << (flag?"true":"false") << endl;
      }

      return flag;
}

NetExpr* NetEBinary::evaluate_function(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      NetExpr*lval = left_->evaluate_function(loc, context_map);
      NetExpr*rval = right_->evaluate_function(loc, context_map);

      if (lval == 0 || rval == 0) {
	    delete lval;
	    delete rval;
	    return 0;
      }

      NetExpr*res = eval_arguments_(lval, rval);
      delete lval;
      delete rval;
      return res;
}

NetExpr* NetEConcat::evaluate_function(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      vector<NetExpr*>vals(parms_.size());
      unsigned gap = 0;

      unsigned valid_vals = 0;
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {
            ivl_assert(*this, parms_[idx]);
            vals[idx] = parms_[idx]->evaluate_function(loc, context_map);
            if (vals[idx] == 0) continue;

            gap += vals[idx]->expr_width();

            valid_vals += 1;
      }

      NetExpr*res = 0;
      if (valid_vals == parms_.size()) {
            res = eval_arguments_(vals, gap);
      }
      for (unsigned idx = 0 ;  idx < vals.size() ;  idx += 1) {
            delete vals[idx];
      }
      return res;
}

NetExpr* NetEConst::evaluate_function(const LineInfo&,
				      map<perm_string,LocalVar>&) const
{
      NetEConst*res = new NetEConst(value_);
      res->set_line(*this);
      return res;
}

NetExpr* NetECReal::evaluate_function(const LineInfo&,
				      map<perm_string,LocalVar>&) const
{
      NetECReal*res = new NetECReal(value_);
      res->set_line(*this);
      return res;
}

NetExpr* NetESelect::evaluate_function(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      NetExpr*sub_exp = expr_->evaluate_function(loc, context_map);
      ivl_assert(loc, sub_exp);

      NetEConst*sub_const = dynamic_cast<NetEConst*> (sub_exp);
      ivl_assert(loc, sub_exp);

      verinum sub = sub_const->value();
      delete sub_exp;

      long base = 0;
      if (base_) {
	    NetExpr*base_val = base_->evaluate_function(loc, context_map);
	    ivl_assert(loc, base_val);

	    NetEConst*base_const = dynamic_cast<NetEConst*>(base_val);
	    ivl_assert(loc, base_const);

	    base = base_const->value().as_long();
	    delete base_val;
      } else {
	    sub.has_sign(has_sign());
	    sub = pad_to_width(sub, expr_width());
      }

      verinum res (verinum::Vx, expr_width());
      for (unsigned idx = 0 ; idx < res.len() ; idx += 1) {
	    long sdx = base + idx;
	    if (sdx >= 0 && (unsigned long)sdx < sub.len())
		  res.set(idx, sub[sdx]);
      }

      NetEConst*res_const = new NetEConst(res);
      return res_const;
}

NetExpr* NetESignal::evaluate_function(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      map<perm_string,LocalVar>::iterator ptr = context_map.find(name());
      if (ptr == context_map.end()) {
	    cerr << get_fileline() << ": error: Cannot evaluate " << name()
		 << " in this context." << endl;
	    return 0;
      }

	// Follow indirect references to the actual variable.
      LocalVar*var = & ptr->second;
      while (var->nwords == -1) {
	    ivl_assert(*this, var->ref);
	    var = var->ref;
      }

      NetExpr*value = 0;
      if (var->nwords > 0) {
	    ivl_assert(loc, word_);
	    NetExpr*word_result = word_->evaluate_function(loc, context_map);
	    if (word_result == 0)
		  return 0;

	    NetEConst*word_const = dynamic_cast<NetEConst*>(word_result);
	    ivl_assert(loc, word_const);

	    int word = word_const->value().as_long();

	    if (word_const->value().is_defined() && (word >= 0) && (word < var->nwords))
		  value = var->array[word];
      } else {
	    value = var->value;
      }

      if (value == 0) {
	    switch (expr_type()) {
		case IVL_VT_REAL:
		  return new NetECReal( verireal(0.0) );
		case IVL_VT_BOOL:
		  return make_const_0(expr_width());
		case IVL_VT_LOGIC:
		  return make_const_x(expr_width());
		default:
		  cerr << get_fileline() << ": sorry: I don't know how to initialize " << *this << endl;
		  return 0;
	    }
      }

      return value->dup_expr();
}

NetExpr* NetETernary::evaluate_function(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      unique_ptr<NetExpr> cval (cond_->evaluate_function(loc, context_map));

      switch (const_logical(cval.get())) {

	  case C_0:
	    return false_val_->evaluate_function(loc, context_map);
	  case C_1:
	    return true_val_->evaluate_function(loc, context_map);
	  case C_X:
	    break;
	  default:
	    cerr << get_fileline() << ": error: Condition expression is not constant here." << endl;
	    return 0;
      }

      NetExpr*tval = true_val_->evaluate_function(loc, context_map);
      NetExpr*fval = false_val_->evaluate_function(loc, context_map);

      NetExpr*res = blended_arguments_(tval, fval);
      delete tval;
      delete fval;
      return res;
}

NetExpr* NetEUnary::evaluate_function(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      NetExpr*val = expr_->evaluate_function(loc, context_map);
      if (val == 0) return 0;

      NetExpr*res = eval_arguments_(val);
      delete val;
      return res;
}

NetExpr* NetESFunc::evaluate_function(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      ID id = built_in_id_();
      ivl_assert(*this, id != NOT_BUILT_IN);

      NetExpr*val0 = 0;
      NetExpr*val1 = 0;
      NetExpr*res = 0;
      switch (parms_.size()) {
	  case 1:
	    val0 = parms_[0]->evaluate_function(loc, context_map);
	    if (val0 == 0) break;
	    res = evaluate_one_arg_(id, val0);
	    break;
	  case 2:
	    val0 = parms_[0]->evaluate_function(loc, context_map);
	    val1 = parms_[1]->evaluate_function(loc, context_map);
	    if (val0 == 0 || val1 == 0) break;
	    res = evaluate_two_arg_(id, val0, val1);
	    break;
	  default:
	    ivl_assert(*this, 0);
	    break;
      }
      delete val0;
      delete val1;
      return res;
}

NetExpr* NetEUFunc::evaluate_function(const LineInfo&loc,
				map<perm_string,LocalVar>&context_map) const
{
      NetFuncDef*def = func_->func_def();
      ivl_assert(*this, def);

      vector<NetExpr*>args(parms_.size());
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1)
	    args[idx] = parms_[idx]->evaluate_function(loc, context_map);

      NetExpr*res = def->evaluate_function(*this, args);
      return res;
}
