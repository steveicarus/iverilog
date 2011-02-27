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

using namespace std;

std::map<perm_string, const VType*> global_types;

const VTypePrimitive primitive_BOOLEAN (VTypePrimitive::BOOLEAN);
const VTypePrimitive primitive_BIT     (VTypePrimitive::BIT);;
const VTypePrimitive primitive_INTEGER (VTypePrimitive::INTEGER);;
const VTypePrimitive primitive_STDLOGIC(VTypePrimitive::STDLOGIC);;

const VTypeArray primitive_BIT_VECTOR(&primitive_BIT,      vector<VTypeArray::range_t> (1));
const VTypeArray primitive_BOOL_VECTOR(&primitive_BOOLEAN, vector<VTypeArray::range_t> (1));

void preload_global_types(void)
{
      global_types[perm_string::literal("boolean")]   = &primitive_BOOLEAN;
      global_types[perm_string::literal("bit")]       = &primitive_BIT;
      global_types[perm_string::literal("integer")]   = &primitive_INTEGER;
      global_types[perm_string::literal("std_logic")] = &primitive_STDLOGIC;
      global_types[perm_string::literal("bit_vector")]= &primitive_BOOL_VECTOR;
}

void import_ieee(void)
{
}

static void import_ieee_use_std_logic_1164(perm_string name)
{
      bool all_flag = name=="all";

      if (all_flag || name == "std_logic_vector") {
	    vector<VTypeArray::range_t> dims (1);
	    global_types[perm_string::literal("std_logic_vector")] = new VTypeArray(&primitive_STDLOGIC, dims, false);
      }
}

static void import_ieee_use_numeric_bit(perm_string name)
{
      bool all_flag = name=="all";

      if (all_flag || name == "signed") {
	    vector<VTypeArray::range_t> dims (1);
	    global_types[perm_string::literal("signed")] = new VTypeArray(&primitive_STDLOGIC, dims, true);
      }
      if (all_flag || name == "unsigned") {
	    vector<VTypeArray::range_t> dims (1);
	    global_types[perm_string::literal("unsigned")] = new VTypeArray(&primitive_BIT, dims, false);
      }
}

static void import_ieee_use_numeric_std(perm_string name)
{
      bool all_flag = name=="all";

      if (all_flag || name == "signed") {
	    vector<VTypeArray::range_t> dims (1);
	    global_types[perm_string::literal("signed")] = new VTypeArray(&primitive_STDLOGIC, dims, true);
      }
      if (all_flag || name == "unsigned") {
	    vector<VTypeArray::range_t> dims (1);
	    global_types[perm_string::literal("unsigned")] = new VTypeArray(&primitive_STDLOGIC, dims, false);
      }
}

void import_ieee_use(perm_string package, perm_string name)
{
      if (package == "std_logic_1164") {
	    import_ieee_use_std_logic_1164(name);
	    return;
      }

      if (package == "numeric_bit") {
	    import_ieee_use_numeric_bit(name);
	    return;
      }

      if (package == "numeric_std") {
	    import_ieee_use_numeric_std(name);
	    return;
      }
}

VType::~VType()
{
}

void VType::show(ostream&out) const
{
      out << typeid(*this).name();
}

VTypePrimitive::VTypePrimitive(VTypePrimitive::type_t tt)
: type_(tt)
{
}

VTypePrimitive::~VTypePrimitive()
{
}

void VTypePrimitive::show(ostream&out) const
{
      switch (type_) {
	  case BOOLEAN:
	    out << "BOOLEAN";
	    break;
	  case BIT:
	    out << "BIT";
	    break;
	  case INTEGER:
	    out << "INTEGER";
	    break;
	  case STDLOGIC:
	    out << "std_logic";
	    break;
      }
}

VTypeArray::VTypeArray(const VType*element, const vector<VTypeArray::range_t>&r, bool sv)
: etype_(element), ranges_(r), signed_flag_(sv)
{
}

VTypeArray::~VTypeArray()
{
}

size_t VTypeArray::dimensions() const
{
      return ranges_.size();
}

const VType* VTypeArray::element_type() const
{
      return etype_;
}

void VTypeArray::show(ostream&out) const
{
      out << "array ";
      for (vector<range_t>::const_iterator cur = ranges_.begin()
		 ; cur != ranges_.end() ; ++cur) {
	    out << "(" << cur->msb() << " downto " << cur->lsb() << ")";
      }
      out << " of ";
      if (signed_flag_)
	    out << "signed ";
      if (etype_)
	    etype_->show(out);
      else
	    out << "<nil>";
}
