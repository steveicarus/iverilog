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

#ifndef INC_SUPPORT_HH
#define INC_SUPPORT_HH

#include "vhdl_syntax.hh"

class unsigned_to_boolean : public vhdl_function {
public:
   unsigned_to_boolean()
      : vhdl_function(function_name(), vhdl_type::boolean()) {}
   void emit(std::ostream &of, int level) const;

   static const char *function_name() { return "Unsigned_To_Boolean"; }
};


class signed_to_boolean : public vhdl_function {
public:
   signed_to_boolean()
      : vhdl_function(function_name(), vhdl_type::boolean()) {}
   void emit(std::ostream &of, int level) const;

   static const char *function_name() { return "Signed_To_Boolean"; }
};

#endif
