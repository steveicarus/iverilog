/*
 * Copyright (c) 2007-2008 Stephen Williams (steve@icarus.com)
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

string_type_t::~string_type_t()
{
}

ivl_variable_type_t data_type_t::figure_packed_base_type(void) const
{
      return IVL_VT_NO_TYPE;
}

ivl_variable_type_t parray_type_t::figure_packed_base_type(void) const
{
      return base_type->figure_packed_base_type();
}

ivl_variable_type_t vector_type_t::figure_packed_base_type(void) const
{
      return base_type;
}

atom2_type_t size_type (32, true);
