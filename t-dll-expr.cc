/*
 * Copyright (c) 2000-2021 Stephen Williams (steve@icarus.com)
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

# include  <iostream>

# include  <cstring>
# include  "t-dll.h"
# include  "netlist.h"
# include  "netclass.h"
# include  <cassert>
# include  <cstdlib>
# include  "ivl_alloc.h"
# include  "ivl_assert.h"

using namespace std;

/*
 * This is a little convenience function for converting a NetExpr
 * expression type to the expression type used by ivl_expr_t objects.
 */
static ivl_variable_type_t get_expr_type(const NetExpr*net)
{
      return net->expr_type();
}

/*
 * These methods implement the expression scan that generates the
 * ivl_expr_t representing the expression. Each method leaves the
 * expr_ member filled with the ivl_expr_t that represents it. Each
 * method expects that the expr_ member empty (0) when it starts.
 */

/*
 * This function takes an expression in the expr_ member that is
 * already built up, and adds a subtraction of the given constant.
 */
void dll_target::sub_off_from_expr_(long off)
{
      assert(expr_ != 0);

      char*bits;
      ivl_expr_t tmpc = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      tmpc->type_   = IVL_EX_NUMBER;
      tmpc->value_  = IVL_VT_VECTOR;
      tmpc->net_type= 0;
      tmpc->width_  = expr_->width_;
      tmpc->signed_ = expr_->signed_;
      tmpc->sized_  = 1;
      tmpc->u_.number_.bits_ = bits = (char*)malloc(tmpc->width_);
      for (unsigned idx = 0 ;  idx < tmpc->width_ ;  idx += 1) {
	    bits[idx] = (off & 1)? '1' : '0';
	    off >>= 1;
      }

	/* Now make the subtracter (x-4 in the above example)
	   that has as input A the index expression and input B
	   the constant to subtract. */
      ivl_expr_t tmps = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      tmps->type_  = IVL_EX_BINARY;
      tmps->value_ = IVL_VT_VECTOR;
      tmps->net_type= 0;
      tmps->width_ = tmpc->width_;
      tmps->signed_ = tmpc->signed_;
      tmps->sized_ = 1;
      tmps->u_.binary_.op_  = '-';
      tmps->u_.binary_.lef_ = expr_;
      tmps->u_.binary_.rig_ = tmpc;

	/* Replace (x) with (x-off) */
      expr_ = tmps;
}

void dll_target::mul_expr_by_const_(long val)
{
      assert(expr_ != 0);

      char*bits;
      ivl_expr_t tmpc = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      tmpc->type_   = IVL_EX_NUMBER;
      tmpc->value_  = IVL_VT_VECTOR;
      tmpc->net_type= 0;
      tmpc->width_  = expr_->width_;
      tmpc->signed_ = expr_->signed_;
      tmpc->sized_  = 1;
      tmpc->u_.number_.bits_ = bits = (char*)malloc(tmpc->width_);
      for (unsigned idx = 0 ;  idx < tmpc->width_ ;  idx += 1) {
	    bits[idx] = (val & 1)? '1' : '0';
	    val >>= 1;
      }

	/* Now make the subtracter (x-4 in the above example)
	   that has as input A the index expression and input B
	   the constant to subtract. */
      ivl_expr_t tmps = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      tmps->type_  = IVL_EX_BINARY;
      tmps->value_ = IVL_VT_VECTOR;
      tmpc->net_type= 0;
      tmps->width_ = tmpc->width_;
      tmps->signed_ = tmpc->signed_;
      tmps->sized_ = 1;
      tmps->u_.binary_.op_  = '*';
      tmps->u_.binary_.lef_ = expr_;
      tmps->u_.binary_.rig_ = tmpc;

	/* Replace (x) with (x*valf) */
      expr_ = tmps;
}

ivl_expr_t dll_target::expr_from_value_(const verinum&val)
{
      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      unsigned idx;
      char*bits;
      expr->type_ = IVL_EX_NUMBER;
      expr->value_= IVL_VT_VECTOR;
      expr->net_type=0;
      expr->width_= val.len();
      expr->signed_ = val.has_sign()? 1 : 0;
      expr->sized_= 1;
      expr->u_.number_.bits_ = bits = (char*)malloc(expr->width_ + 1);
      for (idx = 0 ;  idx < expr->width_ ;  idx += 1)
	    switch (val.get(idx)) {
		case verinum::V0:
		  bits[idx] = '0';
		  break;
		case verinum::V1:
		  bits[idx] = '1';
		  break;
		case verinum::Vx:
		  bits[idx] = 'x';
		  break;
		case verinum::Vz:
		  bits[idx] = 'z';
		  break;
		default:
		  assert(0);
	    }

      bits[expr->width_] = 0;

      return expr;
}

void dll_target::expr_access_func(const NetEAccess*net)
{
      assert(expr_ == 0);
	// Make a stub Branch Access Function expression node.
      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_->type_  = IVL_EX_BACCESS;
      expr_->value_ = IVL_VT_REAL;
      expr_->net_type=0;
      expr_->width_ = 1;
      expr_->signed_= 1;
      expr_->sized_ = 1;
      FILE_NAME(expr_, net);

      expr_->u_.branch_.branch = net->get_branch()->target_obj();
      expr_->u_.branch_.nature = net->get_nature();
}

void dll_target::expr_array_pattern(const NetEArrayPattern*net)
{
      assert(expr_ == 0);
      ivl_expr_t expr_tmp = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_tmp->type_ = IVL_EX_ARRAY_PATTERN;
      expr_tmp->value_= net->expr_type();
      expr_tmp->net_type = net->net_type();
      expr_tmp->width_   = 1;
      expr_tmp->signed_  = 0;
      expr_tmp->sized_   = 0;
      FILE_NAME(expr_tmp, net);

      expr_tmp->u_.array_pattern_.parms = net->item_size();
      expr_tmp->u_.array_pattern_.parm = new ivl_expr_t [net->item_size()];

      for (size_t idx = 0 ; idx < net->item_size() ; idx += 1) {
	    const NetExpr*tmp = net->item(idx);
	    tmp->expr_scan(this);
	    expr_tmp->u_.array_pattern_.parm[idx] = expr_;
	    expr_ = 0;
      }

      expr_ = expr_tmp;
}

void dll_target::expr_binary(const NetEBinary*net)
{
      assert(expr_ == 0);

      net->left()->expr_scan(this);
      ivl_expr_t left = expr_;

      expr_ = 0;
      net->right()->expr_scan(this);
      ivl_expr_t rght = expr_;

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      expr_->type_ = IVL_EX_BINARY;
      expr_->value_= get_expr_type(net);
      expr_->net_type=0;
      expr_->width_= net->expr_width();
      expr_->signed_ = net->has_sign()? 1 : 0;
      expr_->sized_= 1;
      FILE_NAME(expr_, net);

      expr_->u_.binary_.op_ = net->op();
      expr_->u_.binary_.lef_ = left;
      expr_->u_.binary_.rig_ = rght;
}

void dll_target::expr_concat(const NetEConcat*net)
{
      assert(expr_ == 0);

      ivl_expr_t cur = new struct ivl_expr_s;
      assert(cur);

      cur->type_  = IVL_EX_CONCAT;
      cur->value_ = net->expr_type();
      cur->net_type=0;
      cur->width_ = net->expr_width();
      cur->signed_ = net->has_sign() ? 1 : 0;
      cur->sized_ = 1;
      FILE_NAME(cur, net);

      cur->u_.concat_.rept  = net->repeat();
      cur->u_.concat_.parms = net->nparms();
      cur->u_.concat_.parm  = new ivl_expr_t [net->nparms()];

      for (unsigned idx = 0 ;  idx < net->nparms() ;  idx += 1) {
	    expr_ = 0;
	    net->parm(idx)->expr_scan(this);
	    assert(expr_);
	    cur->u_.concat_.parm[idx] = expr_;
      }

      expr_ = cur;
}

void dll_target::expr_const(const NetEConst*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_->value_= net->expr_type();
      expr_->net_type=0;
      FILE_NAME(expr_, net);

      if (net->value().is_string()) {
	    expr_->type_ = IVL_EX_STRING;
	    expr_->width_= net->expr_width();
	    expr_->u_.string_.value_ =strdup(net->value().as_string().c_str());

      } else {
	    verinum val = net->value();
	    unsigned idx;
	    char*bits;
	    expr_->type_ = IVL_EX_NUMBER;
	    expr_->width_= net->expr_width();
	    expr_->signed_ = net->has_sign()? 1 : 0;
	    expr_->sized_= net->has_width()? 1 : 0;
	    expr_->u_.number_.bits_ = bits = (char*)malloc(expr_->width_);
	    for (idx = 0 ;  idx < expr_->width_ ;  idx += 1)
		  switch (val.get(idx)) {
		      case verinum::V0:
			bits[idx] = '0';
			break;
		      case verinum::V1:
			bits[idx] = '1';
			break;
		      case verinum::Vx:
			bits[idx] = 'x';
			break;
		      case verinum::Vz:
			bits[idx] = 'z';
			break;
		      default:
			assert(0);
		  }

      }
}

void dll_target::expr_param(const NetEConstParam*net)
{
      ivl_scope_t scop = find_scope(des_, net->scope());
      ivl_parameter_t par = scope_find_param(scop, net->name());

      if (par == 0) {
	    cerr << net->get_fileline() << ": internal error: "
		 << "Parameter " << net->name() << " missing from "
		 << ivl_scope_name(scop) << endl;
      }
      assert(par);
      expr_const(net);
      expr_->u_.string_.parameter = par;
}

void dll_target::expr_rparam(const NetECRealParam*net)
{
      ivl_scope_t scop = find_scope(des_, net->scope());
      ivl_parameter_t par = scope_find_param(scop, net->name());

      if (par == 0) {
	    cerr << net->get_fileline() << ": internal error: "
		 << "Parameter " << net->name() << " missing from "
		 << ivl_scope_name(scop) << endl;
      }
      assert(par);
      assert(par->value);
      expr_ = par->value;
}

void dll_target::expr_creal(const NetECReal*net)
{
      assert(expr_ == 0);
      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_->width_  = net->expr_width();
      expr_->signed_ = 1;
      expr_->sized_  = 1;
      expr_->type_ = IVL_EX_REALNUM;
      FILE_NAME(expr_, net);
      expr_->value_= IVL_VT_REAL;
      expr_->net_type=0;
      expr_->u_.real_.value = net->value().as_double();
}

void dll_target::expr_last(const NetELast*net)
{
      assert(expr_ == 0);
      ivl_expr_t expr = new struct ivl_expr_s;
      expr->type_   = IVL_EX_SFUNC;
      expr->value_  = IVL_VT_LOGIC;
      expr->width_  = 32;
      expr->signed_ = 1;
      expr->sized_  = 1;
      expr->net_type  = 0;
      FILE_NAME(expr, net);

      expr->u_.sfunc_.name_ = "$high";

      ivl_signal_t sig = find_signal(des_, net->sig());

      ivl_expr_t esig = new struct ivl_expr_s;
      esig->type_   = IVL_EX_SIGNAL;
      esig->value_  = IVL_VT_DARRAY;
      esig->net_type= sig->net_type;
      esig->width_  = 1;
      esig->signed_ = sig->net_type->get_signed()? 1 : 0;
      FILE_NAME(esig, net);
      esig->u_.signal_.word = 0;
      esig->u_.signal_.sig = sig;

      expr->u_.sfunc_.parms = 1;
      expr->u_.sfunc_.parm = new ivl_expr_t[1];
      expr->u_.sfunc_.parm[0] = esig;

      expr_ = expr;
}

void dll_target::expr_new(const NetENew*net)
{
      ivl_expr_t size = 0;
      ivl_expr_t init_val = 0;

      if (net->size_expr()) {
	    net->size_expr()->expr_scan(this);
	    size = expr_;
	    expr_ = 0;
      }

      if (net->init_expr()) {
	    net->init_expr()->expr_scan(this);
	    init_val = expr_;
	    expr_ = 0;
      }

      assert(expr_ == 0);
      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_->width_  = net->expr_width();
      expr_->signed_ = 0;
      expr_->sized_  = 1;
      expr_->type_   = IVL_EX_NEW;
      FILE_NAME(expr_, net);
      expr_->value_  = net->expr_type(); // May be IVL_VT_DARRAY or _CLASS
      expr_->net_type= net->net_type();
      expr_->u_.new_.size = size;
      expr_->u_.new_.init_val = init_val;
}

void dll_target::expr_null(const NetENull*net)
{
      assert(expr_ == 0);
      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_->width_  = net->expr_width();
      expr_->signed_ = 0;
      expr_->sized_  = 1;
      expr_->type_   = IVL_EX_NULL;
      FILE_NAME(expr_, net);
      expr_->value_  = IVL_VT_CLASS;
      expr_->net_type= 0;
}

void dll_target::expr_property(const NetEProperty*net)
{
      ivl_expr_t index = 0;
      if (const NetExpr*index_expr = net->get_index()) {
	    index_expr->expr_scan(this);
	    index = expr_;
	    expr_ = 0;
      }
      assert(expr_ == 0);
      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_->width_  = net->expr_width();
      expr_->signed_ = net->has_sign();
      expr_->sized_  = 1;
      expr_->type_   = IVL_EX_PROPERTY;
      FILE_NAME(expr_, net);
      expr_->value_  = net->expr_type();
      expr_->net_type= net->net_type();
      expr_->u_.property_.sig = find_signal(des_, net->get_sig());
      expr_->u_.property_.prop_idx = net->property_idx();
      expr_->u_.property_.index = index;
}

void dll_target::expr_event(const NetEEvent*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      expr_->type_ = IVL_EX_EVENT;
      FILE_NAME(expr_, net);
      expr_->value_= IVL_VT_VOID;
      expr_->net_type=0;

        /* Locate the event by name. Save the ivl_event_t in the
           expression so that the generator can find it easily. */
      const NetEvent*ev = net->event();
      ivl_scope_t ev_scope = lookup_scope_(ev->scope());

      for (unsigned idx = 0 ;  idx < ev_scope->nevent_ ;  idx += 1) {
            const char*ename = ivl_event_basename(ev_scope->event_[idx]);
            if (strcmp(ev->name(), ename) == 0) {
                  expr_->u_.event_.event = ev_scope->event_[idx];
                  break;
            }
      }
}

void dll_target::expr_scope(const NetEScope*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      expr_->type_ = IVL_EX_SCOPE;
      FILE_NAME(expr_, net);
      expr_->value_= IVL_VT_VOID;
      expr_->net_type=0;
      expr_->u_.scope_.scope = lookup_scope_(net->scope());
}

void dll_target::expr_scopy(const NetEShallowCopy*net)
{
      assert(expr_ == 0);

      net->expr_scan_oper1(this);
      ivl_expr_t expr1 = expr_;
      expr_ = 0;

      net->expr_scan_oper2(this);
      ivl_expr_t expr2 = expr_;
      expr_ = 0;

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_->type_ = IVL_EX_SHALLOWCOPY;
      FILE_NAME(expr_, net);
      expr_->value_ = net->expr_type();
      expr_->net_type = net->net_type();

      expr_->u_.shallow_.dest = expr1;
      expr_->u_.shallow_.src  = expr2;
}

void dll_target::expr_netenum(const NetENetenum*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      expr_->type_ = IVL_EX_ENUMTYPE;
      FILE_NAME(expr_, net);
      expr_->value_= IVL_VT_VOID;
      expr_->net_type=0;
      expr_->u_.enumtype_.type = net->netenum();
}

void dll_target::expr_select(const NetESelect*net)
{
      assert(expr_ == 0);

      net->sub_expr()->expr_scan(this);
      ivl_expr_t expr = expr_;

      expr_ = 0;
      if (net->select())
	    net->select()->expr_scan(this);

      ivl_expr_t base = expr_;

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      expr_->type_ = IVL_EX_SELECT;
      expr_->value_= net->expr_type();
      expr_->net_type=0;
      expr_->width_= net->expr_width();
      expr_->signed_ = net->has_sign()? 1 : 0;
      expr_->sized_= 1;
      FILE_NAME(expr_, net);

      expr_->u_.select_.sel_type_ = net->select_type();
      expr_->u_.select_.expr_ = expr;
      expr_->u_.select_.base_ = base;
}

void dll_target::expr_sfunc(const NetESFunc*net)
{
      assert(expr_ == 0);

      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      expr->type_ = IVL_EX_SFUNC;
      expr->value_= net->expr_type();
      expr->net_type=net->net_type();
      expr->width_= net->expr_width();
      expr->signed_ = net->has_sign()? 1 : 0;
      expr->sized_= 1;
      FILE_NAME(expr, net);
	/* system function names are lex_strings strings. */
      expr->u_.sfunc_.name_ = net->name();

      unsigned cnt = net->nparms();
      expr->u_.sfunc_.parms = cnt;
      expr->u_.sfunc_.parm = new ivl_expr_t[cnt];

	/* make up the parameter expressions. */
      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
	    net->parm(idx)->expr_scan(this);
	    assert(expr_);
	    expr->u_.sfunc_.parm[idx] = expr_;
	    expr_ = 0;
      }

      expr_ = expr;
}

void dll_target::expr_ternary(const NetETernary*net)
{
      assert(expr_ == 0);

      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      expr->type_  = IVL_EX_TERNARY;
      expr->value_= net->expr_type();
      expr->net_type=0;
      expr->width_ = net->expr_width();
      expr->signed_ = net->has_sign()? 1 : 0;
      expr->sized_ = 1;
      FILE_NAME(expr, net);

      net->cond_expr()->expr_scan(this);
      assert(expr_);
      expr->u_.ternary_.cond = expr_;
      expr_ = 0;

      net->true_expr()->expr_scan(this);
      assert(expr_);
      expr->u_.ternary_.true_e = expr_;
      expr_ = 0;

      net->false_expr()->expr_scan(this);
      assert(expr_);
      expr->u_.ternary_.false_e = expr_;

      expr_ = expr;
}

void dll_target::expr_signal(const NetESignal*net)
{
      ivl_signal_t sig = find_signal(des_, net->sig());

      assert(expr_ == 0);

	/* If there is a word expression, generate it. */
      ivl_expr_t word_expr = 0;
      if (const NetExpr*word = net->word_index()) {
	    word->expr_scan(this);
	    assert(expr_);
	    word_expr = expr_;
	    expr_ = 0;
      }

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      expr_->type_ = IVL_EX_SIGNAL;
      expr_->value_= net->expr_type();
      expr_->net_type=0;
      expr_->width_= net->expr_width();
      expr_->signed_ = net->has_sign()? 1 : 0;
      expr_->sized_= 1;
      FILE_NAME(expr_, net);
      expr_->u_.signal_.word = word_expr;
      expr_->u_.signal_.sig = sig;

	/* Make account for the special case that this is a reference
	   to an array as a whole. We detect this case by noting that
	   this is an array (more than 0 array dimensions) and that
	   there is no word select expression. For this case, we have
	   an IVL_EX_ARRAY expression instead of a SIGNAL expression. */
      if (sig->array_dimensions_ > 0 && word_expr == 0) {
	    expr_->type_ = IVL_EX_ARRAY;
	    expr_->width_ = 0; // Doesn't make much sense for arrays.
      }
}


void dll_target::expr_ufunc(const NetEUFunc*net)
{
      assert(expr_ == 0);

      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));

      expr->type_ = IVL_EX_UFUNC;
      expr->value_= net->expr_type();
      expr->net_type=0;
      expr->width_= net->expr_width();
      expr->signed_ = net->has_sign()? 1 : 0;
      expr->sized_= 1;
      FILE_NAME(expr, net);

      expr->u_.ufunc_.def = lookup_scope_(net->func());
      if (expr->u_.ufunc_.def == 0) {
	    cerr << net->get_fileline() << ": internal error: "
		 << "dll_target::expr_ufunc: "
		 << "Unable to match scope " << scope_path(net->func()) << endl;
      }
      ivl_assert(*net, expr->u_.ufunc_.def);
      ivl_assert(*net, expr->u_.ufunc_.def->type_ == IVL_SCT_FUNCTION);

      unsigned cnt = net->parm_count();
      expr->u_.ufunc_.parms = cnt;
      expr->u_.ufunc_.parm = new ivl_expr_t[cnt];

	/* make up the parameter expressions. */
      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
	    net->parm(idx)->expr_scan(this);
	    assert(expr_);
	    expr->u_.ufunc_.parm[idx] = expr_;
	    expr_ = 0;
      }

      expr_ = expr;
}

void dll_target::expr_unary(const NetEUnary*net)
{
      assert(expr_ == 0);

      net->expr()->expr_scan(this);
      assert(expr_);

      ivl_expr_t sub = expr_;

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_->type_ = IVL_EX_UNARY;
      expr_->value_= net->expr_type();
      expr_->net_type=0;
      expr_->width_ = net->expr_width();
      expr_->signed_ = net->has_sign()? 1 : 0;
      expr_->sized_ = 1;
      FILE_NAME(expr_, net);
      expr_->u_.unary_.op_ = net->op();
      expr_->u_.unary_.sub_ = sub;
}
