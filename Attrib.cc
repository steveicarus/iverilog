/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: Attrib.cc,v 1.2 2001/07/25 03:10:48 steve Exp $"
#endif

# include "config.h"

# include  "Attrib.h"
# include  <assert.h>

Attrib::Attrib()
{
      nlist_ = 0;
      list_ = 0;
}

Attrib::~Attrib()
{
      delete[] list_;
}

void Attrib::set_attributes(const map<string,string>&attr)
{
      assert(list_ == 0);

      nlist_ = attr.size();
      list_ = new cell_[nlist_];

      map<string,string>::const_iterator idx;
      unsigned jdx;
      for (idx = attr.begin(), jdx = 0 ;  idx != attr.end() ;  idx ++, jdx++) {
	    struct cell_*tmp = list_ + jdx;
	    tmp->key = (*idx).first;
	    tmp->val = (*idx).second;
      }
}

string Attrib::attribute(const string&key) const
{
      for (unsigned idx = 0 ;  idx < nlist_ ;  idx += 1) {

	    if (key == list_[idx].key)
		  return list_[idx].val;
      }

      return "";
}

void Attrib::attribute(const string&key, const string&value)
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

	    string tmp = attribute(that.list_[idx].key);
	    if (tmp != that.list_[idx].val)
		  return false;
      }

      return true;
}

unsigned Attrib::size() const
{
      return nlist_;
}

string Attrib::key(unsigned idx) const
{
      assert(idx < nlist_);
      return list_[idx].key;
}

string Attrib::value(unsigned idx) const
{
      assert(idx < nlist_);
      return list_[idx].val;
}


/*
 * $Log: Attrib.cc,v $
 * Revision 1.2  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2000/12/04 17:37:03  steve
 *  Add Attrib class for holding NetObj attributes.
 *
 */

