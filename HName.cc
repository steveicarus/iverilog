/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "HName.h"
# include  <iostream>
# include  <cstring>
# include  <cstdlib>
# include  <climits>


hname_t::hname_t()
{
      number_ = INT_MIN;
}

hname_t::hname_t(perm_string text)
{
      name_ = text;
      number_ = INT_MIN;
}

hname_t::hname_t(perm_string text, int num)
{
      name_ = text;
      number_ = num;
}

hname_t::hname_t(const hname_t&that)
{
      name_ = that.name_;
      number_ = that.number_;
}

hname_t& hname_t::operator = (const hname_t&that)
{
      name_ = that.name_;
      number_ = that.number_;
      return *this;
}

hname_t::~hname_t()
{
}

perm_string hname_t::peek_name(void) const
{
      return name_;
}

bool hname_t::has_number() const
{
      return number_ != INT_MIN;
}

int hname_t::peek_number() const
{
      return number_;
}

bool operator < (const hname_t&l, const hname_t&r)
{
      int cmp = strcmp(l.peek_name(), r.peek_name());
      if (cmp < 0) return true;
      if (cmp > 0) return false;
      if (l.has_number() && r.has_number())
	    return l.peek_number() < r.peek_number();
      else
	    return false;
}

bool operator == (const hname_t&l, const hname_t&r)
{
      if (l.peek_name() == r.peek_name()) {
	    if (l.has_number() && r.has_number())
		  return l.peek_number() == r.peek_number();
	    else
		  return true;
      }

      return false;
}

bool operator != (const hname_t&l, const hname_t&r)
{ return ! (l==r); }

ostream& operator<< (ostream&out, const hname_t&that)
{
      if (that.peek_name() == 0) {
	    out << "";
	    return out;
      }

      out << that.peek_name();
      if (that.has_number())
	    out << "[" << that.peek_number() << "]";

      return out;
}
