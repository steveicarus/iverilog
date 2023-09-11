#ifndef IVL_netstruct_H
#define IVL_netstruct_H
/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
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

# include  "LineInfo.h"
# include  <vector>
# include  "ivl_target.h"
# include  "nettypes.h"

class Design;

class netstruct_t : public LineInfo, public ivl_type_s {

    public:
      struct member_t {
	    perm_string name;
	    ivl_type_t net_type;
	    inline ivl_variable_type_t data_type() const
	    { return net_type->base_type(); };
	    bool get_signed() const
	    { return net_type->get_signed(); }
      };

    public:
      netstruct_t();
      ~netstruct_t();

	// If this is a union (instead of struct) then this flag is
	// set. We handle union and struct together because they are
	// so similar.
      void union_flag(bool);
      bool union_flag(void) const;

      void packed(bool flag);
      bool packed(void) const;

        // When the struct is accessed as a primary it can be signed or unsigned
      void set_signed(bool flag) { signed_ = flag; }
      bool get_signed(void) const { return signed_; }

	// Append a new member to the struct/union. This must be done
	// after the union_flag and packed settings are set. This
	// function does error checking, and the "des" argument is
	// only present so that it can set error flags.
      void append_member(Design*des, const member_t&);

	// Given the name of a member, return a pointer to the member
	// description, and set the off value to be the offset into
	// the packed value where the member begins.
      const struct member_t* packed_member(perm_string name, unsigned long&off) const;
      const std::vector<member_t>& members() const { return members_; }

	// Return the width (in bits) of the packed record, or -1 if
	// the record is not packed.
      long packed_width() const;
      netranges_t slice_dimensions() const;

	// Return the base type of the packed record, or
	// IVL_VT_NO_TYPE if the record is not packed.
      ivl_variable_type_t base_type() const;

    private:
      bool test_compatibility(ivl_type_t that) const;
      bool test_equivalence(ivl_type_t that) const;

    private:
      bool union_;
      bool packed_;
      bool signed_;
      std::vector<member_t>members_;
};

inline bool netstruct_t::union_flag(void) const { return union_; }
inline bool netstruct_t::packed(void) const { return packed_; }

#endif /* IVL_netstruct_H */
