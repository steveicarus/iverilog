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
#ident "$Id: HName.cc,v 1.5 2002/11/02 03:27:52 steve Exp $"
#endif

# include  "config.h"
# include  "HName.h"
# include  <iostream>
# include  <string.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

hname_t::hname_t()
{
      item_ = 0;
      count_ = 0;
}

hname_t::hname_t(const char*text)
{
      item_ = strdup(text);
      count_ = 1;
}

hname_t::hname_t(const hname_t&that)
{
      count_ = that.count_;
      switch (count_) {
	  case 0:
	    item_ = 0;
	    break;
	  case 1:
	    item_ = strdup(that.item_);
	    break;
	  default:
	    array_ = new char*[count_];
	    for (unsigned idx = 0 ;  idx < count_ ;  idx += 1)
		  array_[idx] = strdup(that.array_[idx]);
	    break;
      }
}

hname_t::~hname_t()
{
      switch (count_) {
	  case 0:
	    break;
	  case 1:
	    free(item_);
	    break;
	  default:
	    for (unsigned idx = 0 ;  idx < count_ ;  idx += 1)
		  free(array_[idx]);
	    delete[]array_;
	    break;
      }
}

unsigned hname_t::component_count() const
{
      return count_;
}

void hname_t::append(const char*text)
{
      char**tmp;

      switch (count_) {
	  case 0:
	    count_ = 1;
	    item_ = strdup(text);
	    break;
	  case 1:
	    count_ = 2;
	    tmp = new char*[2];
	    tmp[0] = item_;
	    tmp[1] = strdup(text);
	    array_ = tmp;
	    break;
	  default:
	    tmp = new char*[count_+1];
	    for (unsigned idx = 0 ;  idx < count_ ;  idx += 1)
		  tmp[idx] = array_[idx];
	    delete[]array_;
	    array_ = tmp;
	    array_[count_] = strdup(text);
	    count_ += 1;
      }
}

void hname_t::prepend(const char*text)
{
      char**tmp;

      switch (count_) {
	  case 0:
	    count_ = 1;
	    item_ = strdup(text);
	    break;
	  case 1:
	    count_ = 2;
	    tmp = new char*[2];
	    tmp[0] = strdup(text);
	    tmp[1] = item_;
	    array_ = tmp;
	    break;
	  default:
	    tmp = new char*[count_+1];
	    tmp[0] = strdup(text);
	    for (unsigned idx = 0 ;  idx < count_ ;  idx += 1)
		  tmp[idx+1] = array_[idx];
	    delete[]array_;
	    array_ = tmp;
	    count_ += 1;
      }
}

char* hname_t::remove_tail_name()
{
      if (count_ == 0)
	    return 0;

      if (count_ == 1) {
	    char*tmp = item_;
	    count_ = 0;
	    item_ = 0;
	    return tmp;
      }

      if (count_ == 2) {
	    char*tmp1 = array_[0];
	    char*tmp2 = array_[1];
	    delete[]array_;
	    count_ = 1;
	    item_ = tmp1;
	    return tmp2;
      }

      char*tmpo = array_[count_-1];
      char**tmpa = new char*[count_-1];
      for (unsigned idx = 0 ;  idx < count_-1 ;  idx += 1)
	    tmpa[idx] = array_[idx];

      delete[]array_;
      array_ = tmpa;
      count_ -= 1;
      return tmpo;
}

const char* hname_t::peek_name(unsigned idx) const
{
      if (idx >= count_)
	    return 0;

      if (count_ == 1)
	    return item_;

      return array_[idx];
}

const char* hname_t::peek_tail_name() const
{
      switch (count_) {
	  case 0:
	    return 0;
	  case 1:
	    return item_;
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

