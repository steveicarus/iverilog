/*
 *  VHDL variable and signal types.
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

#include "vhdl_type.hh"

#include <sstream>


vhdl_type *vhdl_type::std_logic()
{
   return new vhdl_type(VHDL_TYPE_STD_LOGIC);
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

vhdl_type *vhdl_type::nunsigned(int width)
{
   return new vhdl_type(VHDL_TYPE_UNSIGNED, width-1, 0);
}

vhdl_type *vhdl_type::nsigned(int width)
{
   return new vhdl_type(VHDL_TYPE_SIGNED, width-1, 0);
}

vhdl_type *vhdl_type::time()
{
   return new vhdl_type(VHDL_TYPE_TIME);
}

/*
 * This is just the name of the type, without any parameters.
 */
std::string vhdl_type::get_string() const
{
   switch (name_) {
   case VHDL_TYPE_STD_LOGIC:
      return std::string("std_logic");
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

void vhdl_type::emit(std::ostream &of, int level) const
{
   of << get_decl_string();
}

vhdl_type *vhdl_type::std_logic_vector(int msb, int lsb)
{
   return new vhdl_type(VHDL_TYPE_STD_LOGIC_VECTOR, msb, lsb);
}
