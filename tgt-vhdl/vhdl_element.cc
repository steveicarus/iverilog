/*
 *  VHDL abstract syntax elements.
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

#include "vhdl_element.hh"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <typeinfo>
#include <iostream>
#include <sstream>


static const int VHDL_INDENT = 2;  // Spaces to indent

int indent(int level)
{
   return level + VHDL_INDENT;
}

std::string nl_string(int level)
{
   std::ostringstream ss;
   newline(ss, level);
   return ss.str();           
}

/*
 * Emit a newline and indent to the correct level.
 */
void newline(std::ostream &of, int level)
{
   of << std::endl;
   while (level--)
      of << ' ';
}

void blank_line(std::ostream &of, int level)
{
   of << std::endl;
   newline(of, level);
}

void vhdl_element::set_comment(std::string comment)
{
   comment_ = comment;
}

/*
 * Draw the comment for any element. The comment is either on
 * a line before the element (end_of_line is false) or at the
 * end of the line containing the element (end_of_line is true).
 */
void vhdl_element::emit_comment(std::ostream &of, int level,
                                bool end_of_line) const
{
   if (comment_.size() > 0) {
      if (end_of_line)
         of << "  ";
      of << "-- " << comment_;
      if (!end_of_line)
         newline(of, level);
   }
}

void vhdl_element::print() const
{
   emit(std::cout, 0);
   std::cout << std::endl;
}
