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
#ifdef HAVE_CVS_IDENT
#ident "$Id: Attrib.cc,v 1.6 2004/02/20 18:53:33 steve Exp $"
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


/*
 * $Log: Attrib.cc,v $
 * Revision 1.6  2004/02/20 18:53:33  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.5  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2002/05/26 01:39:02  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 *
 * Revision 1.3  2002/05/23 03:08:50  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.2  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2000/12/04 17:37:03  steve
 *  Add Attrib class for holding NetObj attributes.
 *
 */

