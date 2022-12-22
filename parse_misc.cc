/*
 * Copyright (c) 1998-2022 Stephen Williams (steve@icarus.com)
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

# include  <cstdarg>
# include  "parse_misc.h"
# include  <cstdio>
# include  <iostream>

using namespace std;

extern const char*vl_file;
unsigned error_count = 0;
unsigned warn_count = 0;
unsigned long based_size = 0;

std::ostream& operator << (std::ostream&o, const YYLTYPE&loc)
{
      if (loc.text)
	    o << loc.text << ":";
      else
	    o << "<>:";
      o << loc.first_line;
      return o;
}

void VLwarn(const char*msg)
{
      warn_count += 1;
      cerr << yylloc.text << ":" << yylloc.first_line << ": " << msg << endl;
}

void VLerror(const char*msg)
{
      error_count += 1;
      cerr << yylloc.text << ":" << yylloc.first_line << ": " << msg << endl;
}

void VLerror(const YYLTYPE&loc, const char*msg, ...)
{
      va_list ap;
      va_start(ap, msg);

      fprintf(stderr, "%s:%d: ", loc.text, loc.first_line);
      vfprintf(stderr, msg, ap);
      va_end(ap);
      fprintf(stderr, "\n");

      error_count += 1;
      based_size = 0; /* Clear the base information if we have an error. */
}

void VLwarn(const YYLTYPE&loc, const char*msg)
{
      warn_count += 1;
      cerr << loc << ": " << msg << endl;
}

int VLwrap()
{
      return -1;
}
