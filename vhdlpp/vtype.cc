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

using namespace std;

std::map<perm_string, const VType*> global_types;

const VTypePrimitive primitive_BOOLEAN (VTypePrimitive::BOOLEAN);
const VTypePrimitive primitive_BIT     (VTypePrimitive::BIT);;
const VTypePrimitive primitive_INTEGER (VTypePrimitive::INTEGER);;
const VTypePrimitive primitive_STDLOGIC(VTypePrimitive::STDLOGIC);;

void preload_global_types(void)
{
      global_types[perm_string::literal("boolean")]   = &primitive_BOOLEAN;
      global_types[perm_string::literal("bit")]       = &primitive_BIT;
      global_types[perm_string::literal("integer")]   = &primitive_INTEGER;
      global_types[perm_string::literal("std_logic")] = &primitive_STDLOGIC;

      { vector<VTypeArray::range_t> dims (1);
	global_types[perm_string::literal("unsigned")] = new VTypeArray(&primitive_BIT, dims);
      }
}


VType::~VType()
{
}

VTypePrimitive::VTypePrimitive(VTypePrimitive::type_t tt)
: type_(tt)
{
}

VTypePrimitive::~VTypePrimitive()
{
}

VTypeArray::VTypeArray(const VType*element, const vector<VTypeArray::range_t>&r)
: etype_(element), ranges_(r)
{
}

VTypeArray::~VTypeArray()
{
}
