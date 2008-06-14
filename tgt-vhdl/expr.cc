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

   const vhdl_entity *ent = find_entity_for_signal(sig);
   assert(ent);

   const char *renamed = get_renamed_signal(sig).c_str();
   
   const vhdl_decl *decl = ent->get_arch()->get_decl(renamed);
   assert(decl);

   vhdl_type *type = new vhdl_type(*decl->get_type());
   
   return new vhdl_var_ref(renamed, type);
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
   vhdl_expr *operand = translate_expr(ivl_expr_oper1(e));
   if (NULL == operand)
      return NULL;
      
   switch (ivl_expr_opcode(e)) {
   case '!':
      return new vhdl_unaryop_expr
         (VHDL_UNARYOP_NOT, operand, new vhdl_type(*operand->get_type()));
   default:
      error("No translation for unary opcode '%c'\n",
            ivl_expr_opcode(e));
      delete operand;
      return NULL;
   }
}

/*
 * Translate a numeric binary operator (+, -, etc.) to
 * a VHDL equivalent using the numeric_std package.
 */
static vhdl_expr *translate_numeric(vhdl_expr *lhs, vhdl_expr *rhs,
                                    vhdl_binop_t op)
{ 
   int lwidth = lhs->get_type()->get_width();
   int rwidth = rhs->get_type()->get_width();

   vhdl_type ltype(VHDL_TYPE_UNSIGNED, lhs->get_type()->get_msb(),
                   lhs->get_type()->get_lsb());
   vhdl_type rtype(VHDL_TYPE_UNSIGNED, rhs->get_type()->get_msb(),
                   rhs->get_type()->get_lsb());
   
   // May need to resize the left or right hand side
   if (lwidth < rwidth) {
      vhdl_fcall *resize =
         new vhdl_fcall("resize", vhdl_type::nsigned(rwidth));
      resize->add_expr(lhs);
      resize->add_expr(new vhdl_const_int(rwidth));
      lhs = resize;
      lwidth = rwidth;
   }
   else if (rwidth < lwidth) {
      vhdl_fcall *resize =
         new vhdl_fcall("resize", vhdl_type::nsigned(lwidth));
      resize->add_expr(rhs);
      resize->add_expr(new vhdl_const_int(lwidth));
      rhs = resize;
      rwidth = lwidth;
   }
   
   return new vhdl_binop_expr(lhs, op, rhs, vhdl_type::nsigned(lwidth));
}

static vhdl_expr *translate_binary(ivl_expr_t e)
{
   vhdl_expr *lhs = translate_expr(ivl_expr_oper1(e));
   if (NULL == lhs)
      return NULL;
   
   vhdl_expr *rhs = translate_expr(ivl_expr_oper2(e));
   if (NULL == rhs)
      return NULL;

   switch (ivl_expr_opcode(e)) {
   case '+':
      return translate_numeric(lhs, rhs, VHDL_BINOP_ADD);
   default:
      error("No translation for binary opcode '%c'\n",
            ivl_expr_opcode(e));
      delete lhs;
      delete rhs;
      return NULL;
   }
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
   case IVL_EX_BINARY:
      return translate_binary(e);
   default:
      error("No VHDL translation for expression at %s:%d (type = %d)",
            ivl_expr_file(e), ivl_expr_lineno(e), type);
      return NULL;
   }
}
