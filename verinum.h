#ifndef __verinum_H
#define __verinum_H
/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: verinum.h,v 1.26.2.4 2007/03/23 20:59:26 steve Exp $"
#endif

# include  <string>

# include  "config.h"
#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
class ostream;
#endif

using namespace std;

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
      verinum(const std::string&);
      verinum(const V*v, unsigned nbits, bool has_len =true);
      verinum(V, unsigned nbits =1, bool has_len =true);
      verinum(unsigned long val, unsigned bits);
      verinum(const verinum&);

	// Create a signed number, with an unspecified number of bits.
      explicit verinum(long val);

	// Copy only the specified number of bits from the
	// source. Also mark this number as has_len.
      verinum(const verinum&, unsigned bits);

      ~verinum();
      verinum& operator= (const verinum&);

	// Number of significant bits in this number.
      unsigned len() const { return nbits_; }

	// A number "has a length" if the length was specified fixed
	// in some way.
      bool has_len() const { return has_len_; }

      bool has_sign(bool flag) { has_sign_ = flag; return has_sign_; }
      bool has_sign() const { return has_sign_; }

	// A number is "defined" if there are no x or z bits in its value.
      bool is_defined() const;
      bool is_zero() const;
      bool is_negative() const;

	// A number is "a string" if its value came directly from
	// an ASCII description instead of a number value.
      bool is_string() const { return string_flag_; }

	// Comparison for use in sorting algorithms.
      bool is_before(const verinum&that) const;

	// Individual bits can be accessed with the get and set
	// methods.
      V get(unsigned idx) const;
      V set(unsigned idx, V val);

      V operator[] (unsigned idx) const { return get(idx); }


      unsigned long as_ulong() const;
      signed long   as_long() const;
      std::string as_string() const;

    private:
      V* bits_;
      unsigned nbits_;
      bool has_len_;
      bool has_sign_;

	// These are some convenience flags that help us do a better
	// job of pretty-printing values.
      bool string_flag_;
};

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


/* These are arithmetic operators. These generally work to produce
   results that do not overflow. That means the result may expand or
   contract to hold the bits needed to hold the operation results
   accurately. It is up to the caller to truncate or pad if a specific
   width is expected. */
extern verinum operator + (const verinum&left, const verinum&right);
extern verinum operator - (const verinum&left, const verinum&right);
extern verinum operator * (const verinum&left, const verinum&right);
extern verinum operator / (const verinum&left, const verinum&right);
extern verinum operator % (const verinum&left, const verinum&right);

extern verinum operator<< (const verinum&left, unsigned shift);
extern verinum operator>> (const verinum&left, unsigned shift);

extern verinum concat(const verinum&left, const verinum&right);

/* Bitwise not returns the ones complement. */
extern verinum v_not(const verinum&left);

/*
 * $Log: verinum.h,v $
 * Revision 1.26.2.4  2007/03/23 20:59:26  steve
 *  Fix compile time evaluation of < operator.
 *
 * Revision 1.26.2.3  2005/12/07 03:28:45  steve
 *  Support constant concatenation of constants.
 *
 * Revision 1.26.2.2  2005/08/13 00:45:55  steve
 *  Fix compilation warnings/errors with newer compilers.
 *
 * Revision 1.26.2.1  2005/06/14 15:33:54  steve
 *  Fix gcc4 build issues.
 *
 * Revision 1.26  2004/02/17 06:52:55  steve
 *  Support unsigned divide of huge numbers.
 *
 * Revision 1.25  2003/10/26 04:54:56  steve
 *  Support constant evaluation of binary ^ operator.
 *
 * Revision 1.24  2003/04/14 03:40:21  steve
 *  Make some effort to preserve bits while
 *  operating on constant values.
 *
 * Revision 1.23  2003/04/03 04:30:00  steve
 *  Prevent overrun comparing verinums to zero.
 *
 * Revision 1.22  2003/01/30 16:23:08  steve
 *  Spelling fixes.
 *
 * Revision 1.21  2003/01/30 04:23:25  steve
 *  include config.h to get iosfwd flags.
 *
 * Revision 1.20  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.19  2002/06/03 04:04:24  steve
 *  Add verinum != operator.
 *
 * Revision 1.18  2001/02/10 20:29:39  steve
 *  In the context of range declarations, use elab_and_eval instead
 *  of the less robust eval_const methods.
 *
 * Revision 1.17  2001/02/09 05:44:23  steve
 *  support evaluation of constant < in expressions.
 *
 * Revision 1.16  2001/02/07 02:46:31  steve
 *  Support constant evaluation of / and % (PR#124)
 *
 * Revision 1.15  2001/01/16 02:44:18  steve
 *  Use the iosfwd header if available.
 *
 * Revision 1.14  2001/01/02 03:23:40  steve
 *  Evaluate constant &, | and unary ~.
 *
 * Revision 1.13  2000/12/10 22:01:36  steve
 *  Support decimal constants in behavioral delays.
 *
 * Revision 1.12  2000/09/27 18:28:37  steve
 *  multiply in parameter expressions.
 *
 * Revision 1.11  2000/02/23 04:43:43  steve
 *  Some compilers do not accept the not symbol.
 *
 * Revision 1.10  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.9  2000/01/07 03:45:49  steve
 *  Initial support for signed constants.
 *
 * Revision 1.8  1999/11/06 16:00:17  steve
 *  Put number constants into a static table.
 *
 * Revision 1.7  1999/10/22 23:57:53  steve
 *  do the <= in bits, not numbers.
 *
 * Revision 1.6  1999/10/10 23:29:37  steve
 *  Support evaluating + operator at compile time.
 *
 * Revision 1.5  1999/05/13 04:02:09  steve
 *  More precise handling of verinum bit lengths.
 *
 * Revision 1.4  1998/12/20 02:05:41  steve
 *  Function to calculate wire initial value.
 *
 * Revision 1.3  1998/11/11 00:01:51  steve
 *  Check net ranges in declarations.
 *
 * Revision 1.2  1998/11/09 18:55:35  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.1  1998/11/03 23:29:08  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
