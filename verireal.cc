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

# include "config.h"
# include "compiler.h"

# include  "verireal.h"
# include  "verinum.h"
# include  <cstdlib>
# include  <cctype>
# include  <iostream>
# include  <cmath>
# include  <cassert>
# include  <cstring>

using namespace std;

verireal::verireal()
{
      value_ = 0.0;
}

verireal::verireal(const char*txt)
{
      char*tmp = new char[strlen(txt)+1];
      char*cp = tmp;
      for (unsigned idx = 0 ;  txt[idx] ;  idx += 1) {
	    if (txt[idx] == '_')
		  continue;

	    *cp++ = txt[idx];
      }
      cp[0] = 0;

      value_ = strtod(tmp, 0);
      delete[]tmp;
}

verireal::verireal(long val)
{
      value_ = (double)val;
}

verireal::verireal(double val)
{
      value_ = val;
}

verireal::~verireal()
{
}

long verireal::as_long() const
{
      double out = value_;
      double outf;

      if (out >= 0.0) {
	    outf = floor(out);
	    if (out >= (outf + 0.5))
		  outf += 1.0;
      } else {
	    outf = ceil(out);
	    if (out <= (outf - 0.5))
		  outf -= 1.0;
      }
      return (long) outf;
}

int64_t verireal::as_long64(int shift) const
{
      double out = value_ * pow(10.0,shift);
      double outf;

      if (out >= 0.0) {
	    outf = floor(out);
	    if (out >= (outf + 0.5))
		  outf += 1.0;
      } else {
	    outf = ceil(out);
	    if (out <= (outf - 0.5))
		  outf -= 1.0;
      }
      return (int64_t) outf;
}

double verireal::as_double() const
{
      return value_;
}

verireal operator+ (const verireal&l, const verireal&r)
{
      verireal res;
      res.value_ = l.value_ + r.value_;
      return res;
}

verireal operator- (const verireal&l, const verireal&r)
{
      verireal res;
      res.value_ = l.value_ - r.value_;
      return res;
}

verireal operator* (const verireal&l, const verireal&r)
{
      verireal res;
      res.value_ = l.value_ * r.value_;
      return res;
}

verireal operator/ (const verireal&l, const verireal&r)
{
      verireal res;
      res.value_ = l.value_ / r.value_;
      return res;
}

verireal operator% (const verireal&l, const verireal&r)
{
      verireal res;
	// Modulus of a real value is not supported by the standard,
	// but we support it as an extension. Assert that we are in
	// the correct state before doing the operation.
      assert(gn_icarus_misc_flag);
      res.value_ = fmod(l.value_, r.value_);
      return res;
}

verireal operator- (const verireal&l)
{
      verireal res;
      res.value_ = - l.value_;
      return res;
}

ostream& operator<< (ostream&out, const verireal&v)
{
      out << showpoint << v.value_;
      return out;
}
