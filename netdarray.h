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

class netdarray_t : public nettype_base_t {

    public:
      explicit netdarray_t(netvector_t*vec);
      ~netdarray_t();

      inline ivl_variable_type_t data_type() const { return elem_type_->base_type(); }
      inline unsigned long vector_width(void) const { return elem_type_->packed_width(); }

    private:
      netvector_t*elem_type_;
};

#endif
