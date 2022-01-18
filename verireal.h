#ifndef IVL_verireal_H
#define IVL_verireal_H
/*
 * Copyright (c) 1999-2021 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
# include  <iostream>
#endif

class verinum;

/*
 * This class holds a floating point decimal number. The number is
 * stored as an integer mantissa and a power of 10. The mantissa is an
 * integer so that decimal numbers in the source (which are decimal)
 * can be stored exactly.
 */

class verireal {

      friend std::ostream& operator<< (std::ostream&, const verireal&);
      friend verireal operator+ (const verireal&, const verireal&);
      friend verireal operator- (const verireal&, const verireal&);
      friend verireal operator* (const verireal&, const verireal&);
      friend verireal operator/ (const verireal&, const verireal&);
      friend verireal operator% (const verireal&, const verireal&);

	// Unary minus.
      friend verireal operator- (const verireal&);

    public:
      explicit verireal();
      explicit verireal(const char*text);
      explicit verireal(long val);
      explicit verireal(double val);
      ~verireal();

      // Return the value of the floating point number as an
      // integer, rounded as needed. The shift is the power of 10 to
      // multiply the value before calculating the result. So for
      // example if the value is 2.5 and shift == 1, the result
      // is 25.
      long as_long() const;
      int64_t as_long64(int shift =0) const;

      double as_double() const;

    private:
      double value_;
};

extern std::ostream& operator<< (std::ostream&, const verireal&);
extern verireal operator* (const verireal&, const verireal&);
extern verireal operator/ (const verireal&, const verireal&);
extern verireal operator% (const verireal&, const verireal&);
extern verireal operator- (const verireal&);

#endif /* IVL_verireal_H */
