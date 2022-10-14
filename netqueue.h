#ifndef IVL__netqueue_H
#define IVL__netqueue_H
/*
 * Copyright (c) 2014-2020 Stephen Williams (steve@icarus.com)
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

# include  "netdarray.h"
# include  "ivl_target.h"

/*
 * A queue type is actually a dynamic array with a few extra
 * methods. This will probably result in a different implementation at
 * run-time, but for the most part this applies during elaboration.
 */
class netqueue_t : public netdarray_t {

    public:
      explicit netqueue_t(ivl_type_t vec, long max_idx);
      ~netqueue_t();

	// This is the "base_type()" virtual method of the
	// nettype_base_t. The ivl_target api expects this to return
	// IVL_VT_QUEUE for queues.
      ivl_variable_type_t base_type() const;

      long max_idx(void) const { return max_idx_; }

      std::ostream& debug_dump(std::ostream&) const;

    private:
      long max_idx_;
};

#endif
