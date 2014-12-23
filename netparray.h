#ifndef IVL_netarray_H
#define IVL_netarray_H
/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
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
 * Arrays with static dimensions (packed and unpacked) share this
 * common base type.
 */
class netsarray_t : public netarray_t {

    public:
      explicit netsarray_t(const std::vector<netrange_t>&packed,
			   ivl_type_t etype);
      ~netsarray_t();

    public:
	// Virtual methods from the ivl_type_s type...

    public:
      inline const std::vector<netrange_t>& static_dimensions() const
      { return dims_; }

    private:
      std::vector<netrange_t> dims_;

};

inline netsarray_t::netsarray_t(const std::vector<netrange_t>&pd,
				ivl_type_t etype)
: netarray_t(etype), dims_(pd)
{
}

/*
 * Packed arrays.
 */
class netparray_t : public netsarray_t {

    public:
      explicit netparray_t(const std::vector<netrange_t>&packed,
			   ivl_type_t etype);
      ~netparray_t();

    public:
	// Virtual methods from the ivl_type_s type...
      bool packed(void) const;
      long packed_width(void) const;
      std::vector<netrange_t> slice_dimensions() const;

};

inline netparray_t::netparray_t(const std::vector<netrange_t>&pd,
				ivl_type_t etype)
: netsarray_t(pd, etype)
{
}

/*
 * Unpacked arrays are very similar, but lack packed slices.
 */
class netuarray_t : public netsarray_t {

    public:
      explicit netuarray_t(const std::vector<netrange_t>&packed,
			   ivl_type_t etype);
      ~netuarray_t();

    public:
	// Virtual methods from the ivl_type_s type...
      std::vector<netrange_t> slice_dimensions() const;
};

inline netuarray_t::netuarray_t(const std::vector<netrange_t>&pd,
				ivl_type_t etype)
: netsarray_t(pd, etype)
{
}

#endif /* IVL_netarray_H */
