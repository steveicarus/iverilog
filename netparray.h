#ifndef __netarray_H
#define __netarray_H
/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012 / Stephen Williams (steve@icarus.com)
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

# include  "nettypes.h"
# include  <vector>

/*
 * Packed arrays.
 */
class netparray_t : public ivl_type_s {

    public:
      explicit netparray_t(const std::vector<netrange_t>&packed,
			   ivl_type_t etype);
      ~netparray_t();

    public:
	// Virtual methods from the ivl_type_s type...
      long packed_width(void) const;
      std::vector<netrange_t> slice_dimensions() const;
      ivl_variable_type_t base_type() const;

    public:
      inline const ivl_type_s* element_type() const { return element_type_; }

      inline const std::vector<netrange_t>& packed_dimensions() const
      { return packed_dims_; }

    private:
      std::vector<netrange_t> packed_dims_;
      ivl_type_t element_type_;

};

inline netparray_t::netparray_t(const std::vector<netrange_t>&packed,
				ivl_type_t etype)
: packed_dims_(packed), element_type_(etype)
{
}

inline netparray_t::~netparray_t()
{
}

#endif
