/*
 *  VHDL code generation for LPM devices.
 *
 *  Copyright (C) 2008-2009  Nick Gasson (nick@nickg.me.uk)
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
#include "state.hh"

#include <iostream>
#include <cassert>

/*
 * Return the base of a part select.
 */
static vhdl_expr *part_select_base(vhdl_scope *scope, ivl_lpm_t lpm)
{
   vhdl_expr *off;
   ivl_nexus_t base = ivl_lpm_data(lpm, 1);
   if (base != NULL)
      off = readable_ref(scope, base);
   else
      off = new vhdl_const_int(ivl_lpm_base(lpm));

   // Array indexes must be integers
   vhdl_type integer(VHDL_TYPE_INTEGER);
   return off->cast(&integer);
}

static vhdl_expr *binop_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm, vhdl_binop_t op)
{
   unsigned out_width = ivl_lpm_width(lpm);
   vhdl_type *result_type =
      vhdl_type::type_for(out_width, ivl_lpm_signed(lpm) != 0);
   vhdl_binop_expr *expr = new vhdl_binop_expr(op, result_type);

   for (unsigned i = 0; i < ivl_lpm_size(lpm); i++) {
      vhdl_expr *e = readable_ref(scope, ivl_lpm_data(lpm, i));
      if (NULL == e)
         return NULL;

      // It's possible that the inputs are a mixture of signed and unsigned
      // in which case we must cast them to the output type
      e = e->cast(vhdl_type::type_for(e->get_type()->get_width(),
                                      ivl_lpm_signed(lpm) != 0));

      // Bit of a hack: the LPM inputs are in the wrong order for concatenation
      if (op == VHDL_BINOP_CONCAT)
         expr->add_expr_front(e);
      else
         expr->add_expr(e);
   }

   if (op == VHDL_BINOP_MULT) {
      // Need to resize the output to the desired size,
      // as this does not happen automatically in VHDL

      vhdl_fcall *resize =
         new vhdl_fcall("Resize", vhdl_type::nsigned(out_width));
      resize->add_expr(expr);
      resize->add_expr(new vhdl_const_int(out_width));

      return resize;
   }
   else
      return expr;
}

static vhdl_expr *rel_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm, vhdl_binop_t op)
{
   vhdl_binop_expr *expr = new vhdl_binop_expr(op, vhdl_type::boolean());

   vhdl_expr *lhs = readable_ref(scope, ivl_lpm_data(lpm, 0));
   if (NULL == lhs)
      return NULL;

   vhdl_expr *rhs = readable_ref(scope, ivl_lpm_data(lpm, 1));
   if (NULL == rhs) {
      delete lhs;
      return NULL;
   }

   // Ensure LHS and RHS are the same type
   if (lhs->get_type() != rhs->get_type())
      rhs = rhs->cast(lhs->get_type());

   expr->add_expr(lhs);
   expr->add_expr(rhs);

   return expr;
}

static vhdl_expr *part_select_vp_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm)
{
   vhdl_var_ref *selfrom = readable_ref(scope, ivl_lpm_data(lpm, 0));
   if (NULL == selfrom)
      return NULL;

   vhdl_expr *off = part_select_base(scope, lpm);;
   if (NULL == off)
      return NULL;

   if (selfrom->get_type()->get_width() > 1)
      selfrom->set_slice(off, ivl_lpm_width(lpm) - 1);

   return selfrom;
}


static vhdl_expr *part_select_pv_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm)
{
   return readable_ref(scope, ivl_lpm_data(lpm, 0));
}

static vhdl_expr *ufunc_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm)
{
   ivl_scope_t f_scope = ivl_lpm_define(lpm);
   vhdl_fcall *fcall = new vhdl_fcall(ivl_scope_basename(f_scope), NULL);

   for (unsigned i = 0; i < ivl_lpm_size(lpm); i++) {
      vhdl_var_ref *ref = readable_ref(scope, ivl_lpm_data(lpm, i));
      if (NULL == ref) {
         delete fcall;
         return NULL;
      }

      fcall->add_expr(ref);
   }

   return fcall;
}

static vhdl_expr *reduction_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm,
                                        support_function_t f, bool invert)
{
   vhdl_var_ref *ref = readable_ref(scope, ivl_lpm_data(lpm, 0));
   if (NULL == ref)
      return NULL;

   vhdl_expr *result;
   if (ref->get_type()->get_name() == VHDL_TYPE_STD_LOGIC)
      result = ref;
   else {
      require_support_function(f);
      vhdl_fcall *fcall = new vhdl_fcall(support_function::function_name(f),
                                         vhdl_type::std_logic());

      vhdl_type std_logic_vector(VHDL_TYPE_STD_LOGIC_VECTOR);
      fcall->add_expr(ref->cast(&std_logic_vector));

      result = fcall;
   }

   if (invert)
      return new vhdl_unaryop_expr
         (VHDL_UNARYOP_NOT, result, vhdl_type::std_logic());
   else
      return result;
}

static vhdl_expr *sign_extend_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm)
{
   vhdl_expr *ref = readable_ref(scope, ivl_lpm_data(lpm, 0));
   if (ref)
      return ref->resize(ivl_lpm_width(lpm));
   else
      return NULL;
}

static vhdl_expr *array_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm)
{
   ivl_signal_t array = ivl_lpm_array(lpm);
   if (!seen_signal_before(array))
      return NULL;

   const char *renamed = get_renamed_signal(array).c_str();

   vhdl_decl *adecl = scope->get_decl(renamed);
   assert(adecl);

   vhdl_type *atype = new vhdl_type(*adecl->get_type());

   vhdl_expr *select = readable_ref(scope, ivl_lpm_select(lpm));
   if (NULL == select) {
      delete atype;
      return NULL;
   }

   vhdl_var_ref *ref = new vhdl_var_ref(renamed, atype);
   vhdl_type integer(VHDL_TYPE_INTEGER);
   ref->set_slice(select->cast(&integer));

   return ref;
}

static vhdl_expr *shift_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm,
                                    vhdl_binop_t shift_op)
{
   vhdl_expr *lhs = readable_ref(scope, ivl_lpm_data(lpm, 0));
   vhdl_expr *rhs = readable_ref(scope, ivl_lpm_data(lpm, 1));
   if (!lhs || !rhs)
      return NULL;

   // The RHS must be an integer
   vhdl_type integer(VHDL_TYPE_INTEGER);
   vhdl_expr *r_cast = rhs->cast(&integer);

   vhdl_type *rtype = new vhdl_type(*lhs->get_type());
   return new vhdl_binop_expr(lhs, shift_op, r_cast, rtype);
}

static vhdl_expr *repeat_lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm)
{
   vhdl_expr *in = readable_ref(scope, ivl_lpm_data(lpm, 0));
   return new vhdl_bit_spec_expr(NULL, in);
}

static vhdl_expr *lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm)
{
   switch (ivl_lpm_type(lpm)) {
   case IVL_LPM_ADD:
      return binop_lpm_to_expr(scope, lpm, VHDL_BINOP_ADD);
   case IVL_LPM_SUB:
      return binop_lpm_to_expr(scope, lpm, VHDL_BINOP_SUB);
   case IVL_LPM_MULT:
      return binop_lpm_to_expr(scope, lpm, VHDL_BINOP_MULT);
   case IVL_LPM_DIVIDE:
      return binop_lpm_to_expr(scope, lpm, VHDL_BINOP_DIV);
   case IVL_LPM_MOD:
      return binop_lpm_to_expr(scope, lpm, VHDL_BINOP_MOD);
   case IVL_LPM_CONCAT:
      return binop_lpm_to_expr(scope, lpm, VHDL_BINOP_CONCAT);
   case IVL_LPM_CMP_GE:
      return rel_lpm_to_expr(scope, lpm, VHDL_BINOP_GEQ);
   case IVL_LPM_CMP_GT:
      return rel_lpm_to_expr(scope, lpm, VHDL_BINOP_GT);
   case IVL_LPM_CMP_NE:
   case IVL_LPM_CMP_NEE:
      return rel_lpm_to_expr(scope, lpm, VHDL_BINOP_NEQ);
   case IVL_LPM_CMP_EQ:
   case IVL_LPM_CMP_EEQ:
      return rel_lpm_to_expr(scope, lpm, VHDL_BINOP_EQ);
   case IVL_LPM_PART_VP:
      return part_select_vp_lpm_to_expr(scope, lpm);
   case IVL_LPM_PART_PV:
      return part_select_pv_lpm_to_expr(scope, lpm);
   case IVL_LPM_UFUNC:
      return ufunc_lpm_to_expr(scope, lpm);
   case IVL_LPM_RE_AND:
      return reduction_lpm_to_expr(scope, lpm, SF_REDUCE_AND, false);
   case IVL_LPM_RE_NAND:
      return reduction_lpm_to_expr(scope, lpm, SF_REDUCE_AND, true);
   case IVL_LPM_RE_NOR:
      return reduction_lpm_to_expr(scope, lpm, SF_REDUCE_OR, true);
   case IVL_LPM_RE_OR:
      return reduction_lpm_to_expr(scope, lpm, SF_REDUCE_OR, false);
   case IVL_LPM_RE_XOR:
      return reduction_lpm_to_expr(scope, lpm, SF_REDUCE_XOR, false);
   case IVL_LPM_RE_XNOR:
      return reduction_lpm_to_expr(scope, lpm, SF_REDUCE_XNOR, true);
   case IVL_LPM_SIGN_EXT:
      return sign_extend_lpm_to_expr(scope, lpm);
   case IVL_LPM_ARRAY:
      return array_lpm_to_expr(scope, lpm);
   case IVL_LPM_SHIFTL:
      return shift_lpm_to_expr(scope, lpm, VHDL_BINOP_SL);
   case IVL_LPM_SHIFTR:
      return shift_lpm_to_expr(scope, lpm, VHDL_BINOP_SR);
   case IVL_LPM_REPEAT:
      return repeat_lpm_to_expr(scope, lpm);
   default:
      error("Unsupported LPM type: %d", ivl_lpm_type(lpm));
      return NULL;
   }
}

static int draw_mux_lpm(vhdl_arch *arch, ivl_lpm_t lpm)
{
   int nselects = ivl_lpm_selects(lpm);

   if (nselects > 1) {
      error("Only 1 LPM select bit supported at the moment");
      return 1;
   }

   vhdl_scope *scope = arch->get_scope();

   vhdl_expr *s0 = readable_ref(scope, ivl_lpm_data(lpm, 0));
   vhdl_expr *s1 = readable_ref(scope, ivl_lpm_data(lpm, 1));

   vhdl_expr *sel = readable_ref(scope, ivl_lpm_select(lpm));
   vhdl_expr *b1 = new vhdl_const_bit('1');
   vhdl_expr *t1 =
      new vhdl_binop_expr(sel, VHDL_BINOP_EQ, b1, vhdl_type::boolean());

   vhdl_var_ref *out = nexus_to_var_ref(scope, ivl_lpm_q(lpm));

   // Make sure s0 and s1 have the same type as the output
   s0 = s0->cast(out->get_type());
   s1 = s1->cast(out->get_type());

   vhdl_cassign_stmt *s = new vhdl_cassign_stmt(out, s0);
   s->add_condition(s1, t1);

   arch->add_stmt(s);
   return 0;
}

int draw_lpm(vhdl_arch *arch, ivl_lpm_t lpm)
{
   if (ivl_lpm_type(lpm) == IVL_LPM_MUX)
      return draw_mux_lpm(arch, lpm);

   vhdl_expr *f = lpm_to_expr(arch->get_scope(), lpm);
   if (NULL == f)
      return 1;

   vhdl_var_ref *out = nexus_to_var_ref(arch->get_scope(), ivl_lpm_q(lpm));
   if (ivl_lpm_type(lpm) == IVL_LPM_PART_PV) {
      vhdl_expr *off = part_select_base(arch->get_scope(), lpm);
      assert(off);

      out->set_slice(off, ivl_lpm_width(lpm) - 1);
   }

   // Converting from Boolean to std_logic is a common case so should be
   // replaced by an idiomatic VHDL construct rather than a call to a
   // conversion function

   bool bool_to_logic =
      out->get_type()->get_name() == VHDL_TYPE_STD_LOGIC
      && f->get_type()->get_name() == VHDL_TYPE_BOOLEAN;

   if (bool_to_logic) {
      vhdl_cassign_stmt* s =
         new vhdl_cassign_stmt(out, new vhdl_const_bit('0'));
      s->add_condition(new vhdl_const_bit('1'), f);
      arch->add_stmt(s);
   }
   else
      arch->add_stmt(new vhdl_cassign_stmt(out, f->cast(out->get_type())));

   return 0;
}

