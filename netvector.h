#ifndef __netvector_H
#define __netvector_H
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
# include  "ivl_target.h"
# include  <vector>
# include  <ostream>

class netvector_t : public nettype_base_t {

    public:
      explicit netvector_t(const std::list<netrange_t>&packed,
			   ivl_variable_type_t type);

	// special case: there is a single packed dimension and we
	// know it in the form [<msb>:<lsb>]. This step saves me
	// creating a netrange_t for this single item.
      explicit netvector_t(ivl_variable_type_t type, long msb, long lsb);

	// Special case: scaler object--no packed dimenions at all.
      explicit netvector_t(ivl_variable_type_t type);

      ~netvector_t();

      ivl_variable_type_t base_type() const;
      const std::list<netrange_t>&packed_dims() const;

      long packed_width() const;

      std::ostream& debug_dump(std::ostream&) const;

    private:
      std::list<netrange_t> packed_dims_;
      ivl_variable_type_t type_;

};

inline netvector_t::netvector_t(const std::list<netrange_t>&packed,
				ivl_variable_type_t type)
: packed_dims_(packed), type_(type)
{
}

inline const std::list<netrange_t>& netvector_t::packed_dims() const
{
      return packed_dims_;
}

inline static std::ostream& operator << (std::ostream&out, const netvector_t&obj)
{
      return obj.debug_dump(out);
}

#endif
