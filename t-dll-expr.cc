/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: t-dll-expr.cc,v 1.39.2.2 2006/09/15 23:56:05 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

# include  "t-dll.h"
# include  "netlist.h"
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>

/*
 * This is a little convenience function for converting a NetExpr
 * expression type to the expression type used by ivl_expr_t objects.
 */
static ivl_variable_type_t get_expr_type(const NetExpr*net)
{
      switch (net->expr_type()) {
	  case NetExpr::ET_VOID:
	    return IVL_VT_VOID;

	  case NetExpr::ET_REAL:
	    return IVL_VT_REAL;

	  case NetExpr::ET_VECTOR:
	    return IVL_VT_VECTOR;
      }

      return IVL_VT_VOID;
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
      tmpc->width_  = expr_->width_;
      tmpc->signed_ = expr_->signed_;
      tmpc->u_.number_.bits_ = bits = (char*)malloc(tmpc->width_);
      for (unsigned idx = 0 ;  idx < tmpc->width_ ;  idx += 1) {
	    bits[idx] = (off & 1)? '1' : '0';
	    off >>= 1;
      }

	/* Now make the subtractor (x-4 in the above example)
	   that has as input A the index expression and input B
	   the constant to subtract. */
      ivl_expr_t tmps = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      tmps->type_  = IVL_EX_BINARY;
      tmps->value_ = IVL_VT_VECTOR;
      tmps->width_ = tmpc->width_;
      tmps->signed_ = tmpc->signed_;
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
      tmpc->width_  = expr_->width_;
      tmpc->signed_ = expr_->signed_;
      tmpc->u_.number_.bits_ = bits = (char*)malloc(tmpc->width_);
      for (unsigned idx = 0 ;  idx < tmpc->width_ ;  idx += 1) {
	    bits[idx] = (val & 1)? '1' : '0';
	    val >>= 1;
      }

	/* Now make the subtractor (x-4 in the above example)
	   that has as input A the index expression and input B
	   the constant to subtract. */
      ivl_expr_t tmps = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      tmps->type_  = IVL_EX_BINARY;
      tmps->value_ = IVL_VT_VECTOR;
      tmps->width_ = tmpc->width_;
      tmps->signed_ = tmpc->signed_;
      tmps->u_.binary_.op_  = '*';
      tmps->u_.binary_.lef_ = expr_;
      tmps->u_.binary_.rig_ = tmpc;

	/* Replace (x) with (x*valf) */
      expr_ = tmps;
}

ivl_expr_t dll_target::expr_from_value_(const verinum&val)
{
      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr);

      unsigned idx;
      char*bits;
      expr->type_ = IVL_EX_NUMBER;
      expr->value_= IVL_VT_VECTOR;
      expr->width_= val.len();
      expr->signed_ = val.has_sign()? 1 : 0;
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

void dll_target::expr_binary(const NetEBinary*net)
{
      assert(expr_ == 0);

      net->left()->expr_scan(this);
      ivl_expr_t left = expr_;

      expr_ = 0;
      net->right()->expr_scan(this);
      ivl_expr_t rght = expr_;

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr_);

      expr_->type_ = IVL_EX_BINARY;
      expr_->value_= get_expr_type(net);
      expr_->width_= net->expr_width();
      expr_->signed_ = net->has_sign()? 1 : 0;

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
      cur->value_ = IVL_VT_VECTOR;
      cur->width_ = net->expr_width();

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

void dll_target::expr_memory(const NetEMemory*net)
{
      assert(expr_ == 0);
      if (net->index()) {
	    net->index()->expr_scan(this);
	    assert(expr_);
      }

      ivl_expr_t cur = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(cur);

      const NetMemory*mem = net->memory();

      if (const NetNet*reg = mem->reg_from_explode()) {

	    if (expr_ == 0) {
		    // If there is no index expression for the
		    // exploded memory, then replace it with the
		    // entire exploded reg.
		  cur->type_ = IVL_EX_SIGNAL;
		  cur->value_ = IVL_VT_VECTOR;
		  cur->width_= reg->pin_count();
		  cur->signed_ = net->has_sign()? 1 : 0;
		  cur->u_.signal_.sig = find_signal(des_, reg);
		  cur->u_.signal_.lsi = 0;
		  cur->u_.signal_.msi = cur->width_ - 1;

	    } else {
		  cur->type_ = IVL_EX_SELECT;
		  cur->value_ = IVL_VT_VECTOR;
		  cur->width_ = net->expr_width();
		  cur->signed_ = net->has_sign()? 1 : 0;

		    // Create an expression form of the exploded
		    // memory. This is what the select will apply to.
		  ivl_expr_t sig = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
		  sig->type_ = IVL_EX_SIGNAL;
		  sig->value_ = IVL_VT_VECTOR;
		  sig->width_ = reg->pin_count();
		  sig->signed_ = 0;
		  sig->u_.signal_.sig = find_signal(des_, reg);
		  assert(sig->u_.signal_.sig);
		  sig->u_.signal_.lsi = 0;
		  sig->u_.signal_.msi = reg->pin_count()-1;
		  cur->u_.binary_.lef_ = sig;

		    // Create an expression of the address calculation.
		  cur->u_.binary_.rig_ = expr_;

		  if (cur->width_ > 1) {
			ivl_expr_t mul = (ivl_expr_t)calloc(2, sizeof(struct ivl_expr_s));
			ivl_expr_t fac = mul+1;

			fac->type_  = IVL_EX_ULONG;
			fac->value_ = IVL_VT_VECTOR;
			fac->width_ = 8*sizeof(cur->width_);
			fac->signed_= 0;
			fac->u_.ulong_.value = cur->width_;

			mul->type_  = IVL_EX_BINARY;
			mul->value_ = IVL_VT_VECTOR;
			mul->width_ = fac->width_;
			mul->signed_= 0;
			mul->u_.binary_.op_  = '*';
			mul->u_.binary_.lef_ = cur->u_.binary_.rig_;
			mul->u_.binary_.rig_ = fac;
			cur->u_.binary_.rig_ = mul;
		  }
	    }

      } else {
	    cur->type_ = IVL_EX_MEMORY;
	    cur->value_ = IVL_VT_VECTOR;
	    cur->width_= net->expr_width();
	    cur->signed_ = net->has_sign()? 1 : 0;
	    cur->u_.memory_.mem_ = find_memory(des_, net->memory());
	    cur->u_.memory_.idx_ = expr_;
      }

      expr_ = cur;
}

void dll_target::expr_const(const NetEConst*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr_);
      expr_->value_= IVL_VT_VECTOR;

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
      ivl_scope_t scope = find_scope(des_, net->scope());
      ivl_parameter_t par = scope_find_param(scope, net->name());

      if (par == 0) {
	    cerr << net->get_line() << ": internal error: "
		 << "Parameter " << net->name() << " missing from "
		 << ivl_scope_name(scope) << endl;
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
      expr_->type_ = IVL_EX_REALNUM;
      expr_->value_= IVL_VT_REAL;
      expr_->u_.real_.value = net->value().as_double();
}

void dll_target::expr_event(const NetEEvent*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr_);

      expr_->type_ = IVL_EX_EVENT;
      expr_->value_= IVL_VT_VOID;

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
      assert(expr_);

      expr_->type_ = IVL_EX_SCOPE;
      expr_->value_= IVL_VT_VOID;
      expr_->u_.scope_.scope = lookup_scope_(net->scope());
}

void dll_target::expr_select(const NetESelect*net)
{
      assert(expr_ == 0);

      net->sub_expr()->expr_scan(this);
      ivl_expr_t left = expr_;

      expr_ = 0;
      if (net->select())
	    net->select()->expr_scan(this);

      ivl_expr_t rght = expr_;

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr_);

      expr_->type_ = IVL_EX_SELECT;
      expr_->value_= IVL_VT_VECTOR;
      expr_->width_= net->expr_width();
      expr_->signed_ = net->has_sign()? 1 : 0;

      expr_->u_.binary_.lef_ = left;
      expr_->u_.binary_.rig_ = rght;
}

void dll_target::expr_sfunc(const NetESFunc*net)
{
      assert(expr_ == 0);

      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr);

      expr->type_ = IVL_EX_SFUNC;
      switch (net->expr_type()) {
	  case NetExpr::ET_VECTOR:
	    expr->value_= IVL_VT_VECTOR;
	    break;
	  case NetExpr::ET_REAL:
	    expr->value_= IVL_VT_REAL;
	    break;
	  case NetExpr::ET_VOID:
	    assert(0);
	    expr->value_= IVL_VT_VECTOR;
	    break;
      }
      expr->width_= net->expr_width();
      expr->signed_ = net->has_sign()? 1 : 0;
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
      assert(expr);

      expr->type_  = IVL_EX_TERNARY;
      expr->value_= IVL_VT_VECTOR;
      expr->width_ = net->expr_width();
      expr->signed_ = net->has_sign()? 1 : 0;

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
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr_);

      expr_->type_ = IVL_EX_SIGNAL;
      expr_->value_= IVL_VT_VECTOR;
      expr_->width_= net->expr_width();
      expr_->signed_ = net->has_sign()? 1 : 0;
      expr_->u_.signal_.sig = find_signal(des_, net->sig());
      expr_->u_.signal_.lsi = net->lsi();
      expr_->u_.signal_.msi = net->msi();
}

void dll_target::expr_subsignal(const NetEBitSel*net)
{
      assert(expr_ == 0);

      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr);

      expr->type_ = IVL_EX_BITSEL;
      expr->value_= IVL_VT_VECTOR;
      expr->width_= net->expr_width();
      expr->signed_ = net->has_sign()? 1 : 0;
      expr->u_.bitsel_.sig = find_signal(des_, net->sig());

      assert(expr->u_.bitsel_.sig->lsb_index == net->sig()->lsb());

      net->index()->expr_scan(this);
      assert(expr_);
      expr->u_.bitsel_.bit = expr_;

	/* If the lsb of the signal is not 0, then we are about to
	   lose the proper offset to the normalized vector. Modify the
	   expression to subtract the offset:

	      reg [7:4] a;
	      ... = a[x];

	      becomes

	      reg [3:0] a;
	      ... = a[x-4];

	    to reflect the normalizing of vectors that is done by the
	    compiler. */

      if (expr->u_.bitsel_.sig->lsb_index != 0) {

	      /* Create in tmpc the constant offset (4 in the above
		 example) to be subtracted from the index. */
	    char*bits;
	    long lsb = expr->u_.bitsel_.sig->lsb_index;
	    ivl_expr_t tmpc = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
	    tmpc->type_  = IVL_EX_NUMBER;
	    tmpc->width_ = expr->u_.bitsel_.bit->width_;
	    tmpc->signed_ = net->index()->has_sign()? 1 : 0;
	    tmpc->u_.number_.bits_ = bits = (char*)malloc(tmpc->width_);
	    for (unsigned idx = 0 ;  idx < tmpc->width_ ;  idx += 1) {
		  bits[idx] = (lsb & 1)? '1' : '0';
		  lsb >>= 1;
	    }

	      /* Now make the subtractor (x-4 in the above example)
		 that has as input A the index expression and input B
		 the constant to subtract. */
	    ivl_expr_t tmps = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
	    tmps->type_  = IVL_EX_BINARY;
	    tmps->width_ = tmpc->width_;
	    tmps->signed_ = net->index()->has_sign()? 1 : 0;
	    tmps->u_.binary_.op_  = '-';
	    tmps->u_.binary_.lef_ = expr->u_.bitsel_.bit;
	    tmps->u_.binary_.rig_ = tmpc;

	      /* Replace (x) with (x-4) */
	    expr->u_.bitsel_.bit = tmps;

	      /* If the index item distance (the distance to the next
		 most significant bit) is not 1, then multiply the
		 previous result to convert the index. */
	    if (expr->u_.bitsel_.sig->lsb_dist != 1) {
		  long dist = expr->u_.bitsel_.sig->lsb_dist;

		  tmpc = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
		  tmpc->type_  = IVL_EX_NUMBER;
		  tmpc->width_ = expr->u_.bitsel_.bit->width_;
		  tmpc->signed_ = 1;
		  tmpc->u_.number_.bits_ = bits = (char*)malloc(tmpc->width_);
		  for (unsigned idx = 0 ;  idx < tmpc->width_ ;  idx += 1) {
			bits[idx] = (dist & 1)? '1' : '0';
			dist >>= 1;
		  }

		  tmps = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
		  tmps->type_  = IVL_EX_BINARY;
		  tmps->width_ = tmpc->width_;
		  tmps->signed_ = 1;
		  tmps->u_.binary_.op_  = '*';
		  tmps->u_.binary_.lef_ = expr->u_.bitsel_.bit;
		  tmps->u_.binary_.rig_ = tmpc;

		  expr->u_.bitsel_.bit = tmps;
	    }

      }

      expr_ = expr;
}

void dll_target::expr_ufunc(const NetEUFunc*net)
{
      assert(expr_ == 0);

      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr);

      expr->type_ = IVL_EX_UFUNC;
      expr->value_= IVL_VT_VECTOR;
      expr->width_= net->expr_width();
      expr->signed_ = net->has_sign()? 1 : 0;

      expr->u_.ufunc_.def = lookup_scope_(net->func());
      assert(expr->u_.ufunc_.def->type_ == IVL_SCT_FUNCTION);

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
      expr_->value_= IVL_VT_VECTOR;
      expr_->width_ = net->expr_width();
      expr_->signed_ = net->has_sign()? 1 : 0;
      expr_->u_.unary_.op_ = net->op();
      expr_->u_.unary_.sub_ = sub;
}

void dll_target::expr_variable(const NetEVariable*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      expr_->type_  = IVL_EX_VARIABLE;
      expr_->value_ = IVL_VT_REAL;
      expr_->width_ = 0;
      expr_->signed_= net->has_sign()? 1 : 0;
      expr_->u_.variable_.var = find_variable(des_, net->variable());
}

/*
 * $Log: t-dll-expr.cc,v $
 * Revision 1.39.2.2  2006/09/15 23:56:05  steve
 *  Special handling of exploded memory arguments.
 *
 * Revision 1.39.2.1  2006/03/12 07:34:19  steve
 *  Fix the memsynth1 case.
 *
 * Revision 1.39  2004/06/17 16:06:19  steve
 *  Help system function signedness survive elaboration.
 *
 * Revision 1.38  2003/07/26 03:34:43  steve
 *  Start handling pad of expressions in code generators.
 *
 * Revision 1.37  2003/06/24 01:38:03  steve
 *  Various warnings fixed.
 *
 * Revision 1.36  2003/04/22 04:48:30  steve
 *  Support event names as expressions elements.
 *
 * Revision 1.35  2003/03/10 23:40:53  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.34  2003/03/01 06:25:30  steve
 *  Add the lex_strings string handler, and put
 *  scope names and system task/function names
 *  into this table. Also, permallocate event
 *  names from the beginning.
 *
 * Revision 1.33  2003/02/02 00:19:27  steve
 *  Terminate bits string from ivl_expr_bits.
 *
 * Revision 1.32  2003/01/30 16:23:08  steve
 *  Spelling fixes.
 *
 * Revision 1.31  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.30  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.29  2002/10/23 01:47:17  steve
 *  Fix synth2 handling of aset/aclr signals where
 *  flip-flops are split by begin-end blocks.
 *
 * Revision 1.28  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.27  2002/08/04 18:28:15  steve
 *  Do not use hierarchical names of memories to
 *  generate vvp labels. -tdll target does not
 *  used hierarchical name string to look up the
 *  memory objects in the design.
 *
 * Revision 1.26  2002/06/16 20:39:12  steve
 *  Normalize run-time index expressions for bit selects
 *
 * Revision 1.25  2002/06/16 19:19:16  steve
 *  Generate runtime code to normalize indices.
 *
 * Revision 1.24  2002/05/29 22:05:54  steve
 *  Offset lvalue index expressions.
 *
 * Revision 1.23  2002/04/14 02:56:19  steve
 *  Support signed expressions through to VPI.
 *
 * Revision 1.22  2002/01/28 00:52:41  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.21  2001/12/31 00:08:14  steve
 *  Support $signed cast of expressions.
 *
 * Revision 1.20  2001/10/23 04:22:41  steve
 *  Support bit selects of non-0 lsb for vectors.
 *
 * Revision 1.19  2001/10/19 21:53:24  steve
 *  Support multiple root modules (Philip Blundell)
 *
 * Revision 1.18  2001/09/15 18:27:04  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.17  2001/07/27 04:51:44  steve
 *  Handle part select expressions as variants of
 *  NetESignal/IVL_EX_SIGNAL objects, instead of
 *  creating new and useless temporary signals.
 *
 * Revision 1.16  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.15  2001/07/22 00:17:49  steve
 *  Support the NetESubSignal expressions in vvp.tgt.
 *
 * Revision 1.14  2001/07/07 20:20:10  steve
 *  Pass parameters to system functions.
 *
 * Revision 1.13  2001/05/17 04:37:02  steve
 *  Behavioral ternary operators for vvp.
 *
 * Revision 1.12  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 */

