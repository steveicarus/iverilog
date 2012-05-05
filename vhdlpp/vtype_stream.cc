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
# include  <typeinfo>
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

void VTypeArray::write_to_stream(ostream&fd) const
{
	// Special case: std_logic_vector
      if (etype_ == primitive_STDLOGIC) {
	    fd << "std_logic_vector";
	    if (! ranges_.empty() && ! ranges_[0].is_box()) {
		  assert(ranges_.size() < 2);
		  fd << " (";
		  if (ranges_[0].msb())
			ranges_[0].msb()->write_to_stream(fd);
		  else
			fd << "<>";
		  fd << " downto ";
		  if (ranges_[0].lsb())
			ranges_[0].lsb()->write_to_stream(fd);
		  else
			fd << "<>";
		  fd << ") ";
	    }
	    return;
      }

      fd << "array ";
      if (! ranges_.empty()) {
	    assert(ranges_.size() < 2);
	    if (ranges_[0].is_box()) {
		  fd << "(INTEGER range <>) ";
	    } else {
		  assert(ranges_[0].msb() && ranges_[0].lsb());
		  fd << "(";
		  if (ranges_[0].msb())
			ranges_[0].msb()->write_to_stream(fd);
		  else
			fd << "<>";
		  fd << " downto ";
		  if (ranges_[0].lsb())
			ranges_[0].lsb()->write_to_stream(fd);
		  else
			fd << "<>";
		  fd << ") ";
	    }
      }

      fd << "of ";
      etype_->write_to_stream(fd);
}

void VTypeDef::write_type_to_stream(ostream&fd) const
{
      type_->write_to_stream(fd);
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
	  case STDLOGIC:
	    fd << "std_logic";
	    break;
	  case BOOLEAN:
	    fd << "boolean";
	    break;
	  default:
	    assert(0);
	    fd << "/* PRIMITIVE: " << type_ << " */";
	    break;
      }
}

void VTypeRange::write_to_stream(ostream&fd) const
{
      base_->write_to_stream(fd);
      fd << " range " << min_ << " to " << max_;
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
