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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: mangle.cc,v 1.6 2001/04/22 23:09:46 steve Exp $"
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
	    size_t pos = tmp.find_first_of(".<\\[]/");
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
		case '\\':
		  res << "$$";
		  tmp = tmp.substr(1);
		  break;
		case '/':
		  res << "$sl$";
		  tmp = tmp.substr(1);
		  break;
		case '[':
		  res << "$lb$";
		  tmp = tmp.substr(1);
		  break;
		case ']':
		  res << "$rb$";
		  tmp = tmp.substr(1);
		  break;
	    }
      }
      res << ends;
      return res.str();
}

string stresc(const string&str)
{
      ostrstream res;
      string tmp = str;

      while (tmp.length() > 0) {
	    size_t pos = tmp.find_first_of("\\");
	    if (pos > tmp.length())
		  pos = tmp.length();

	    res << tmp.substr(0, pos);
	    if (pos >= tmp.length())
		  break;

	    tmp = tmp.substr(pos);
	    switch (tmp[0]) {
		case '\\':
		  res << "\\\\";
		  tmp = tmp.substr(1);
		  break;
	    }
      }
      res << ends;
      return res.str();
}

/*
 * $Log: mangle.cc,v $
 * Revision 1.6  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.5  2000/12/11 01:06:24  steve
 *  Mangle [] characters. (PR#67)
 *
 * Revision 1.4  2000/10/25 23:21:37  steve
 *  mangle the backslash to a dollar.
 *
 * Revision 1.3  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.2  1999/02/15 05:52:50  steve
 *  Mangle that handles device instance numbers.
 *
 * Revision 1.1  1998/11/03 23:29:00  steve
 *  Introduce verilog to CVS.
 *
 */

