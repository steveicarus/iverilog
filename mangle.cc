/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: mangle.cc,v 1.2 1999/02/15 05:52:50 steve Exp $"
#endif

# include  "target.h"
# include  <strstream>

string mangle(const string&str)
{
      if (str[0] == '$')
	    return str;

      ostrstream res;
      string tmp = str;

      while (tmp.length() > 0) {
	    size_t pos = tmp.find_first_of(".<");
	    if (pos > tmp.length())
		  pos = tmp.length();

	    res << "S" << pos << tmp.substr(0, pos);
	    if (pos >= tmp.length())
		  break;

	    tmp = tmp.substr(pos);
	    switch (tmp[0]) {
		case '.':
		  tmp = tmp.substr(1);
		  break;
		case '<':
		  tmp = tmp.substr(1);
		  res << "_";
		  while (tmp[0] != '>') {
			res << tmp[0];
			tmp = tmp.substr(1);
		  }
		  tmp = tmp.substr(1);
		  res << "_";
		  break;
	    }
      }
      res << ends;
      return res.str();
}

/*
 * $Log: mangle.cc,v $
 * Revision 1.2  1999/02/15 05:52:50  steve
 *  Mangle that handles device instance numbers.
 *
 * Revision 1.1  1998/11/03 23:29:00  steve
 *  Introduce verilog to CVS.
 *
 */

