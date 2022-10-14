#ifndef IVL_netdarray_H
#define IVL_netdarray_H
/*
 * Copyright (c) 2012-2015 Stephen Williams (steve@icarus.com)
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

# include  "nettypes.h"
# include  "ivl_target.h"

class netdarray_t : public netarray_t {

    public:
      explicit netdarray_t(ivl_type_t vec);
      ~netdarray_t();

	// This is the "base_type()" virtual method of the
	// nettype_base_t. The ivl_target api expects this to return
	// IVL_VT_DARRAY for dynamic arrays?
      ivl_variable_type_t base_type() const;

	// A dynamic array may have a type that is signed.
      inline bool get_signed() const { return element_type()->get_signed(); }

	// This is the base_type() of the element of the array. We
	// need this in some cases in order to get the base type of
	// the element, and not the IVL_VT_DARRAY of the array itself.
      inline ivl_variable_type_t element_base_type() const { return element_type()->base_type(); }

	// This is a convenience function for getting the width of an
	// element. Strictly speaking it's not necessary.
      inline unsigned long element_width(void) const { return element_type()->packed_width(); }

      std::ostream& debug_dump(std::ostream&) const;

    private:
      bool test_compatibility(ivl_type_t that) const;
      bool test_equivalence(ivl_type_t that) const;
};

#endif /* IVL_netdarray_H */
