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
static vhdl_var_ref *translate_signal(ivl_expr_t e)
{
   ivl_signal_t sig = ivl_expr_signal(e);

   const vhdl_scope *scope = find_scope_for_signal(sig);
   assert(scope);

   const char *renamed = get_renamed_signal(sig).c_str();
   
   const vhdl_decl *decl = scope->get_decl(strip_var(renamed));
   assert(decl);

   vhdl_type *type = new vhdl_type(*decl->get_type());
   
   return new vhdl_var_ref(renamed, type);
}

/*
 * A numeric literal ends up as std_logic bit string.
 */
static vhdl_expr *translate_number(ivl_expr_t e)
{
   return new vhdl_const_bits(ivl_expr_bits(e), ivl_expr_width(e),
                              ivl_expr_signed(e) != 0);
}

static vhdl_expr *translate_unary(ivl_expr_t e)
{
   vhdl_expr *operand = translate_expr(ivl_expr_oper1(e));
   if (NULL == operand)
      return NULL;
      
   switch (ivl_expr_opcode(e)) {
   case '!':
   case '~':
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
   vhdl_type *rtype = new vhdl_type(*lhs->get_type());
   return new vhdl_binop_expr(lhs, op, rhs, rtype);
}

static vhdl_expr *translate_relation(vhdl_expr *lhs, vhdl_expr *rhs,
                                     vhdl_binop_t op)
{
   // Generate any necessary casts
   // Arbitrarily, the RHS is casted to the type of the LHS
   vhdl_expr *r_cast = rhs->cast(lhs->get_type());
   
   return new vhdl_binop_expr(lhs, op, r_cast, vhdl_type::boolean());
}

static vhdl_expr *translate_shift(vhdl_expr *lhs, vhdl_expr *rhs,
                                  vhdl_binop_t op)
{
   // The RHS must be an integer
   vhdl_type integer(VHDL_TYPE_INTEGER);
   vhdl_expr *r_cast = rhs->cast(&integer);

   vhdl_type *rtype = new vhdl_type(*lhs->get_type());
   return new vhdl_binop_expr(lhs, op, r_cast, rtype);
}

static vhdl_expr *translate_binary(ivl_expr_t e)
{
   vhdl_expr *lhs = translate_expr(ivl_expr_oper1(e));
   if (NULL == lhs)
      return NULL;
   
   vhdl_expr *rhs = translate_expr(ivl_expr_oper2(e));
   if (NULL == rhs)
      return NULL;
   
   int lwidth = lhs->get_type()->get_width();
   int rwidth = rhs->get_type()->get_width();

   // May need to resize the left or right hand side
   int opwidth;
   if (lwidth < rwidth) {
      rhs = rhs->cast(lhs->get_type());
      opwidth = lwidth;
   }
   else if (rwidth < lwidth) {
      lhs = lhs->cast(rhs->get_type());
      opwidth = rwidth;
   }
   else
      opwidth = lwidth;

   // For === and !== we need to compare std_logic_vectors
   // rather than signeds
   vhdl_type std_logic_vector(VHDL_TYPE_STD_LOGIC_VECTOR, opwidth-1, 0);
   bool vectorop =
      (lhs->get_type()->get_name() == VHDL_TYPE_SIGNED
       || lhs->get_type()->get_name() == VHDL_TYPE_UNSIGNED) &&
      (rhs->get_type()->get_name() == VHDL_TYPE_SIGNED
       || rhs->get_type()->get_name() == VHDL_TYPE_UNSIGNED);
      
   switch (ivl_expr_opcode(e)) {
   case '+':
      return translate_numeric(lhs, rhs, VHDL_BINOP_ADD);
   case '-':
      return translate_numeric(lhs, rhs, VHDL_BINOP_SUB);
   case 'e':
      return translate_relation(lhs, rhs, VHDL_BINOP_EQ);
   case 'E':
      if (vectorop)
         return translate_relation(lhs->cast(&std_logic_vector),
                                   rhs->cast(&std_logic_vector), VHDL_BINOP_EQ);
      else
         return translate_relation(lhs, rhs, VHDL_BINOP_EQ);
   case 'n':
      return translate_relation(lhs, rhs, VHDL_BINOP_NEQ);
   case 'N':
      if (vectorop)
         return translate_relation(lhs->cast(&std_logic_vector),
                                   rhs->cast(&std_logic_vector), VHDL_BINOP_NEQ);
      else
         return translate_relation(lhs, rhs, VHDL_BINOP_NEQ);
   case '&':    // Bitwise AND
      return translate_numeric(lhs, rhs, VHDL_BINOP_AND);
   case 'o':
      return translate_relation(lhs, rhs, VHDL_BINOP_OR);
   case '<':
      return translate_relation(lhs, rhs, VHDL_BINOP_LT);
   case '>':
      return translate_relation(lhs, rhs, VHDL_BINOP_GT);
   case 'l':
      return translate_shift(lhs, rhs, VHDL_BINOP_SL);
   case 'r':
      return translate_shift(lhs, rhs, VHDL_BINOP_SR);
   case '^':
      return translate_numeric(lhs, rhs, VHDL_BINOP_XOR);
   default:
      error("No translation for binary opcode '%c'\n",
            ivl_expr_opcode(e));
      delete lhs;
      delete rhs;
      return NULL;
   }
}

vhdl_expr *translate_select(ivl_expr_t e)
{
   vhdl_expr *from = translate_expr(ivl_expr_oper1(e));
   if (NULL == from)
      return NULL;   
   
   // Hack: resize it to the correct size
   return from->resize(ivl_expr_width(e));
}

vhdl_expr *translate_ufunc(ivl_expr_t e)
{
   ivl_scope_t defscope = ivl_expr_def(e);
   ivl_scope_t parentscope = ivl_scope_parent(defscope);
   assert(ivl_scope_type(parentscope) == IVL_SCT_MODULE);

   // A function is always declared in a module, which should have
   // a corresponding entity by this point: so we can get type
   // information, etc. from the declaration
   vhdl_entity *parent_ent = find_entity(ivl_scope_tname(parentscope));
   assert(parent_ent);

   const char *funcname = ivl_scope_tname(defscope);
   
   vhdl_decl *fdecl =
      parent_ent->get_arch()->get_scope()->get_decl(funcname);
   assert(fdecl);
   
   vhdl_type *rettype = new vhdl_type(*fdecl->get_type());
   vhdl_fcall *fcall = new vhdl_fcall(funcname, rettype);

   int nparams = ivl_expr_parms(e);
   for (int i = 0; i < nparams; i++) {
      vhdl_expr *param = translate_expr(ivl_expr_parm(e, i));
      if (NULL == param)
         return NULL;

      fcall->add_expr(param);
   }

   return fcall;
}

/*
 * Generate a VHDL expression from a Verilog expression.
 */
vhdl_expr *translate_expr(ivl_expr_t e)
{
   assert(e);
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
   case IVL_EX_SELECT:
      return translate_select(e);
   case IVL_EX_UFUNC:
      return translate_ufunc(e);
   default:
      error("No VHDL translation for expression at %s:%d (type = %d)",
            ivl_expr_file(e), ivl_expr_lineno(e), type);
      return NULL;
   }
}
