/*
 *  VHDL code generation for expressions.
 *
 *  Copyright (C) 2008  Nick Gasson (nick@nickg.me.uk)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "vhdl_target.h"

#include <iostream>
#include <cassert>

/*
 * Convert a constant Verilog string to a constant VHDL string.
 */
static vhdl_expr *translate_string(ivl_expr_t e)
{   
   // TODO: May need to inspect or escape parts of this
   const char *str = ivl_expr_string(e);
   return new vhdl_const_string(str);
}

/*
 * A reference to a signal in an expression. It's assumed that the
 * signal has already been defined elsewhere.
 */
static vhdl_expr *translate_signal(ivl_expr_t e)
{
   ivl_signal_t sig = ivl_expr_signal(e);

   // Assume all signals are single bits at the moment
   vhdl_type *type = vhdl_type::std_logic();
   
   return new vhdl_var_ref(ivl_signal_basename(sig), type);
}

/*
 * A numeric literal ends up as std_logic bit string.
 */
static vhdl_expr *translate_number(ivl_expr_t e)
{
   return new vhdl_const_bits(ivl_expr_bits(e));
}

static vhdl_expr *translate_unary(ivl_expr_t e)
{
   std::cout << "Unary opcode " << ivl_expr_opcode(e) << std::endl;
   return NULL;
}

/*
 * Generate a VHDL expression from a Verilog expression.
 */
vhdl_expr *translate_expr(ivl_expr_t e)
{
   ivl_expr_type_t type = ivl_expr_type(e);

   switch (type) {
   case IVL_EX_STRING:
      return translate_string(e);
   case IVL_EX_SIGNAL:
      return translate_signal(e);
   case IVL_EX_NUMBER:
      return translate_number(e);
   case IVL_EX_UNARY:
      return translate_unary(e);
   default:
      error("No VHDL translation for expression at %s:%d (type = %d)",
            ivl_expr_file(e), ivl_expr_lineno(e), type);
      return NULL;
   }
}
