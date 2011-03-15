#ifndef __vhdlreal_h
#define __vhdlreal_h
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

#include <iostream>
#include <cmath>

/*
* This class holds a floating point decimal number. The number is
* stored as double. All based numbers are converted by an external
* function to a double and then stored as class instance.
*/
class vhdlreal
{
    public:
      friend std::ostream& operator<< (std::ostream&, const vhdlreal&);
      friend vhdlreal operator+ (const vhdlreal&, const vhdlreal&);
      friend vhdlreal operator- (const vhdlreal&, const vhdlreal&);
      friend vhdlreal operator* (const vhdlreal&, const vhdlreal&);
      friend vhdlreal operator/ (const vhdlreal&, const vhdlreal&);
      friend vhdlreal operator% (const vhdlreal&, const vhdlreal&);
      friend vhdlreal pow(const vhdlreal&, const vhdlreal&);
	// Unary minus.
      friend vhdlreal operator- (const vhdlreal&);

      explicit vhdlreal();
      explicit vhdlreal(const char*text);
      explicit vhdlreal(const double& val);
      vhdlreal(const vhdlreal& val);
      virtual ~vhdlreal() {};

      double as_double() const
      {
	    return value_;
      }
    private:
      double value_;
};

#endif
