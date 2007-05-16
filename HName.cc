/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: HName.cc,v 1.7 2007/05/16 19:12:33 steve Exp $"
#endif

# include  "config.h"
# include  "HName.h"
# include  <iostream>
# include  <string.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

inline perm_string& hname_t::item_ref1_()
{
      return *reinterpret_cast<perm_string*>(item_);
}

inline const perm_string& hname_t::item_ref1_() const
{
      return *reinterpret_cast<const perm_string*>(item_);
}


hname_t::hname_t()
{
      count_ = 0;
}

hname_t::hname_t(perm_string text)
{
      new (item_) perm_string(text);
      count_ = 1;
}

hname_t::hname_t(const hname_t&that)
{
      count_ = that.count_;
      switch (count_) {
	  case 0:
	    break;
	  case 1:
	    new(item_) perm_string (that.item_ref1_());
	    break;
	  default:
	    array_ = new perm_string[count_];
	    for (unsigned idx = 0 ;  idx < count_ ;  idx += 1)
		  array_[idx] = that.array_[idx];
	    break;
      }
}

hname_t::~hname_t()
{
      switch (count_) {
	  case 0:
	    break;
	  case 1:
	    item_ref1_().~perm_string();
	    break;
	  default:
	    delete[]array_;
	    break;
      }
}

unsigned hname_t::component_count() const
{
      return count_;
}

void hname_t::append(perm_string text)
{
      perm_string*tmp;

      switch (count_) {
	  case 0:
	    count_ = 1;
	    new (item_) perm_string(text);
	    break;
	  case 1:
	    count_ = 2;
	    tmp = new perm_string[2];
	    tmp[0] = item_ref1_();
	    tmp[1] = text;
	    item_ref1_().~perm_string();
	    array_ = tmp;
	    break;
	  default:
	    tmp = new perm_string[count_+1];
	    for (unsigned idx = 0 ;  idx < count_ ;  idx += 1)
		  tmp[idx] = array_[idx];
	    delete[]array_;
	    array_ = tmp;
	    array_[count_] = text;
	    count_ += 1;
      }
}

void hname_t::prepend(perm_string text)
{
      perm_string*tmp;

      switch (count_) {
	  case 0:
	    count_ = 1;
	    new (item_) perm_string(text);
	    break;
	  case 1:
	    count_ = 2;
	    tmp = new perm_string[2];
	    tmp[0] = text;
	    tmp[1] = item_ref1_();
	    item_ref1_().~perm_string();
	    array_ = tmp;
	    break;
	  default:
	    tmp = new perm_string[count_+1];
	    tmp[0] = text;
	    for (unsigned idx = 0 ;  idx < count_ ;  idx += 1)
		  tmp[idx+1] = array_[idx];
	    delete[]array_;
	    array_ = tmp;
	    count_ += 1;
      }
}

perm_string hname_t::remove_tail_name()
{
      if (count_ == 0)
	    return perm_string();

      if (count_ == 1) {
	    perm_string tmp = item_ref1_();
	    count_ = 0;
	    item_ref1_().~perm_string();
	    return tmp;
      }

      if (count_ == 2) {
	    perm_string tmp1 = array_[0];
	    perm_string tmp2 = array_[1];
	    delete[]array_;
	    count_ = 1;
	    new (item_) perm_string(tmp1);
	    return tmp2;
      }

      perm_string tmpo = array_[count_-1];
      perm_string*tmpa = new perm_string[count_-1];
      for (unsigned idx = 0 ;  idx < count_-1 ;  idx += 1)
	    tmpa[idx] = array_[idx];

      delete[]array_;
      array_ = tmpa;
      count_ -= 1;
      return tmpo;
}

perm_string hname_t::peek_name(unsigned idx) const
{
      if (idx >= count_)
	    return perm_string();

      if (count_ == 1)
	    return item_ref1_();

      return array_[idx];
}

perm_string hname_t::peek_tail_name() const
{
      switch (count_) {
	  case 0:
	    return perm_string();
	  case 1:
	    return item_ref1_();
	  default:
	    return array_[count_-1];
      }
}

bool operator < (const hname_t&l, const hname_t&r)
{
      unsigned idx = 0;
      const char*lc = l.peek_name(idx);
      const char*rc = r.peek_name(idx);

      while (lc && rc) {
	    int cmp = strcmp(lc, rc);
	    if (cmp < 0)
		  return true;
	    if (cmp > 0)
		  return false;
	    idx += 1;
	    lc = l.peek_name(idx);
	    rc = r.peek_name(idx);
      }

      if (lc && !rc)
	    return false;
      if (rc && !lc)
	    return true;

	// Must be ==
      return false;
}

bool operator == (const hname_t&l, const hname_t&r)
{
      unsigned idx = 0;
      const char*lc = l.peek_name(idx);
      const char*rc = r.peek_name(idx);

      while (lc && rc) {
	    int cmp = strcmp(lc, rc);
	    if (cmp != 0)
		  return false;
	    idx += 1;
	    lc = l.peek_name(idx);
	    rc = r.peek_name(idx);
      }

      if (lc || rc)
	    return false;

	// Must be ==
      return true;
}

ostream& operator<< (ostream&out, const hname_t&that)
{
      switch (that.count_) {
	  case 0:
	    out << "";
	    return out;
	  case 1:
	    out << that.item_;
	    return out;

	  default:
	    out << that.array_[0];
	    for (unsigned idx = 1 ;  idx < that.count_ ;  idx += 1)
		  out << "." << that.array_[idx];

	    return out;
      }
}

/*
 * $Log: HName.cc,v $
 * Revision 1.7  2007/05/16 19:12:33  steve
 *  Fix hname_t use of space for 1 perm_string.
 *
 * Revision 1.6  2007/04/26 03:06:21  steve
 *  Rework hname_t to use perm_strings.
 *
 * Revision 1.5  2002/11/02 03:27:52  steve
 *  Allow named events to be referenced by
 *  hierarchical names.
 *
 * Revision 1.4  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2002/01/05 04:36:06  steve
 *  include malloc.h only when available.
 *
 * Revision 1.2  2001/12/18 04:52:45  steve
 *  Include config.h for namespace declaration.
 *
 * Revision 1.1  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 */

