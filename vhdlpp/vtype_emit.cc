/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


# include  "vtype.h"
# include <iostream>
# include  <typeinfo>
# include  <cassert>

using namespace std;


int VType::decl_t::emit(ostream&out, perm_string name) const
{
      return type->emit(out, name, reg_flag);
}


int VTypeArray::emit(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      const VTypePrimitive*base = dynamic_cast<const VTypePrimitive*> (etype_);
      assert(base != 0);
      assert(dimensions() == 1);

      if (reg_flag)
	    out << "reg ";
      else
	    out << "wire ";

      base->emit_primitive_type(out);
      if (signed_flag_)
	    out << "signed ";

      out << "[" << dimension(0).msb() << ":" << dimension(0).lsb() << "] ";

      out << "\\" << name << " ";

      return errors;
}

int VTypeEnum::emit(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      assert(0);
      return errors;
}

int VTypePrimitive::emit_primitive_type(ostream&out) const
{
      int errors = 0;
      switch (type_) {
	  case BOOLEAN:
	  case BIT:
	    out << "bool ";
	    break;
	  case STDLOGIC:
	    out << "logic ";
	    break;
	  case INTEGER:
	    out << "bool [31:0] ";
	    break;
	  default:
	    assert(0);
	    break;
      }
      return errors;
}

int VTypePrimitive::emit(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      if (reg_flag)
	    out << "reg ";
      else
	    out << "wire ";

      errors += emit_primitive_type(out);

      out << "\\" << name << " ";

      return errors;
}

int VTypeRange::emit(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      assert(0);
      return errors;
}
