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

class vhdl_element {
public:
   virtual void emit(std::ofstream &of) = 0;
};

class vhdl_entity : public vhdl_element {
public:
   vhdl_entity(const char *name);

   void emit(std::ofstream &of);
private:
   const char *name_;
};

class vhdl_arch : public vhdl_element {
public:
   vhdl_arch(const char *entity, const char *name="Behavioural");

   void emit(std::ofstream &of);
private:
   const char *name_, *entity_;
};

#endif
