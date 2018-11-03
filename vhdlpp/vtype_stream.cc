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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# define __STDC_LIMIT_MACROS
# include  "std_types.h"
# include  "expression.h"
# include  <typeinfo>
# include  <stdint.h>
# include  <cassert>

using namespace std;

void VType::write_to_stream(ostream&fd) const
{
      fd << "/* UNKNOWN TYPE: " << typeid(*this).name() << " */";
}

void VType::write_type_to_stream(ostream&fd) const
{
      write_to_stream(fd);
}

void VType::write_typedef_to_stream(ostream&fd, perm_string name) const
{
      if(is_global_type(name))
          return;

      fd << "type " << name << " is ";
      write_type_to_stream(fd);
      fd << ";" << endl;
}

void VTypeArray::write_to_stream(ostream&fd) const
{
      if(write_special_case(fd))
          return;

      bool typedefed = false;
      if(const VTypeDef*tdef = dynamic_cast<const VTypeDef*>(etype_)) {
            tdef->write_to_stream(fd);
            typedefed = true;
      } else {
            fd << "array ";
      }

      if (! ranges_.empty()) {
	    assert(ranges_.size() < 2);
	    if (ranges_[0].is_box()) {
		  fd << "(INTEGER range <>) ";
	    } else {
		  write_range_to_stream_(fd);
	    }
      }

      if(!typedefed) {
            fd << "of ";
            etype_->write_to_stream(fd);
      }
}

void VTypeArray::write_range_to_stream_(std::ostream&fd) const
{
    assert(ranges_.size() < 2);
    assert(ranges_[0].msb() && ranges_[0].lsb());

    fd << "(";
    if (ranges_[0].msb())
        ranges_[0].msb()->write_to_stream(fd);
    else
        fd << "<>";

    fd << (ranges_[0].is_downto() ? " downto " : " to ");

    if (ranges_[0].lsb())
        ranges_[0].lsb()->write_to_stream(fd);
    else
        fd << "<>";
    fd << ") ";
}

bool VTypeArray::write_special_case(std::ostream&fd) const
{
    if(this == &primitive_SIGNED) {
        fd << "signed";
    } else if(this == &primitive_UNSIGNED) {
        fd << "unsigned";
    } else if(etype_ == &primitive_STDLOGIC) {
        fd << "std_logic_vector";
    } else if(etype_ == &primitive_BIT) {
        fd << "bit_vector";
    } else if(etype_ == &primitive_CHARACTER) {
        fd << "string";
    } else {
        return false;
    }

    if(!ranges_.empty() && !ranges_[0].is_box()) {
        write_range_to_stream_(fd);
    }

    return true;
}

void VTypeArray::write_type_to_stream(ostream&fd) const
{
      if(write_special_case(fd))
          return;

      fd << "array ";

      // Unbounded array
      if (! ranges_.empty()) {
	    assert(ranges_.size() < 2);
	    if (ranges_[0].is_box()) {
		  fd << "(INTEGER range <>) ";
	    } else {
		  write_range_to_stream_(fd);
	    }
      }

      fd << "of ";

      if(const VTypeDef*tdef = dynamic_cast<const VTypeDef*>(etype_)) {
          tdef->write_to_stream(fd);
      } else {
          etype_->write_to_stream(fd);
      }
}

void VTypeDef::write_type_to_stream(ostream&fd) const
{
      type_->write_type_to_stream(fd);
}

void VTypeDef::write_to_stream(ostream&fd) const
{
      fd << name_;
}

void VTypePrimitive::write_to_stream(ostream&fd) const
{
      switch (type_) {
	  case BIT:
	    fd << "bit";
	    break;
	  case INTEGER:
	    fd << "integer";
	    break;
	  case NATURAL:
	    fd << "natural";
	    break;
	  case REAL:
	    fd << "real";
	    break;
	  case STDLOGIC:
	    fd << "std_logic";
	    break;
	  case TIME:
	    fd << "time";
	    break;
	  default:
	    assert(0);
	    fd << "/* PRIMITIVE: " << type_ << " */";
	    break;
      }
}

bool VTypeRange::write_std_types(ostream&fd) const
{
    // Detect some special cases that can be written as ieee or
    // standard types.
    if (const VTypePrimitive*tmp = dynamic_cast<const VTypePrimitive*>(base_)) {
        if (tmp->type()==VTypePrimitive::NATURAL) {
            fd << "natural";
            return true;
        }
    }

    return false;
}

void VTypeRangeConst::write_to_stream(ostream&fd) const
{
    if(write_std_types(fd))
        return;

    base_type()->write_to_stream(fd);
    fd << " range " << start_;
    fd << (start_ < end_ ? " to " : " downto ");
    fd << end_;
}

void VTypeRangeExpr::write_to_stream(ostream&fd) const
{
    if(write_std_types(fd))
        return;

    base_type()->write_to_stream(fd);
    fd << " range ";
    start_->write_to_stream(fd);
    fd << (downto_ ? " downto " : " to ");
    end_->write_to_stream(fd);
}

void VTypeRecord::write_to_stream(ostream&fd) const
{
      fd << "record ";
      for (size_t idx = 0 ; idx < elements_.size() ; idx += 1) {
	    elements_[idx]->write_to_stream(fd);
	    fd << "; ";
      }
      fd << "end record";
}

void VTypeRecord::element_t::write_to_stream(ostream&fd) const
{
      fd << name_ << ": ";
      type_->write_to_stream(fd);
}

void VTypeEnum::write_to_stream(std::ostream&fd) const
{
      fd << "(";
      for (vector<perm_string>::const_iterator it = names_.begin();
        it != names_.end(); ++it) {
            if(it != names_.begin())
                fd << ",";

            fd << *it;

      }
      fd << ")";
}

void VSubTypeDef::write_typedef_to_stream(ostream&fd, perm_string name) const
{
      if(is_global_type(name))
          return;

      fd << "subtype " << name << " is ";
      write_type_to_stream(fd);
      fd << ";" << endl;
}
