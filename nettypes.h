#ifndef __nettypes_H
#define __nettypes_H
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include <list>
# include <climits>
# include <cassert>

/*
 * This is a fully abstract type that is a type that can be attached
 * to a NetNet object.
 */
class nettype_base_t {
    public:
      virtual ~nettype_base_t() =0;
};

class netrange_t {

    public:
	// Create an undefined range. An undefined range is a range
	// used to declare dynamic arrays, etc.
      inline netrange_t() : msb_(LONG_MAX), lsb_(LONG_MAX) { }
	// Create a properly defined netrange
      inline netrange_t(long m, long l) : msb_(m), lsb_(l) { }
	// Copy constructure.
      inline netrange_t(const netrange_t&that)
      : msb_(that.msb_), lsb_(that.lsb_) { }

      inline netrange_t& operator = (const netrange_t&that)
      { msb_ = that.msb_; lsb_ = that.lsb_; return *this; }

      inline bool defined() const
      { return msb_!=LONG_MAX || lsb_!= LONG_MAX; }

      inline unsigned long width()const
      { if (!defined()) return 0;
	else if (msb_ >= lsb_) return msb_-lsb_+1;
	else return lsb_-msb_+1;
      }

      inline long get_msb() const { assert(defined()); return msb_; }
      inline long get_lsb() const { assert(defined()); return lsb_; }

    private:
      long msb_;
      long lsb_;
};

extern unsigned long netrange_width(const std::list<netrange_t>&dims);

/*
 * Take as input a list of packed dimensions and a list of prefix
 * indices, and calculate the offset/width of the resulting slice into
 * the packed array.
 */
extern bool prefix_to_slice(const std::list<netrange_t>&dims,
			    const std::list<long>&prefix, long sb,
			    long&loff, unsigned long&lwid);

#endif
