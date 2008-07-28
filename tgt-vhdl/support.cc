/*
 *  Support functions for VHDL output.
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
#include "support.hh"

#include <cassert>
#include <iostream>

void require_support_function(support_function_t f)
{
   vhdl_scope *scope = get_active_entity()->get_arch()->get_scope();
   if (!scope->have_declared(support_function::function_name(f)))
      scope->add_decl(new support_function(f));
}

const char *support_function::function_name(support_function_t type)
{
   switch (type) {
   case SF_UNSIGNED_TO_BOOLEAN: return "Unsigned_To_Boolean";
   case SF_SIGNED_TO_BOOLEAN:   return "Signed_To_Boolean";
   case SF_BOOLEAN_TO_LOGIC:    return "Boolean_To_Logic";
   case SF_REDUCE_OR:           return "Reduce_OR";
   case SF_REDUCE_AND:          return "Reduce_AND";
   case SF_REDUCE_XOR:          return "Reduce_XOR";
   case SF_TERNARY_LOGIC:       return "Ternary_Logic";
   case SF_TERNARY_UNSIGNED:    return "Ternary_Unsigned";
   case SF_TERNARY_SIGNED:      return "Ternary_Signed";
   default:
      assert(false);
   }
}

vhdl_type *support_function::function_type(support_function_t type)
{
   switch (type) {
   case SF_UNSIGNED_TO_BOOLEAN:
   case SF_SIGNED_TO_BOOLEAN:
      return vhdl_type::boolean();
   case SF_BOOLEAN_TO_LOGIC:
   case SF_REDUCE_OR:
   case SF_REDUCE_AND:
   case SF_REDUCE_XOR:
   case SF_TERNARY_LOGIC:
      return vhdl_type::std_logic();
   default:
      assert(false);
   }
}

void support_function::emit_ternary(std::ostream &of, int level) const
{
   of << "begin" << nl_string(indent(level))
      << "if T then return X; else return Y; end if;"
      << nl_string(level);
}

void support_function::emit(std::ostream &of, int level) const
{
   of << "function " << function_name(type_);
   
   switch (type_) {
   case SF_UNSIGNED_TO_BOOLEAN:
      of << "(X : unsigned) return Boolean is" << nl_string(level)
         << "begin" << nl_string(indent(level))
         << "return X /= To_Unsigned(0, X'Length);" << nl_string(level);
      break;
   case SF_SIGNED_TO_BOOLEAN:
      of << "(X : signed) return Boolean is" << nl_string(level)
         << "begin" << nl_string(indent(level))
         << "return X /= To_Signed(0, X'Length);" << nl_string(level);
      break;
   case SF_BOOLEAN_TO_LOGIC:
      of << "(B : Boolean) return std_logic is" << nl_string(level)
         << "begin" << nl_string(indent(level))
         << "if B then" << nl_string(indent(indent(level)))
         << "return '1';" << nl_string(indent(level))
         << "else" << nl_string(indent(indent(level)))
         << "return '0';" << nl_string(indent(level))
         << "end if;" << nl_string(level);
      break;
   case SF_REDUCE_OR:
      of << "(X : std_logic_vector) return std_logic is" << nl_string(level)
         << "begin" << nl_string(indent(level))
         << "for I in X'Range loop" << nl_string(indent(indent(level)))
         << "if X(I) = '1' then" << nl_string(indent(indent(indent(level))))
         << "return '1';" << nl_string(indent(indent(level)))
         << "end if;" << nl_string(indent(level))
         << "end loop;" << nl_string(indent(level))
         << "return '0';" << nl_string(level);
      break;
   case SF_REDUCE_AND:
      of << "(X : std_logic_vector) return std_logic is" << nl_string(level)
         << "begin" << nl_string(indent(level))
         << "for I in X'Range loop" << nl_string(indent(indent(level)))
         << "if X(I) = '0' then" << nl_string(indent(indent(indent(level))))
         << "return '0';" << nl_string(indent(indent(level)))
         << "end if;" << nl_string(indent(level))
         << "end loop;" << nl_string(indent(level))
         << "return '1';" << nl_string(level);
      break;
   case SF_REDUCE_XOR:
      of << "(X : std_logic_vector) return std_logic is"
         << nl_string(indent(level))
         << "variable R : std_logic := '0';" << nl_string(level)
         << "begin" << nl_string(indent(level))
         << "for I in X'Range loop" << nl_string(indent(indent(level)))
         << "R := X(I) xor R;" << nl_string(indent(level))
         << "end loop;" << nl_string(indent(level))
         << "return R;" << nl_string(level);
      break;
   case SF_TERNARY_LOGIC:
      of << "(T : Boolean; X, Y : std_logic) return std_logic is"
         << nl_string(level);
      emit_ternary(of, level);
      break;
   case SF_TERNARY_SIGNED:
      of << "(T : Boolean; X, Y : signed) return signed is"
         << nl_string(level);
      emit_ternary(of, level);
      break;
   case SF_TERNARY_UNSIGNED:
      of << "(T : Boolean; X, Y : signed) return unsigned is"
         << nl_string(level);
      emit_ternary(of, level);
      break;
   default:
      assert(false);
   }
   
   of << "end function;" << nl_string(level);
}
