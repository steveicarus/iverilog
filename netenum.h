#ifndef __netenum_H
#define __netenum_H
/*
 * Copyright (c) 2010-2011 Stephen Williams (steve@icarus.com)
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

# include  "ivl_target.h"
# include  "nettypes.h"
# include  "verinum.h"
# include  "StringHeap.h"
# include  "LineInfo.h"
# include  <vector>
# include  <map>

class NetScope;

class netenum_t : public LineInfo, public nettype_base_t {

    public:
      explicit netenum_t(ivl_variable_type_t base_type, bool signed_flag,
			 long msb, long lsb, size_t name_count);
      ~netenum_t();

      ivl_variable_type_t base_type() const;
      unsigned base_width() const;
      bool has_sign() const;

	// The size() is the number of enumeration literals.
      size_t size() const;

	// Insert the name (and value) at the specific place in the
	// enumeration. This must be done exactly once for each
	// enumeration value.
      bool insert_name(size_t idx, perm_string name, const verinum&val);

      typedef std::map<perm_string,verinum>::const_iterator iterator;
      iterator find_name(perm_string name) const;
      iterator end_name() const;
      perm_string find_value(const verinum&val) const;

	// These methods roughly match the .first() and .last() methods.
      iterator first_name() const;
      iterator last_name() const;

      perm_string name_at(size_t idx) const;
      perm_string bits_at(size_t idx);

    private:
      ivl_variable_type_t base_type_;
      bool signed_flag_;
      long msb_, lsb_;

      std::map<perm_string,verinum> names_map_;
      std::vector<perm_string> names_;
      std::vector<perm_string> bits_;
};

inline ivl_variable_type_t netenum_t::base_type() const
{ return base_type_; }

inline unsigned netenum_t::base_width() const
{
      if (msb_ >= lsb_)
	    return msb_ - lsb_ + 1;
      else
	    return lsb_ - msb_ + 1;
}

inline size_t netenum_t::size() const { return names_.size(); }

inline bool netenum_t::has_sign() const { return signed_flag_; }

#endif
