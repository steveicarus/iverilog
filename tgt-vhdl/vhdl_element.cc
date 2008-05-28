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


//////// ENTITY ////////

vhdl_entity::vhdl_entity(const char *name)
   : name_(name)
{

}

void vhdl_entity::emit(std::ofstream &of)
{
   of << "entity " << name_ << " is" << std::endl;
   // ...ports...
   of << "end entity; " << std::endl;
}


//////// ARCHITECTURE ////////

vhdl_arch::vhdl_arch(const char *entity, const char *name)
   : name_(name), entity_(entity)
{
   
}

void vhdl_arch::emit(std::ofstream &of)
{
   of << "architecture " << name_ << " of " << entity_;
   of << " is" << std::endl;
   // ...declarations...
   of << "begin" << std::endl;
   // ...statements...
   of << "end architecture;" << std::endl;   
}
