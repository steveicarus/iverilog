/*
 *  VHDL variable and signal types.
 *
 *  Copyright (C) 2008-2021  Nick Gasson (nick@nickg.me.uk)
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

#include "vhdl_type.hh"

#include <cassert>
#include <sstream>
#include <iostream>

vhdl_type *vhdl_type::std_logic()
{
   return new vhdl_type(VHDL_TYPE_STD_LOGIC);
}

vhdl_type *vhdl_type::std_ulogic()
{
   return new vhdl_type(VHDL_TYPE_STD_ULOGIC);
}

vhdl_type *vhdl_type::string()
{
   return new vhdl_type(VHDL_TYPE_STRING);
}

vhdl_type *vhdl_type::line()
{
   return new vhdl_type(VHDL_TYPE_LINE);
}

vhdl_type *vhdl_type::boolean()
{
   return new vhdl_type(VHDL_TYPE_BOOLEAN);
}

vhdl_type *vhdl_type::integer()
{
   return new vhdl_type(VHDL_TYPE_INTEGER);
}

vhdl_type *vhdl_type::nunsigned(int width, int lsb)
{
   return new vhdl_type(VHDL_TYPE_UNSIGNED, width-1+lsb, lsb);
}

vhdl_type *vhdl_type::nsigned(int width, int lsb)
{
   return new vhdl_type(VHDL_TYPE_SIGNED, width-1+lsb, lsb);
}

vhdl_type *vhdl_type::time()
{
   return new vhdl_type(VHDL_TYPE_TIME);
}

vhdl_type *vhdl_type::get_base() const
{
   assert(name_ == VHDL_TYPE_ARRAY);
   return base_;
}

/*
 * This is just the name of the type, without any parameters.
 */
std::string vhdl_type::get_string() const
{
   switch (name_) {
   case VHDL_TYPE_STD_LOGIC:
      return std::string("std_logic");
   case VHDL_TYPE_STD_ULOGIC:
      return std::string("std_ulogic");
   case VHDL_TYPE_STD_LOGIC_VECTOR:
      return std::string("std_logic_vector");
   case VHDL_TYPE_STRING:
      return std::string("String");
   case VHDL_TYPE_LINE:
      return std::string("Line");
   case VHDL_TYPE_FILE:
      return std::string("File");
   case VHDL_TYPE_INTEGER:
      return std::string("Integer");
   case VHDL_TYPE_BOOLEAN:
      return std::string("Boolean");
   case VHDL_TYPE_SIGNED:
      return std::string("signed");
   case VHDL_TYPE_UNSIGNED:
      return std::string("unsigned");
   case VHDL_TYPE_ARRAY:
      // Each array has its own type declaration
      return array_name_;
   default:
      return std::string("BadType");
   }
}

/*
 * The is the qualified name of the type.
 */
std::string vhdl_type::get_decl_string() const
{
   switch (name_) {
   case VHDL_TYPE_STD_LOGIC_VECTOR:
   case VHDL_TYPE_UNSIGNED:
   case VHDL_TYPE_SIGNED:
      {
         std::ostringstream ss;
         ss << get_string() << "(" << msb_;
         ss << " downto " << lsb_ << ")";
         return ss.str();
      }
   default:
      return get_string();
   }
}

/*
 * Like get_decl_string but completely expands array declarations.
 */
std::string vhdl_type::get_type_decl_string() const
{
   switch (name_) {
   case VHDL_TYPE_ARRAY:
      {
         std::ostringstream ss;
         ss << "array (" << msb_ << " downto "
            << lsb_ << ") of "
            << base_->get_decl_string();
         return ss.str();
      }
   default:
      return get_decl_string();
   }
}

void vhdl_type::emit(std::ostream &of, int) const
{
   of << get_decl_string();
}

vhdl_type::vhdl_type(const vhdl_type &other)
   : vhdl_element(other), name_(other.name_),
     msb_(other.msb_), lsb_(other.lsb_), array_name_(other.array_name_)
{
   if (other.base_ != NULL)
      base_ = new vhdl_type(*other.base_);
   else
      base_ = NULL;
}

vhdl_type::~vhdl_type()
{
   delete base_;
}

vhdl_type *vhdl_type::std_logic_vector(int msb, int lsb)
{
   return new vhdl_type(VHDL_TYPE_STD_LOGIC_VECTOR, msb, lsb);
}

vhdl_type *vhdl_type::type_for(int width, bool issigned,
                               int lsb, bool unresolved)
{
   if (width == 1) {
      if (unresolved)
         return vhdl_type::std_ulogic();
      else
         return vhdl_type::std_logic();
   }
   else if (issigned)
      return vhdl_type::nsigned(width, lsb);
   else
      return vhdl_type::nunsigned(width, lsb);
}

vhdl_type *vhdl_type::array_of(vhdl_type *b, const std::string &n, int m, int l)
{
   return new vhdl_type(b, n, m, l);
}
