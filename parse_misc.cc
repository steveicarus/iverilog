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
#if !defined(WINNT)
#ident "$Id: parse_misc.cc,v 1.1 1998/11/03 23:29:02 steve Exp $"
#endif

# include  "parse_misc.h"
# include  <iostream.h>

extern const char*vl_file;

void VLerror(const char*msg)
{
      cerr << yylloc.text << ":" << yylloc.first_line << ": " << msg << endl;
}

void VLerror(const YYLTYPE&loc, const char*msg)
{
      if (loc.text)
	    cerr << loc.text << ":";

      cerr << loc.first_line << ": " << msg << endl;
}

void yywarn(const YYLTYPE&loc, const char*msg)
{
      if (loc.text)
	    cerr << loc.text << ":";

      cerr << loc.first_line << ": warning -- " << msg << endl;
}

int VLwrap()
{
      return -1;
}

/*
 * $Log: parse_misc.cc,v $
 * Revision 1.1  1998/11/03 23:29:02  steve
 *  Introduce verilog to CVS.
 *
 */

