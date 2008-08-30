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

#ifndef INC_VHDL_ELEMENT_HH
#define INC_VHDL_ELEMENT_HH

#include <fstream>
#include <list>
#include <string>

typedef std::list<std::string> string_list_t;

/*
 * Any VHDL syntax element. Each element can also contain a comment.
 */
class vhdl_element {
public:
   virtual ~vhdl_element() {}
   
   virtual void emit(std::ostream &of, int level=0) const = 0;
   void print() const;

   void set_comment(std::string comment);
protected:
   void emit_comment(std::ostream &of, int level,
                     bool end_of_line=false) const;
private:
   std::string comment_;
};

typedef std::list<vhdl_element*> element_list_t;

int indent(int level);
void newline(std::ostream &of, int level);
std::string nl_string(int level);
void blank_line(std::ostream &of, int level);

#endif

