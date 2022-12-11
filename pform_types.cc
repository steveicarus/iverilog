/*
 * Copyright (c) 2007-2019 Stephen Williams (steve@icarus.com)
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


# include  "pform_types.h"

data_type_t::~data_type_t()
{
}

PNamedItem::SymbolType data_type_t::symbol_type() const
{
      return TYPE;
}

string_type_t::~string_type_t()
{
}

atom_type_t size_type (atom_type_t::INT, true);

PNamedItem::SymbolType enum_type_t::symbol_type() const
{
      return ENUM;
}

PNamedItem::SymbolType class_type_t::symbol_type() const
{
      return CLASS;
}

bool typedef_t::set_data_type(data_type_t *t)
{
      if (data_type.get())
	    return false;

      data_type.reset(t);

      return true;
}

bool typedef_t::set_basic_type(enum basic_type bt)
{
      if (bt == ANY)
	    return true;
      if (basic_type != ANY && bt != basic_type)
	    return false;

      basic_type = bt;

      return true;
}

std::ostream& operator<< (std::ostream&out, enum typedef_t::basic_type bt)
{
	switch (bt) {
	case typedef_t::ANY:
		out << "any";
		break;
	case typedef_t::ENUM:
		out << "enum";
		break;
	case typedef_t::STRUCT:
		out << "struct";
		break;
	case typedef_t::UNION:
		out << "union";
		break;
	case typedef_t::CLASS:
		out << "class";
		break;
	}

	return out;
}
