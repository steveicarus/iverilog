/*
 * Copyright (c) 2011-2012 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2014 CERN
 * @author Maciej Suminski (maciej.suminski@cern.ch)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

      errors += emit_def(out, name);
      out << " ";
      return errors;
}

int VType::emit_typedef(std::ostream&, typedef_context_t&) const
{
      return 0;
}

int VTypeERROR::emit_def(ostream&out, perm_string) const
{
      out << "/* ERROR */";
      return 1;
}

int VTypeArray::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      const VType*raw_base = basic_type();
      const VTypePrimitive*base = dynamic_cast<const VTypePrimitive*> (raw_base);

      if (base) {
	    assert(dimensions() == 1);

	    base->emit_def(out, empty_perm_string);
	    if (signed_flag_)
		  out << " signed";
      } else {
	    raw_base->emit_def(out, empty_perm_string);
      }

      errors += emit_with_dims_(out, raw_base->can_be_packed(), name);

      return errors;
}

int VTypeArray::emit_typedef(std::ostream&out, typedef_context_t&ctx) const
{
      return etype_->emit_typedef(out, ctx);
}

int VTypeArray::emit_with_dims_(std::ostream&out, bool packed, perm_string name) const
{
      int errors = 0;

      list<const VTypeArray*> dims;
      const VTypeArray*cur = this;
      while (const VTypeArray*sub = dynamic_cast<const VTypeArray*> (cur->element_type())) {
	    dims.push_back(cur);
	    cur = sub;
      }
      dims.push_back(cur);

      bool name_emitted = false;

      while (! dims.empty()) {
	    cur = dims.front();
	    dims.pop_front();

	    if(!packed) {
	        emit_name(out, name);
	        name_emitted = true;
	    }

	    for(unsigned i = 0; i < cur->dimensions(); ++i) {
	        if(cur->dimension(i).is_box() && !name_emitted) {
	            emit_name(out, name);
	            name_emitted = true;
	        }

	        out << "[";
	        if (!cur->dimension(i).is_box()) {  // if not unbounded {
	            errors += cur->dimension(i).msb()->emit(out, 0, 0);
	        out << ":";
	            errors += cur->dimension(i).lsb()->emit(out, 0, 0);
	        }
	        out << "]";
	    }
      }

      if(!name_emitted) {
	    emit_name(out, name);
      }

      return errors;
}

int VTypeEnum::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      out << "enum integer {";
      assert(names_.size() >= 1);
      out << "\\" << names_[0] << " ";
      for (size_t idx = 1 ; idx < names_.size() ; idx += 1)
	    out << ", \\" << names_[idx] << " ";

      out << "}";
      emit_name(out, name);

      return errors;
}

int VTypePrimitive::emit_primitive_type(ostream&out) const
{
      int errors = 0;
      switch (type_) {
	  case BIT:
	    out << "bit";
	    break;
	  case STDLOGIC:
	    out << "logic";
	    break;
	  case NATURAL:
	    out << "int unsigned";
	    break;
	  case INTEGER:
	    out << "int";
	    break;
	  case REAL:
	    out << "real";
	    break;
	  case CHARACTER:
	    out << "byte";
	    break;
	  case TIME:
	    out << "time";
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
      emit_name(out, name);
      return errors;
}

int VTypeRange::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      out << "/* Internal error: Don't know how to emit range */";
      errors += base_->emit_def(out, name);
      return errors;
}

int VTypeRecord::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      out << "struct packed {";

      for (vector<element_t*>::const_iterator cur = elements_.begin()
		 ; cur != elements_.end() ; ++cur) {
	    perm_string element_name = (*cur)->peek_name();
	    const VType*element_type = (*cur)->peek_type();
	    element_type->emit_def(out, empty_perm_string);
	    out << " \\" << element_name << " ; ";
      }

      out << "}";
      emit_name(out, name);
      return errors;
}

/*
 * For VTypeDef objects, use the name of the defined type as the
 * type. (We are defining a variable here, not the type itself.) The
 * emit_typedef() method was presumably called to define type already.
 */
int VTypeDef::emit_def(ostream&out, perm_string name) const
{
      int errors = 0;
      emit_name(out, name_);
      emit_name(out, name);
      return errors;
}

int VTypeDef::emit_decl(ostream&out, perm_string name, bool reg_flag) const
{
      int errors = 0;

      if (!dynamic_cast<const VTypeEnum*>(type_))
	    out << (reg_flag ? "reg " : "wire ");

      if(dynamic_cast<const VTypeArray*>(type_)) {
          errors += type_->emit_def(out, name);
      } else {
          assert(name_ != empty_perm_string);
          cout << "\\" << name_;
          emit_name(out, name);
      }

      return errors;
}

int VTypeDef::emit_typedef(ostream&out, typedef_context_t&ctx) const
{
	// The typedef_context_t is used by me to determine if this
	// typedef has already been emitted in this architecture. If
	// it has, then it is MARKED, give up. Otherwise, recurse the
	// emit_typedef to make sure all sub-types that I use have
	// been emitted, then emit my typedef.
      typedef_topo_t&flag = ctx[this];
      switch (flag) {
	  case MARKED:
	    return 0;
	  case PENDING:
	    out << "typedef \\" << name_ << " ; /* typedef cycle? */" << endl;
	    return 0;
	  case NONE:
	    break;
      }

      flag = PENDING;
      int errors = type_->emit_typedef(out, ctx);
      flag = MARKED;

      // Array types are used directly anyway and typedefs for unpacked
      // arrays do not work currently
      if(dynamic_cast<const VTypeArray*>(type_))
        out << "// ";

      out << "typedef ";
      errors += type_->emit_def(out, name_);
      out << " ;" << endl;

      return errors;
}
