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
#ident "$Id: t-dll-expr.cc,v 1.6 2000/10/28 17:55:03 steve Exp $"
#endif

# include  "t-dll.h"
# include  "netlist.h"
# include  <assert.h>
# include  <malloc.h>

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

      expr_->u_.binary_.op_ = net->op();
      expr_->u_.binary_.lef_ = left;
      expr_->u_.binary_.rig_ = rght;
}

void dll_target::expr_concat(const NetEConcat*net)
{
      assert(expr_ == 0);

      expr_ = new struct ivl_expr_s;
      assert(expr_);

      expr_->type_ = IVL_EX_CONCAT;
      expr_->width_= net->expr_width();
}

void dll_target::expr_const(const NetEConst*net)
{
      assert(expr_ == 0);

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

void dll_target::expr_sfunc(const NetESFunc*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr_);

      expr_->type_ = IVL_EX_SFUNC;
      expr_->width_= net->expr_width();
      expr_->u_.sfunc_.name_ = strdup(net->name());
}

void dll_target::expr_signal(const NetESignal*net)
{
      assert(expr_ == 0);

      expr_ = (ivl_expr_t)calloc(1, sizeof(struct ivl_expr_s));
      assert(expr_);

      expr_->type_ = IVL_EX_SIGNAL;
      expr_->width_= net->expr_width();
      expr_->u_.subsig_.name_ = strdup(net->name().c_str());
}

/*
 * $Log: t-dll-expr.cc,v $
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

