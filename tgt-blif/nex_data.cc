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
# include  <iostream>
# include  <cstdlib>
# include  <cstdio>
# include  <cstring>
# include  <cassert>

using namespace std;

inline blif_nex_data_t::blif_nex_data_t(ivl_nexus_t nex)
: nex_(nex), name_(0)
{
}

blif_nex_data_t::~blif_nex_data_t()
{
      if (name_) free(name_);

      for (size_t idx = 0 ; idx < name_index_.size() ; idx += 1)
	    if (name_index_[idx]) free(name_index_[idx]);
}

blif_nex_data_t* blif_nex_data_t::get_nex_data(ivl_nexus_t nex)
{
      void*tmp = ivl_nexus_get_private(nex);
      if (tmp != 0) return reinterpret_cast<blif_nex_data_t*> (tmp);

      blif_nex_data_t*data = new blif_nex_data_t(nex);
      ivl_nexus_set_private(nex, data);
      return data;
}

void blif_nex_data_t::make_name_from_sig_(ivl_signal_t sig)
{
      assert(name_ == 0);

      string tmp = ivl_signal_basename(sig);
      for (ivl_scope_t sscope = ivl_signal_scope(sig) ; ivl_scope_parent(sscope) ; sscope = ivl_scope_parent(sscope)) {
	    tmp = ivl_scope_basename(sscope) + string("/") + tmp;
      }

      name_ = strdup(tmp.c_str());

      assert(name_index_.size()==0);
      if (ivl_signal_width(sig) > 1) {
	    name_index_.resize(ivl_signal_width(sig));

	    assert(ivl_signal_packed_dimensions(sig) == 1);
	    int msb = ivl_signal_packed_msb(sig,0);
	    int lsb = ivl_signal_packed_lsb(sig,0);
	    int dir = (lsb <= msb)? 1 : -1;

	    int val = lsb;
	    for (unsigned idx = 0 ; idx < ivl_signal_width(sig) ; idx += 1) {

		  char buf[64];
		  snprintf(buf, sizeof buf, "[%d]", val);
		  name_index_[idx] = strdup(buf);
		  val += dir;
	    }
      }
}

/*
 * Given that there is not an explicit binding to a signal for naming,
 * search for a signal and use that signal to derive the name of this
 * nexus.
 */
void blif_nex_data_t::select_name_(void)
{
      for (unsigned idx = 0 ; idx < ivl_nexus_ptrs(nex_) ; idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex_, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0)
		  continue;

	    make_name_from_sig_(sig);
	    break;
      }

      assert(name_);
}

void blif_nex_data_t::set_name(ivl_signal_t sig)
{
      assert(name_ == 0);
      assert(ivl_signal_nex(sig,0) == nex_);

      make_name_from_sig_(sig);
}

const char* blif_nex_data_t::get_name(void)
{
      if (name_==0) select_name_();
      return name_;
}

const char* blif_nex_data_t::get_name_index(unsigned bit)
{
      if (name_==0) select_name_();

      if (name_index_.size()==0) {
	    assert(bit == 0);
	    return "";
      }

      assert(bit < name_index_.size());
      assert(name_index_[bit]);
      return name_index_[bit];
}

/*
 * Get the width from any signal that is attached to the nexus.
 */
size_t blif_nex_data_t::get_width(void)
{
      if (name_==0) select_name_();

	// Special case: If the nexus width is 1 bit, then there is no
	// need for index_name maps, so the name_index_ will be empty.
      if (name_index_.size()==0)
	    return 1;

      return name_index_.size();
}
