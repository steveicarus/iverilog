#ifndef __netenum_H
#define __netenum_H
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

# include  "ivl_target.h"
# include  "verinum.h"
# include  "StringHeap.h"
# include  <map>

class netenum_t {

    public:
      explicit netenum_t(ivl_variable_type_t base_type, bool signed_flag,
			 long msb, long lsb);
      ~netenum_t();

      bool insert_name(perm_string name, const verinum&val);

      typedef std::map<perm_string,verinum>::const_iterator iterator;
      iterator find_name(perm_string name) const;
      iterator end_name() const;

    private:
      ivl_variable_type_t base_type_;
      bool signed_flag_;
      long msb_, lsb_;

      std::map<perm_string,verinum> names_;
};

#endif
