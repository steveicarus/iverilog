/*
 * Copyright (c) 2011-2012 Stephen Williams (steve@icarus.com)
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


int VType::emit_decl(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;

      if (!reg_flag)
	    out << "wire ";

      errors += emit_def(out);

      out << " \\" << name << " ";
      return errors;
}

int VTypeArray::emit_def(ostream&out) const
{
      int errors = 0;

      list<const VTypeArray*> dims;
      const VTypeArray*cur = this;
      while (const VTypeArray*sub = dynamic_cast<const VTypeArray*> (cur->etype_)) {
	    dims.push_back(cur);
	    cur = sub;
      }

      const VTypePrimitive*base = dynamic_cast<const VTypePrimitive*> (cur->etype_);
      assert(base != 0);
      assert(dimensions() == 1);

      base->emit_primitive_type(out);
      if (signed_flag_)
	    out << " signed";

      dims.push_back(cur);

      while (! dims.empty()) {
	    cur = dims.front();
	    dims.pop_front();
	    out << "[";
	    errors += cur->dimension(0).msb()->emit(out, 0, 0);
	    out << ":";
	    errors += cur->dimension(0).lsb()->emit(out, 0, 0);
	    out << "]";
      }

      return errors;
}

int VTypeEnum::emit_def(ostream&out) const
{
      int errors = 0;
      out << "enum {";
      assert(names_.size() >= 1);
      out << "\\" << names_[0] << " ";
      for (size_t idx = 1 ; idx < names_.size() ; idx += 1)
	    out << ", \\" << names_[idx] << " ";

      out << "}";

      return errors;
}

int VTypePrimitive::emit_primitive_type(ostream&out) const
{
      int errors = 0;
      switch (type_) {
	  case BOOLEAN:
	  case BIT:
	    out << "bool";
	    break;
	  case STDLOGIC:
	    out << "logic";
	    break;
	  case INTEGER:
	    out << "bool [31:0]";
	    break;
	  default:
	    assert(0);
	    break;
      }
      return errors;
}

int VTypePrimitive::emit_def(ostream&out) const
{
      int errors = 0;
      errors += emit_primitive_type(out);
      return errors;
}

int VTypeRange::emit_def(ostream&out) const
{
      int errors = 0;
      assert(0);
      out << "/* Internal error: Don't know how to emit type */";
      return errors;
}

int VTypeRecord::emit_def(ostream&out) const
{
      int errors = 0;
      out << "struct packed {";

      for (vector<element_t*>::const_iterator cur = elements_.begin()
		 ; cur != elements_.end() ; ++cur) {
	    perm_string element_name = (*cur)->peek_name();
	    const VType*element_type = (*cur)->peek_type();
	    element_type->emit_def(out);
	    out << " \\" << element_name << " ; ";
      }

      out << "}";
      return errors;
}

int VTypeDef::emit_def(ostream&) const
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
      errors += type_->emit_def(out);
      out << " \\" << name_ << " ;" << endl;
      return errors;
}
