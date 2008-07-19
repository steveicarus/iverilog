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

void require_support_function(support_function_t f)
{
   vhdl_scope *scope = get_active_entity()->get_arch()->get_scope();
   if (!scope->have_declared(support_function::function_name(f)))
      scope->add_decl(new support_function(f));
}

const char *support_function::function_name(support_function_t type)
{
   switch (type) {
   case SF_UNSIGNED_TO_BOOLEAN:
      return "Unsigned_To_Boolean";
   case SF_SIGNED_TO_BOOLEAN:
      return "Signed_To_Boolean";
   case SF_BOOLEAN_TO_LOGIC:
      return "Boolean_To_Logic";
   default:
      assert(false);
   }
}

vhdl_type *support_function::function_type(support_function_t type)
{
   switch (type) {
   case SF_UNSIGNED_TO_BOOLEAN:
      return vhdl_type::boolean();
   case SF_SIGNED_TO_BOOLEAN:
      return vhdl_type::boolean();
   case SF_BOOLEAN_TO_LOGIC:
      return vhdl_type::std_logic();
   default:
      assert(false);
   }
}

void support_function::emit(std::ostream &of, int level) const
{
   of << "function " << function_name(type_);
   
   switch (type_) {
   case SF_UNSIGNED_TO_BOOLEAN:
      of << "(X : unsigned) return Boolean is";
      newline(of, level);
      of << "begin";
      newline(of, indent(level));
      of << "return X /= To_Unsigned(0, X'Length);";
      newline(of, level);
      break;
   case SF_SIGNED_TO_BOOLEAN:
      of << "(X : signed) return Boolean is";
      newline(of, level);
      of << "begin";
      newline(of, indent(level));
      of << "return X /= To_Signed(0, X'Length);";
      newline(of, level);
      break;
   case SF_BOOLEAN_TO_LOGIC:
      of << "(B : Boolean) return std_logic is";
      newline(of, level);
      of << "begin";
      newline(of, indent(level));
      of << "if B then";
      newline(of, indent(indent(level)));
      of << "return '1'";
      newline(of, indent(level));
      of << "else";
      newline(of, indent(indent(level)));
      of << "return '0'";
      newline(of, indent(level));
      of << "end if;";
      newline(of, level);
      break;
   default:
      assert(false);
   }
   
   of << "end function;";
   newline(of, level);
}
