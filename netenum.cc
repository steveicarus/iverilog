/*
 * Copyright (c) 2010 Stephen Williams (steve@icarus.com)
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

# include  "netenum.h"

netenum_t::netenum_t(ivl_variable_type_t base_type, bool signed_flag, long msb, long lsb)
: base_type_(base_type), signed_flag_(signed_flag), msb_(msb), lsb_(lsb)
{
}

netenum_t::~netenum_t()
{
}

bool netenum_t::insert_name(perm_string name, const verinum&val)
{
      names_[name] = verinum(val, msb_-lsb_+1);
      return true;
}

netenum_t::iterator netenum_t::find_name(perm_string name) const
{
      return names_.find(name);
}

netenum_t::iterator netenum_t::end_name() const
{
      return names_.end();
}
