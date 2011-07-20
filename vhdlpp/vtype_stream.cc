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
# include  <typeinfo>
# include  <cassert>

using namespace std;

void VType::write_to_stream(ostream&fd) const
{
      fd << "/* UNKNOWN TYPE: " << typeid(*this).name() << " */";
}

void VTypeArray::write_to_stream(ostream&fd) const
{
      fd << "array ";
      if (ranges_.size() > 0) {
	    assert(ranges_.size() < 2);
	    fd << "(" << ranges_[0].msb()
	       << " downto " << ranges_[0].lsb() << ") ";
      }

      fd << "of ";
      etype_->write_to_stream(fd);
}

void VTypePrimitive::write_to_stream(ostream&fd) const
{
      switch (type_) {
	  case INTEGER:
	    fd << "integer";
	    break;
	  case STDLOGIC:
	    fd << "std_logic";
	    break;
	  default:
	    assert(0);
	    fd << "/* PRIMITIVE: " << type_ << " */";
	    break;
      }
}
