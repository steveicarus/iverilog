#ifndef IVL_HName_H
#define IVL_HName_H
/*
 * Copyright (c) 2001-2021 Stephen Williams (steve@icarus.com)
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

# include  <iostream>
# include  <list>
# include  <vector>
# include  "StringHeap.h"

# include  <cassert>

/*
 * This class represents a component of a Verilog hierarchical name. A
 * hierarchical component contains a name string (represented here
 * with a perm_string) and an optional signed number. This signed
 * number is used if the scope is part of an array, for example an
 * array of module instances or a loop generated scope.
 */

class hname_t {

      friend std::ostream& operator<< (std::ostream&out, const hname_t&that);

    public:
      hname_t ();
      explicit hname_t (perm_string text);
      explicit hname_t (perm_string text, int num);
      explicit hname_t (perm_string text, const std::vector<int>&nums);
      hname_t (const hname_t&that);
      ~hname_t();

      hname_t& operator= (const hname_t&);

      bool operator == (const hname_t&that) const;
      bool operator <  (const hname_t&that) const;

	// Return the string part of the hname_t.
      perm_string peek_name(void) const;

      size_t has_numbers() const;
      int peek_number(size_t idx) const;
      const std::vector<int>&peek_numbers() const;

    private:
      perm_string name_;
	// If this vector has size, then the numbers all together make
	// up part of the hierarchical name.
      std::vector<int> number_;

    private: // not implemented
};

inline hname_t::~hname_t()
{
}

inline perm_string hname_t::peek_name(void) const
{
      return name_;
}

inline int hname_t::peek_number(size_t idx) const
{
      assert(number_.size() > idx);
      return number_[idx];
}

inline const std::vector<int>& hname_t::peek_numbers(void) const
{
      return number_;
}

inline size_t hname_t::has_numbers() const
{
      return number_.size();
}

extern std::ostream& operator<< (std::ostream&, const hname_t&);

inline bool operator != (const hname_t&l, const hname_t&r)
{ return ! (l == r); }

inline std::ostream& operator<< (std::ostream&out, const std::list<hname_t>&ll)
{
      std::list<hname_t>::const_iterator cur = ll.begin();
      out << *cur;
      ++ cur;
      while (cur != ll.end()) {
	    out << "." << *cur;
	    ++ cur;
      }
      return out;
}

#endif /* IVL_HName_H */
