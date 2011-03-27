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

void VTypeArray::elaborate(VType::decl_t&decl) const
{
      const VTypePrimitive*base = dynamic_cast<const VTypePrimitive*>(etype_);
      assert(base != 0);

      base->elaborate(decl);
	//assert(decl.msb == decl.lsb == 0);

      decl.msb = dimension(0).msb();
      decl.lsb = dimension(0).lsb();
      decl.signed_flag = signed_flag_;
}

void VTypePrimitive::elaborate(VType::decl_t&decl) const
{
      decl.type = VNONE;
      decl.signed_flag = false;
      decl.msb = 0;
      decl.lsb = 0;

      switch (type_) {
	  case BOOLEAN:
	  case BIT:
	    decl.type = VBOOL;
	    break;
	  case STDLOGIC:
	    decl.type = VLOGIC;
	    break;
	  case INTEGER:
	    decl.type = VBOOL;
	    decl.msb = 31;
	    decl.lsb = 0;
	    break;
      }
}
