/*
 *  Support functions for VHDL output.
 *
 *  Copyright (C) 2008-2010  Nick Gasson (nick@nickg.me.uk)
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

enum support_function_t {
   SF_UNSIGNED_TO_BOOLEAN = 0,
   SF_SIGNED_TO_BOOLEAN,
   SF_BOOLEAN_TO_LOGIC,
   SF_REDUCE_OR,
   SF_REDUCE_AND,
   SF_REDUCE_XOR,
   SF_REDUCE_XNOR,
   SF_TERNARY_LOGIC,
   SF_TERNARY_UNSIGNED,
   SF_TERNARY_SIGNED,
   SF_LOGIC_TO_INTEGER,
   SF_SIGNED_TO_LOGIC,
   SF_UNSIGNED_TO_LOGIC
};

class support_function : public vhdl_function {
public:
   explicit support_function(support_function_t type)
      : vhdl_function(function_name(type), function_type(type)),
        type_(type) {}
   void emit(std::ostream &of, int level) const;
   static const char *function_name(support_function_t type);
   static vhdl_type *function_type(support_function_t type);

private:
   void emit_ternary(std::ostream &of, int level) const;
   void emit_reduction(std::ostream &of, int level, const char *op,
                       char unit) const;

   support_function_t type_;
};

#endif
