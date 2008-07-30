/*
 *  VHDL code generation for logic devices.
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
#include "vhdl_element.hh"

#include <cassert>

 
/*
 * Convert the inputs of a logic gate to a binary expression.
 */
static vhdl_expr *inputs_to_expr(vhdl_scope *scope, vhdl_binop_t op,
                                 ivl_net_logic_t log)
{
   // Not always std_logic but this is probably OK since
   // the program has already been type checked
   vhdl_binop_expr *gate =
      new vhdl_binop_expr(op, vhdl_type::std_logic());
   
   int npins = ivl_logic_pins(log);
   for (int i = 1; i < npins; i++) {
      ivl_nexus_t input = ivl_logic_pin(log, i);
      gate->add_expr(nexus_to_var_ref(scope, input));
   }

   return gate;
}

/*
 * Convert a gate intput to an unary expression.
 */
static vhdl_expr *input_to_expr(vhdl_scope *scope, vhdl_unaryop_t op,
                                ivl_net_logic_t log)
{
   ivl_nexus_t input = ivl_logic_pin(log, 1);
   assert(input);

   vhdl_expr *operand = nexus_to_var_ref(scope, input);
   return new vhdl_unaryop_expr(op, operand, vhdl_type::std_logic()); 
}

static void bufif_logic(vhdl_arch *arch, ivl_net_logic_t log, bool if0)
{
   ivl_nexus_t output = ivl_logic_pin(log, 0);
   vhdl_var_ref *lhs = nexus_to_var_ref(arch->get_scope(), output);
   assert(lhs);
   
   vhdl_expr *val = nexus_to_var_ref(arch->get_scope(), ivl_logic_pin(log, 1));
   assert(val);

   vhdl_expr *sel = nexus_to_var_ref(arch->get_scope(), ivl_logic_pin(log, 2));
   assert(val);

   vhdl_expr *on = new vhdl_const_bit(if0 ? '0' : '1');
   vhdl_expr *cmp = new vhdl_binop_expr(sel, VHDL_BINOP_EQ, on, NULL);

   ivl_signal_t sig = find_signal_named(lhs->get_name(), arch->get_scope());
   char zbit;
   switch (ivl_signal_type(sig)) {
   case IVL_SIT_TRI0:
      zbit = '0';
      break;
   case IVL_SIT_TRI1:
      zbit = '1';
      break;
   case IVL_SIT_TRI:
   default:
      zbit = 'Z';
   }
   
   vhdl_const_bit *z = new vhdl_const_bit(zbit);
   vhdl_cassign_stmt *cass = new vhdl_cassign_stmt(lhs, z);
   cass->add_condition(val, cmp);

   arch->add_stmt(cass);
}

static vhdl_expr *translate_logic_inputs(vhdl_scope *scope, ivl_net_logic_t log)
{
   switch (ivl_logic_type(log)) {
   case IVL_LO_NOT:
      return input_to_expr(scope, VHDL_UNARYOP_NOT, log);
   case IVL_LO_AND:
      return inputs_to_expr(scope, VHDL_BINOP_AND, log);
   case IVL_LO_OR:
      return inputs_to_expr(scope, VHDL_BINOP_OR, log);
   case IVL_LO_XOR:
      return inputs_to_expr(scope, VHDL_BINOP_XOR, log);
   case IVL_LO_BUF:
   case IVL_LO_BUFZ:
      return nexus_to_var_ref(scope, ivl_logic_pin(log, 1));
   case IVL_LO_PULLUP:
      return new vhdl_const_bit('1');
   case IVL_LO_PULLDOWN:
      return new vhdl_const_bit('0');
   default:
      error("Don't know how to translate logic type = %d to expression",
            ivl_logic_type(log));
      return NULL;
   }
}

void draw_logic(vhdl_arch *arch, ivl_net_logic_t log)
{
   switch (ivl_logic_type(log)) {
   case IVL_LO_BUFIF0:
      bufif_logic(arch, log, true);
      break;
   case IVL_LO_BUFIF1:
      bufif_logic(arch, log, false);
      break;
   default:
      {          
         // The output is always pin zero
         ivl_nexus_t output = ivl_logic_pin(log, 0);
         vhdl_var_ref *lhs = nexus_to_var_ref(arch->get_scope(), output);

         vhdl_expr *rhs = translate_logic_inputs(arch->get_scope(), log);
         vhdl_cassign_stmt *ass = new vhdl_cassign_stmt(lhs, rhs);
         
         ivl_expr_t delay = ivl_logic_delay(log, 1);
         if (delay)
            ass->set_after(translate_time_expr(delay));
         
         arch->add_stmt(ass);
      }
   }
}
