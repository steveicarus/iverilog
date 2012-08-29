/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "Attrib.h"
# include  <cassert>

Attrib::Attrib()
{
      nlist_ = 0;
      list_ = 0;
}

Attrib::~Attrib()
{
      delete[] list_;
}


const verinum& Attrib::attribute(perm_string key) const
{
      for (unsigned idx = 0 ;  idx < nlist_ ;  idx += 1) {

	    if (key == list_[idx].key)
		  return list_[idx].val;
      }

      static const verinum null;
      return null;
}

void Attrib::attribute(perm_string key, const verinum&value)
{
      unsigned idx;

      for (idx = 0 ; idx < nlist_ ;  idx += 1) {
	    if (key == list_[idx].key) {
		  list_[idx].val = value;
		  return;
	    }
      }

      struct cell_*tmp = new struct cell_[nlist_+1];
      for (idx = 0 ;  idx < nlist_ ;  idx += 1)
	    tmp[idx] = list_[idx];

      tmp[nlist_].key = key;
      tmp[nlist_].val = value;

      nlist_ += 1;
      delete[]list_;
      list_ = tmp;
}

bool Attrib::has_compat_attributes(const Attrib&that) const
{
      unsigned idx;

      for (idx = 0 ;  idx < that.nlist_ ;  idx += 1) {

	    verinum tmp = attribute(that.list_[idx].key);
	    if (tmp != that.list_[idx].val)
		  return false;
      }

      return true;
}

unsigned Attrib::attr_cnt() const
{
      return nlist_;
}

perm_string Attrib::attr_key(unsigned idx) const
{
      assert(idx < nlist_);
      return list_[idx].key;
}

const verinum& Attrib::attr_value(unsigned idx) const
{
      assert(idx < nlist_);
      return list_[idx].val;
}
