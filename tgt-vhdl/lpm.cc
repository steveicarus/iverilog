/*
 *  VHDL code generation for LPM devices.
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

static vhdl_expr *draw_binop_lpm(vhdl_scope *scope, ivl_lpm_t lpm, vhdl_binop_t op)
{
   vhdl_expr *lhs = nexus_to_var_ref(scope, ivl_lpm_data(lpm, 0));
   if (NULL == lhs)
      return NULL;

   vhdl_expr *rhs = nexus_to_var_ref(scope, ivl_lpm_data(lpm, 1));
   if (NULL == rhs)
      return NULL;

   vhdl_type *result_type = new vhdl_type(*lhs->get_type());
   vhdl_expr *expr = new vhdl_binop_expr(lhs, op, rhs, result_type);

   if (op == VHDL_BINOP_MULT) {
      // Need to resize the output to the desired size,
      // as this does not happen automatically in VHDL
      
      unsigned out_width = ivl_lpm_width(lpm);
      vhdl_fcall *resize =
         new vhdl_fcall("Resize", vhdl_type::nsigned(out_width));
      resize->add_expr(expr);
      resize->add_expr(new vhdl_const_int(out_width));
      
      expr = resize;
   }

   return expr;
}

/*
 * Return the base of a part select.
 */
static vhdl_expr *part_select_base(vhdl_scope *scope, ivl_lpm_t lpm)
{
   vhdl_expr *off;
   ivl_nexus_t base = ivl_lpm_data(lpm, 1);
   if (base != NULL)
      off = nexus_to_var_ref(scope, base);
   else
      off = new vhdl_const_int(ivl_lpm_base(lpm));

   // Array indexes must be integers
   vhdl_type integer(VHDL_TYPE_INTEGER);
   return off->cast(&integer);
}

static vhdl_expr *draw_part_select_vp_lpm(vhdl_scope *scope, ivl_lpm_t lpm)
{
   std::cout << "Part select vp" << std::endl;
   
   vhdl_var_ref *selfrom = nexus_to_var_ref(scope, ivl_lpm_data(lpm, 0));
   if (NULL == selfrom)
      return NULL;

   vhdl_expr *off = part_select_base(scope, lpm);;
   if (NULL == off)
      return NULL;

   selfrom->set_slice(off, ivl_lpm_width(lpm) - 1);
   return selfrom;
}

static int draw_part_select_pv_lpm(vhdl_arch *arch, ivl_lpm_t lpm)
{
   vhdl_var_ref *selfrom = nexus_to_var_ref(arch->get_scope(), ivl_lpm_data(lpm, 0));
   if (NULL == selfrom)
      return 1;

   vhdl_expr *off = part_select_base(arch->get_scope(), lpm);;
   if (NULL == off)
      return 1;

   vhdl_var_ref *out = nexus_to_var_ref(arch->get_scope(), ivl_lpm_q(lpm, 0));
   if (NULL == out)
      return 1;

   out->set_slice(off, ivl_lpm_width(lpm) - 1);
   arch->add_stmt(new vhdl_cassign_stmt(out, selfrom));
   return 0;
}

static vhdl_expr *draw_ufunc_lpm(vhdl_scope *scope, ivl_lpm_t lpm)
{
   vhdl_fcall *fcall = new vhdl_fcall(ivl_lpm_basename(lpm), NULL);

   for (unsigned i = 0; i < ivl_lpm_size(lpm); i++) {
      vhdl_var_ref *ref = nexus_to_var_ref(scope, ivl_lpm_data(lpm, i));
      if (NULL == ref)
         return NULL;

      fcall->add_expr(ref);
   }

   return fcall;
}

static vhdl_expr *draw_reduction_lpm(vhdl_scope *scope, ivl_lpm_t lpm,
                                     const char *rfunc, bool invert)
{
   vhdl_fcall *fcall = new vhdl_fcall(rfunc, vhdl_type::std_logic());

   vhdl_var_ref *ref = nexus_to_var_ref(scope, ivl_lpm_data(lpm, 0));
   if (NULL == ref)
      return NULL;
   
   fcall->add_expr(ref);

   if (invert)
      return new vhdl_unaryop_expr
         (VHDL_UNARYOP_NOT, fcall, vhdl_type::std_logic());
   else
      return fcall;
}

static vhdl_expr *draw_sign_extend_lpm(vhdl_scope *scope, ivl_lpm_t lpm)
{
   vhdl_expr *ref = nexus_to_var_ref(scope, ivl_lpm_data(lpm, 0));
   if (ref)
      return ref->resize(ivl_lpm_width(lpm));
   else
      return NULL;
}

vhdl_expr *lpm_to_expr(vhdl_scope *scope, ivl_lpm_t lpm)
{
   std::cout << "LPM type " << ivl_lpm_type(lpm) << std::endl;
   
   switch (ivl_lpm_type(lpm)) {
   case IVL_LPM_ADD:
      return draw_binop_lpm(scope, lpm, VHDL_BINOP_ADD);
   case IVL_LPM_SUB:
      return draw_binop_lpm(scope, lpm, VHDL_BINOP_SUB);
   case IVL_LPM_MULT:
      return draw_binop_lpm(scope, lpm, VHDL_BINOP_MULT);
   case IVL_LPM_CONCAT:
      return draw_binop_lpm(scope, lpm, VHDL_BINOP_CONCAT);
   case IVL_LPM_PART_VP:
      return draw_part_select_vp_lpm(scope, lpm);
   case IVL_LPM_UFUNC:
      return draw_ufunc_lpm(scope, lpm);
   case IVL_LPM_RE_AND:
      return draw_reduction_lpm(scope, lpm, "Reduce_AND", false);
   case IVL_LPM_RE_NAND:
      return draw_reduction_lpm(scope, lpm, "Reduce_AND", true);
   case IVL_LPM_RE_NOR:
      return draw_reduction_lpm(scope, lpm, "Reduce_OR", true);
   case IVL_LPM_RE_OR:
      return draw_reduction_lpm(scope, lpm, "Reduce_OR", false);
   case IVL_LPM_RE_XOR:
      return draw_reduction_lpm(scope, lpm, "Reduce_XOR", false);
   case IVL_LPM_RE_XNOR:
      return draw_reduction_lpm(scope, lpm, "Reduce_XNOR", false);
   case IVL_LPM_SIGN_EXT:
      return draw_sign_extend_lpm(scope, lpm);
   default:
      error("Unsupported LPM type: %d", ivl_lpm_type(lpm));
      return NULL;
   }
}

int draw_lpm(vhdl_arch *arch, ivl_lpm_t lpm)
{
   if (ivl_lpm_type(lpm) == IVL_LPM_PART_PV)
      return draw_part_select_pv_lpm(arch, lpm);
   else {
      vhdl_expr *f = lpm_to_expr(arch->get_scope(), lpm);
      if (NULL == f)
         return 1;
        
      vhdl_var_ref *out = nexus_to_var_ref(arch->get_scope(), ivl_lpm_q(lpm, 0));
      if (out)
         arch->add_stmt(new vhdl_cassign_stmt(out, f));
      
      return 0;
   }
}

