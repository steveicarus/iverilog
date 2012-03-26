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

class netrange_t {

    public:
      inline netrange_t() : msb(0), lsb(0) { }
      inline netrange_t(long m, long l) : msb(m), lsb(l) { }

      inline netrange_t(const netrange_t&that)
      : msb(that.msb), lsb(that.lsb) { }

      inline netrange_t& operator = (const netrange_t&that)
      { msb = that.msb; lsb = that.lsb; return *this; }

    public:
      long msb;
      long lsb;

      inline unsigned long width()const
      { if (msb >= lsb) return msb-lsb+1; else return lsb-msb+1; }
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
