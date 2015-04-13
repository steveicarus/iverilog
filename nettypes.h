#ifndef IVL_nettypes_H
#define IVL_nettypes_H
/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
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

# include  "ivl_target.h"
# include  <list>
# include  <vector>
# include  <climits>
# include  <ostream>
# include  <cassert>

class netrange_t;

/*
 * This is a fully abstract type that is a type that can be attached
 * to a NetNet object.
 */
class ivl_type_s {
    public:
      virtual ~ivl_type_s() =0;
      virtual bool packed(void) const;
      virtual long packed_width(void) const;
      virtual std::vector<netrange_t> slice_dimensions() const;

	// Some types have a base variable type. This is the bit type
	// for packed data types, or IVL_VT_DARRAY or IVL_VT_CLASS for
	// those specific types.
      virtual ivl_variable_type_t base_type() const;
      virtual bool get_signed() const;

	// Return true if "that" type is compatible with this
	// type. Compatible means the types are essentially the same.
      bool type_compatible(ivl_type_t that) const;

      virtual std::ostream& debug_dump(std::ostream&) const;

    private:
	// The "type_compatible" method uses this virtual method to
	// invoke type-specific tests of compatibility. This should
	// only be called by the type_compatible method above.
      virtual bool test_compatibility(ivl_type_t that) const;
};

/*
 * There are a couple types of array types. This class represents the
 * common bits of array types.
 */
class netarray_t : public ivl_type_s {

    public:
      inline explicit netarray_t(ivl_type_t etype) : element_type_(etype) { }
      ~netarray_t();

    public:
	// Some virtual methods have a common implementation for arrays.

	// The base_type() for arrays is the base_Typeof the element.
      ivl_variable_type_t base_type() const;

    public:
      inline ivl_type_t element_type() const { return element_type_; }

    private:
      ivl_type_t element_type_;
};

inline static std::ostream& operator << (std::ostream&out, const ivl_type_s&obj)
{
      return obj.debug_dump(out);
}

class netrange_t {

    public:
	// Create an undefined range. An undefined range is a range
	// used to declare dynamic arrays, etc.
      inline netrange_t() : msb_(LONG_MAX), lsb_(LONG_MAX) { }
	// Create a properly defined netrange
      inline netrange_t(long m, long l) : msb_(m), lsb_(l) { }
	// Copy constructor.
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

      inline bool operator == (const netrange_t&that) const
      { if (msb_ != that.msb_) return false;
	if (lsb_ != that.lsb_) return false;
	return true;
      }

      inline bool operator != (const netrange_t&that) const
      { if (msb_ != that.msb_) return true;
	if (lsb_ != that.lsb_) return true;
	return false;
      }

    private:
      long msb_;
      long lsb_;
};

extern std::ostream&operator << (std::ostream&out, const std::list<netrange_t>&rlist);
extern std::ostream&operator << (std::ostream&out, const std::vector<netrange_t>&rlist);

extern unsigned long netrange_width(const std::vector<netrange_t>&dims);

/*
 * Take as input a list of packed dimensions and a list of prefix
 * indices, and calculate the offset/width of the resulting slice into
 * the packed array.
 */
extern bool prefix_to_slice(const std::vector<netrange_t>&dims,
			    const std::list<long>&prefix, long sb,
			    long&loff, unsigned long&lwid);

#endif /* IVL_nettypes_H */
