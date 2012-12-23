/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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

# include  "vvp_cobject.h"
# include  "class_type.h"
# include  <iostream>
# include  <cassert>

using namespace std;

vvp_cobject::vvp_cobject(const class_type*defn)
: defn_(defn), properties_(new int32_t[defn->property_count()])
{
}

vvp_cobject::~vvp_cobject()
{
      delete[]properties_;
      properties_ = 0;
}

void vvp_cobject::set_vec4(size_t pid, const vvp_vector4_t&val)
{
      assert(pid < defn_->property_count());

      int32_t tmp;
      bool flag = vector4_to_value(val, tmp, true, false);
      assert(flag);

      properties_[pid] = tmp;
}

void vvp_cobject::get_vec4(size_t pid, vvp_vector4_t&val)
{
      assert(pid < defn_->property_count());

      unsigned long tmp[1];
      tmp[0] = properties_[pid];
      val.resize(32);
      val.setarray(0, 32, tmp);
}
