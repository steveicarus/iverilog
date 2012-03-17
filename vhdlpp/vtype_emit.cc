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
# include  "expression.h"
# include  <iostream>
# include  <typeinfo>
# include  <cassert>

using namespace std;


int VType::decl_t::emit(ostream&out, perm_string name) const
{
      return type->emit_decl(out, name, reg_flag);
}


int VTypeArray::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;

      if (const VTypeArray*sub = dynamic_cast<const VTypeArray*> (etype_)) {
	    sub->emit_def(out, name);
	    assert(dimensions() == 1);
	    out << "[";
	    errors += dimension(0).msb()->emit(out, 0, 0);
	    out << ":";
	    errors += dimension(0).lsb()->emit(out, 0, 0);
	    out << "] ";
	    return errors;
      }

      const VTypePrimitive*base = dynamic_cast<const VTypePrimitive*> (etype_);
      assert(base != 0);
      assert(dimensions() == 1);

      base->emit_primitive_type(out);
      if (signed_flag_)
	    out << "signed ";

      out << "[";
      errors += dimension(0).msb()->emit(out, 0, 0);
      out << ":";
      errors += dimension(0).lsb()->emit(out, 0, 0);
      out << "] ";

      out << "\\" << name << " ";

      return errors;
}

int VTypeArray::emit_decl(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      assert(dimensions() == 1);

      if (reg_flag)
	    out << "reg ";
      else
	    out << "wire ";

      errors += emit_def(out, name);

      return errors;
}

int VTypeEnum::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      out << "enum {";
      assert(names_.size() >= 1);
      out << "\\" << names_[0] << " ";
      for (size_t idx = 1 ; idx < names_.size() ; idx += 1)
	    out << ", \\" << names_[idx] << " ";

      out << "} \\" << name << " ";

      return errors;
}

int VTypeEnum::emit_decl(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      errors += emit_def(out, name);
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

int VTypePrimitive::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      errors += emit_primitive_type(out);
      out << "\\" << name << " ";
      return errors;
}

int VTypePrimitive::emit_decl(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      if (reg_flag)
	    out << "reg ";
      else
	    out << "wire ";

      errors += emit_def(out, name);

      return errors;
}

int VTypeRange::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      assert(0);
      return errors;
}

int VTypeRange::emit_decl(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      assert(0);
      return errors;
}

int VTypeRecord::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      assert(0);
      return errors;
}

int VTypeRecord::emit_decl(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      assert(0);
      return errors;
}

int VTypeDef::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      assert(0);
      return errors;
}

int VTypeDef::emit_decl(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;
      if (reg_flag)
	    out << "reg ";
      else
	    out << "wire ";

      out << "\\" << name_ << " \\" << name << " ";
      return errors;
}

int VTypeDef::emit_typedef(ostream&out) const
{
      int errors = 0;
      out << "typedef ";
      errors += type_->emit_def(out, name_);
      out << ";" << endl;
      return errors;
}
