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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: verinum.h,v 1.13 2000/12/10 22:01:36 steve Exp $"
#endif

# include  <string>

/*
 * Numbers in verlog are multibit strings, where each bit has 4
 * possible values: 0, 1, x or z. The verinum number is store in
 * little-endian format. This means that if the long value is 2b'10,
 * get(0) is 0 and get(1) is 1.
 */
class verinum {

    public:
      enum V { V0, V1, Vx, Vz };

      verinum();
      verinum(const string&str);
      verinum(const V*v, unsigned nbits, bool has_len =true);
      verinum(V, unsigned nbits =1);
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

	// A number is "a string" if its value came directly from
	// an ascii description instead of a number value.
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
      string as_string() const;

    private:
      V* bits_;
      unsigned nbits_;
      bool has_len_;
      bool has_sign_;

	// These are some convenience flags that help us do a better
	// job of pretty-printing values.
      bool string_flag_;
};


class ostream;
extern ostream& operator<< (ostream&, const verinum&);
extern ostream& operator<< (ostream&, verinum::V);

extern verinum::V operator == (const verinum&left, const verinum&right);
extern verinum::V operator <= (const verinum&left, const verinum&right);
extern verinum operator + (const verinum&left, const verinum&right);
extern verinum operator - (const verinum&left, const verinum&right);
extern verinum operator * (const verinum&left, const verinum&right);
extern verinum operator / (const verinum&left, const verinum&right);

extern verinum v_not(const verinum&left);

/*
 * $Log: verinum.h,v $
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
