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
#ident "$Id: t-dll-expr.cc,v 1.2 2000/09/24 02:21:53 steve Exp $"
#endif

# include  "t-dll.h"
# include  "netlist.h"
# include  <assert.h>


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
	    expr_->type_ = IVL_EX_NUMBER;
	    expr_->width_= net->expr_width();

      }
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
 * Revision 1.2  2000/09/24 02:21:53  steve
 *  Add support for signal expressions.
 *
 * Revision 1.1  2000/09/23 05:15:07  steve
 *  Add enough tgt-verilog code to support hello world.
 *
 */

