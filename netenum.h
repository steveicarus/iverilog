#ifndef IVL_netenum_H
#define IVL_netenum_H
/*
 * Copyright (c) 2010-2014 Stephen Williams (steve@icarus.com)
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

# include  "ivl_target.h"
# include  "nettypes.h"
# include  "verinum.h"
# include  "StringHeap.h"
# include  "LineInfo.h"
# include  <vector>
# include  <map>

class NetScope;

class netenum_t : public LineInfo, public ivl_type_s {

    public:
      explicit netenum_t(ivl_type_t base_type, size_t name_count,
			 bool integer_flag);
      ~netenum_t();

      virtual ivl_variable_type_t base_type() const;
      virtual bool packed() const;
      virtual long packed_width() const;
      netranges_t slice_dimensions() const;
      bool get_signed() const;
      bool get_isint() const;

	// The size() is the number of enumeration literals.
      size_t size() const;

	// Insert the name (and value) at the specific place in the
	// enumeration. This must be done exactly once for each
	// enumeration value.
      bool insert_name(size_t idx, perm_string name, const verinum&val);

	// Indicate that there will be no more names to insert.
      void insert_name_close(void);

      typedef std::map<perm_string,verinum>::const_iterator iterator;
      iterator find_name(perm_string name) const;
      iterator end_name() const;
      perm_string find_value(const verinum&val) const;

	// These methods roughly match the .first() and .last() methods.
      iterator first_name() const;
      iterator last_name() const;

      perm_string name_at(size_t idx) const;
      perm_string bits_at(size_t idx) const;

	// Check if two enumerations have the same definition.
      bool matches(const netenum_t*other) const;

    private:
      ivl_type_t base_type_;
      bool integer_flag_;

      std::map<perm_string,verinum> names_map_;
      std::vector<perm_string> names_;
      std::vector<perm_string> bits_;
};

inline ivl_variable_type_t netenum_t::base_type() const
{ return base_type_->base_type(); }

inline size_t netenum_t::size() const { return names_.size(); }

#endif /* IVL_netenum_H */
