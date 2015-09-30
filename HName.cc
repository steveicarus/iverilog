/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "HName.h"
# include  <iostream>
# include  <cstring>
# include  <cstdlib>

using namespace std;

hname_t::hname_t()
{
}

hname_t::hname_t(perm_string text)
: name_(text)
{
}

hname_t::hname_t(perm_string text, int num)
: name_(text), number_(1)
{
      number_[0] = num;
}

hname_t::hname_t(perm_string text, const vector<int>&nums)
: name_(text), number_(nums)
{
}

hname_t::hname_t(const hname_t&that)
: name_(that.name_), number_(that.number_)
{
}

hname_t& hname_t::operator = (const hname_t&that)
{
      name_ = that.name_;
      number_ = that.number_;
      return *this;
}

bool hname_t::operator < (const hname_t&r) const
{
      int cmp = strcmp(name_, r.name_);
      if (cmp < 0) return true;
      if (cmp > 0) return false;

	// The text parts are equal, so compare then number
	// parts. Finish as soon as we find one to be less or more
	// than the other.
      size_t idx = 0;
      while (number_.size() > idx || r.number_.size() > idx) {

	      // Ran out of l numbers, so less.
	    if (number_.size() <= idx)
		  return true;

	      // Ran out of r numbers, so greater.
	    if (r.number_.size() <= idx)
		  return false;

	    if (number_[idx] < r.number_[idx])
		  return true;

	    if (number_[idx] > r.number_[idx])
		  return false;

	    idx += 1;
      }

	// Fall-through means that we are equal, including all the
	// number parts, so not less.
      return false;
}

bool hname_t::operator == (const hname_t&r) const
{
      if (name_ == r.name_) {
	    if (number_.size() != r.number_.size())
		  return false;

	    for (size_t idx = 0 ; idx < number_.size() ; idx += 1)
		  if (number_[idx] != r.number_[idx]) return false;

	    return true;
      }

      return false;
}

ostream& operator<< (ostream&out, const hname_t&that)
{
      if (that.peek_name() == 0) {
	    out << "";
	    return out;
      }

      out << that.peek_name();
      for (size_t idx = 0 ; idx < that.number_.size() ; idx += 1)
	    out << "[" << that.number_[idx] << "]";

      return out;
}
