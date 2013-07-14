/*
 * Copyright (c) 2013 Stephen Williams (steve@icarus.com)
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

# include  "nex_data.h"
# include  <cstdlib>
# include  <cstdio>
# include  <cstring>
# include  <cassert>

inline blif_nex_data_t::blif_nex_data_t(ivl_nexus_t nex)
: nex_(nex), name_(0)
{
}

blif_nex_data_t::~blif_nex_data_t()
{
      if (name_) free(name_);
}

blif_nex_data_t* blif_nex_data_t::get_nex_data(ivl_nexus_t nex)
{
      void*tmp = ivl_nexus_get_private(nex);
      if (tmp != 0) return reinterpret_cast<blif_nex_data_t*> (tmp);

      blif_nex_data_t*data = new blif_nex_data_t(nex);
      ivl_nexus_set_private(nex, data);
      return data;
}

void blif_nex_data_t::set_name(const char*txt)
{
      assert(name_ == 0);
      name_ = strdup(txt);
}

const char* blif_nex_data_t::get_name(void)
{
      if (name_) return name_;

      for (unsigned idx = 0 ; idx < ivl_nexus_ptrs(nex_) ; idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex_, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0)
		  continue;

	    name_ = strdup(ivl_signal_basename(sig));
	    break;
      }

      if (name_ == 0) {
	    char buf[64];
	    snprintf(buf, sizeof buf, "@%p", nex_);
	    name_ = strdup(buf);
      }

      assert(name_);
      return name_;
}
