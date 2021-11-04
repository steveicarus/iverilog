#ifndef IVL_verinum_H
#define IVL_verinum_H
/*
 * Copyright (c) 1998-2021 Stephen Williams (steve@icarus.com)
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

# include  <string>

# include  "config.h"
#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
# include  <iostream>
#endif

/*
 * Numbers in Verilog are multibit strings, where each bit has 4
 * possible values: 0, 1, x or z. The verinum number is store in
 * little-endian format. This means that if the long value is 2b'10,
 * get(0) is 0 and get(1) is 1.
 */
class verinum {

    public:
      enum V { V0 = 0, V1, Vx, Vz };

      verinum();
      explicit verinum(const std::string&str);
      verinum(const V*v, unsigned nbits, bool has_len =true);
      explicit verinum(V, unsigned nbits =1, bool has_len =true);
      verinum(uint64_t val, unsigned bits);
      verinum(double val, bool);
      verinum(const verinum&);

	// Create a signed number, with an unspecified number of bits.
      explicit verinum(int64_t val);

	// Copy only the specified number of bits from the
	// source. Also mark this number as has_len.
      verinum(const verinum&, unsigned bits);

      ~verinum();
      verinum& operator= (const verinum&);

	// Number of stored bits in this number.
      unsigned len() const { return nbits_; }

	// A number "has a length" if the length was specified fixed
	// in some way.
      bool has_len(bool flag) { has_len_ = flag; return has_len_; }
      bool has_len() const { return has_len_; }

      bool has_sign(bool flag) { has_sign_ = flag; return has_sign_; }
      bool has_sign() const { return has_sign_; }

        // A number "is single" if it comes from a SystemVerilog 'N bit vector
      bool is_single(bool flag) { is_single_ = flag; return is_single_; }
      bool is_single() const { return is_single_; }

	// A number is "defined" if there are no x or z bits in its value.
      bool is_defined() const;
      bool is_zero() const;
      bool is_negative() const;

	// A number is "a string" if its value came directly from
	// an ASCII description instead of a number value.
      bool is_string() const { return string_flag_; }

	// Comparison for use in sorting algorithms.
      bool is_before(const verinum&that) const;

	// Number of significant bits in this number.
      unsigned significant_bits() const;

	// Convert 4-state to 2-state
      void cast_to_int2();

	// Individual bits can be accessed with the get and set
	// methods.
      V get(unsigned idx) const;
      V set(unsigned idx, V val);
      void set(unsigned idx, const verinum&val);

      V operator[] (unsigned idx) const { return get(idx); }

	// Return the value as a native unsigned integer. If the value is
	// larger than can be represented by the returned type, return
	// the maximum value of that type. If the value has any x or z
	// bits or has zero width, return the value 0.
      uint64_t as_ulong64() const;
      unsigned as_unsigned() const;
      unsigned long as_ulong() const;

      signed long   as_long() const;
      double as_double() const;
      std::string as_string() const;
    private:
      void signed_trim();

    private:
      V* bits_;
      unsigned nbits_;
      bool has_len_;
      bool has_sign_;
      bool is_single_;

	// These are some convenience flags that help us do a better
	// job of pretty-printing values.
      bool string_flag_;
};

/*
 * This returns the sign bit of the verinum value. If the value is
 * unsigned, then return an implicit sign bit of 0. Otherwise, return
 * the high bit.
 */
inline verinum::V sign_bit(const verinum&val)
{
      if (val.has_sign())
	    return val.get(val.len()-1);
      else
	    return verinum::V0;
}

/* Return a verinum that has the same value as the input, but is at
   least as wide as the requested width. This may involve sign
   extension, if the value is signed. */
extern verinum pad_to_width(const verinum&, unsigned width);

/* Return a verinum that has the same value as the input, but is
   exactly the requested width. This may involve sign extension,
   if the value is signed. The returned verinum will have fixed
   width. */
extern verinum cast_to_width(const verinum&, unsigned width);

/* Return a verinum that is minimal. That is, it has only the length
   needed to accurately represent the contained value, signed or not. */
extern verinum trim_vnum(const verinum&);

extern std::ostream& operator<< (std::ostream&, const verinum&);
extern std::ostream& operator<< (std::ostream&, verinum::V);

inline verinum::V bit4_z2x(verinum::V bit)
{ return bit<2? bit : verinum::Vx; /* Relies on V0 and V1 being <2 */}

extern verinum::V operator ~ (verinum::V l);
extern verinum::V operator | (verinum::V l, verinum::V r);
extern verinum::V operator & (verinum::V l, verinum::V r);
extern verinum::V operator ^ (verinum::V l, verinum::V r);

extern verinum::V operator == (const verinum&left, const verinum&right);
extern verinum::V operator <= (const verinum&left, const verinum&right);
extern verinum::V operator <  (const verinum&left, const verinum&right);

inline verinum::V operator > (const verinum&left, const verinum&right)
{ return right < left; }

inline verinum::V operator >= (const verinum&left, const verinum&right)
{ return right <= left; }

inline verinum::V operator != (const verinum&left, const verinum&right)
{ return (left == right)? verinum::V0 : verinum::V1; }


/* These are arithmetic operators. If any operand is unsized, they
   generally work to produce results that do not overflow. That means
   the result may expand or contract to hold the bits needed to hold
   the operation results accurately. It is up to the caller to truncate
   or pad if a specific width is expected. If all operands are sized,
   the normal Verilog rules for result size are used. */
extern verinum operator - (const verinum&right);
extern verinum operator + (const verinum&left, const verinum&right);
extern verinum operator - (const verinum&left, const verinum&right);
extern verinum operator * (const verinum&left, const verinum&right);
extern verinum operator / (const verinum&left, const verinum&right);
extern verinum operator % (const verinum&left, const verinum&right);

extern verinum pow(const verinum&left, const verinum&right);

extern verinum operator<< (const verinum&left, unsigned shift);
extern verinum operator>> (const verinum&left, unsigned shift);

extern verinum concat(const verinum&left, const verinum&right);

/* Bitwise not returns the ones complement. */
extern verinum operator ~ (const verinum&left);

#endif /* IVL_verinum_H */
