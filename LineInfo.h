#ifndef __LineInfo_H
#define __LineInfo_H
/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: LineInfo.h,v 1.7.2.2 2005/08/13 00:45:53 steve Exp $"
#endif

# include  <string>

using namespace std;

/*
 * This class holds line information for an internal object.
 *
 * Note that the file names are C-style strings that are allocated by
 * the lexor (which parses the line directives) and are never
 * deallocated. We can therefore safely store the pointer and never
 * delete the string, even if LineInfo objects are destroyed.
 */

class LineInfo {
    public:
      LineInfo();
      ~LineInfo();

      std::string get_line() const;

      void set_line(const LineInfo&that);

      void set_file(const char*f);
      void set_lineno(unsigned n);

    private:
      const char* file_;
      unsigned lineno_;
};

/*
 * $Log: LineInfo.h,v $
 * Revision 1.7.2.2  2005/08/13 00:45:53  steve
 *  Fix compilation warnings/errors with newer compilers.
 *
 * Revision 1.7.2.1  2005/06/14 15:33:54  steve
 *  Fix gcc4 build issues.
 *
 * Revision 1.7  2003/01/17 05:49:03  steve
 *  Use stringstream in place of sprintf.
 *
 * Revision 1.6  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2000/11/30 17:31:42  steve
 *  Change LineInfo to store const C strings.
 *
 * Revision 1.4  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.3  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 */
#endif
