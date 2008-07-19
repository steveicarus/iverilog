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

void unsigned_to_boolean::emit(std::ostream &of, int level) const
{
   of << "function " << function_name()
      << "(X : unsigned) return Boolean is";
   newline(of, level);
   of << "begin";
   newline(of, indent(level));
   of << "return X /= To_Unsigned(0, X'Length);";
   newline(of, level);
   of << "end function;";
   newline(of, level);
}

void signed_to_boolean::emit(std::ostream &of, int level) const
{
   of << "function " << function_name()
      << "(X : signed) return Boolean is";
   newline(of, level);
   of << "begin";
   newline(of, indent(level));
   of << "return X /= To_Signed(0, X'Length);";
   newline(of, level);
   of << "end function;";
   newline(of, level);
}
