#ifndef __netdarray_H
#define __netdarray_H
/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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
# include  "netvector.h"
# include  "ivl_target.h"
# include  <list>

class netvector_t;

class netdarray_t : public ivl_type_s {

    public:
      explicit netdarray_t(netvector_t*vec);
      ~netdarray_t();

	// This is the "base_type()" virtual method of the
	// nettype_base_t. The ivl_target api expects this to return
	// IVL_VT_DARRAY for dynamic arrays?
      ivl_variable_type_t base_type() const;

	// The ivl_target.h API uses this method to get the type of
	// the element of the array.
      inline const ivl_type_s* element_type() const { return elem_type_; }

	// This is the base_type() of the element of the array.
      inline ivl_variable_type_t element_base_type() const { return elem_type_->base_type(); }
      inline unsigned long vector_width(void) const { return elem_type_->packed_width(); }

      std::ostream& debug_dump(std::ostream&) const;

    private:
      netvector_t*elem_type_;
};

#endif
