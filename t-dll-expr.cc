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
#if !defined(WINNT) & !defined(macintosh)
#ident "$Id: t-dll-expr.cc,v 1.18 2001/09/15 18:27:04 steve Exp $"
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
 * These methods implement the expression scan that generates the
 * ivl_expr_t representing the expression. Each method leaves the
 * expr_ member filled with the ivl_expr_t that represents it. Each
 * method expects that the expr_ member empty (0) when it starts.
 */


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

      cur->type_ = IVL_EX_CONCAT;
      cur->width_= net->expr_width();

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

      cur->type_ = IVL_EX_MEMORY;
      cur->width_= net->expr_width();
      cur->signed_ = net->has_sign()? 1 : 0;
      cur->u_.memory_.mem_ = lookup_memory_(net->memory());
      cur->u_.memory_.idx_ = expr_;

      expr_ = cur;
}

void dll_target::expr_const(const NetEConst*net)
{
      assert(expr_ == 0);

      ivl_expr_t idx = 0;

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr_);

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
	    expr_->signed_ = val.has_sign()? 1 : 0;
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

void dll_target::expr_scope(const NetEScope*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr_);

      expr_->type_ = IVL_EX_SCOPE;
      expr_->u_.scope_.scope = lookup_scope_(net->scope());
}

void dll_target::expr_sfunc(const NetESFunc*net)
{
      assert(expr_ == 0);

      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr);

      expr->type_ = IVL_EX_SFUNC;
      expr->width_= net->expr_width();
      expr->u_.sfunc_.name_ = strdup(net->name());

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
      expr_->width_= net->expr_width();
      expr_->signed_ = net->has_sign()? 1 : 0;
      expr_->u_.signal_.sig = find_signal(des_.root_, net->sig());
      expr_->u_.signal_.lsi = net->lsi();
      expr_->u_.signal_.msi = net->msi();
}

void dll_target::expr_subsignal(const NetEBitSel*net)
{
      assert(expr_ == 0);

      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr);

      if (net->sig()->lsb() != 0) {
	    cerr << net->get_line() << ": sorry: LSB for signal "
		 << "is not zero." << endl;
      }

      expr->type_ = IVL_EX_BITSEL;
      expr->width_= net->expr_width();
      expr->signed_ = net->has_sign()? 1 : 0;
      expr->u_.bitsel_.sig = find_signal(des_.root_, net->sig());

      net->index()->expr_scan(this);
      assert(expr_);
      expr->u_.bitsel_.bit = expr_;

      expr_ = expr;
}

void dll_target::expr_ufunc(const NetEUFunc*net)
{
      assert(expr_ == 0);

      ivl_expr_t expr = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr);

      expr->type_ = IVL_EX_UFUNC;
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
      expr_->width_= net->expr_width();
      expr_->u_.unary_.op_ = net->op();
      expr_->u_.unary_.sub_ = sub;
}

/*
 * $Log: t-dll-expr.cc,v $
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
 *
 * Revision 1.11  2001/04/06 02:28:02  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.10  2001/04/05 01:12:28  steve
 *  Get signed compares working correctly in vvp.
 *
 * Revision 1.9  2001/04/02 00:28:35  steve
 *  Support the scope expression node.
 *
 * Revision 1.8  2001/03/29 02:52:39  steve
 *  Add unary ~ operator to tgt-vvp.
 *
 * Revision 1.7  2000/10/28 22:32:34  steve
 *  API for concatenation expressions.
 *
 * Revision 1.6  2000/10/28 17:55:03  steve
 *  stub for the concat operator.
 *
 * Revision 1.5  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 *
 * Revision 1.4  2000/09/30 02:18:15  steve
 *  ivl_expr_t support for binary operators,
 *  Create a proper ivl_scope_t object.
 *
 * Revision 1.3  2000/09/26 00:30:07  steve
 *  Add EX_NUMBER and ST_TRIGGER to dll-api.
 *
 * Revision 1.2  2000/09/24 02:21:53  steve
 *  Add support for signal expressions.
 *
 * Revision 1.1  2000/09/23 05:15:07  steve
 *  Add enough tgt-verilog code to support hello world.
 *
 */

