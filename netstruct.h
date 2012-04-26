#ifndef __netstruct_H
#define __netstruct_H
/*
 * Copyright (c) 2011-2012 Stephen Williams (steve@icarus.com)
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

# include  "LineInfo.h"
# include <vector>
# include  "ivl_target.h"

class netstruct_t : public LineInfo {

    public:
      struct member_t {
	    perm_string name;
	    ivl_variable_type_t type;
	    long msb;
	    long lsb;
	    long width() const;
	    ivl_variable_type_t data_type() const { return type; };
	      // We need to keep the individual element sign information.
	    bool get_signed() const { return false; };
      };

    public:
      netstruct_t();
      ~netstruct_t();

      void packed(bool flag);
      bool packed(void) const;

      void append_member(const member_t&);

	// Given the name of a member, return a pointer to the member
	// description, and set the off value to be the offset into
	// the packed value where the member begins.
      const struct member_t* packed_member(perm_string name, unsigned long&off) const;

	// Return the width (in bits) of the packed record, or -1 if
	// the record is not packed.
      long packed_width() const;

    private:
      bool packed_;
      std::vector<member_t>members_;
};

inline bool netstruct_t::packed(void) const { return packed_; }

inline long netstruct_t::member_t::width() const
{
      if (msb >= lsb)
	    return msb - lsb + 1;
      else
	    return lsb - msb + 1;
}

#endif
