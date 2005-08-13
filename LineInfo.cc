/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: LineInfo.cc,v 1.4.2.1 2005/08/13 00:45:53 steve Exp $"
#endif

# include "config.h"

# include  "LineInfo.h"
# include  <sstream>

LineInfo::LineInfo()
: file_(0), lineno_(0)
{
}

LineInfo::~LineInfo()
{
}

std::string LineInfo::get_line() const
{
      std::ostringstream buf;
      buf << (file_? file_ : "") << ":" << lineno_;

      std::string res = buf.str();
      return res;
}

void LineInfo::set_line(const LineInfo&that)
{
      file_ = that.file_;
      lineno_ = that.lineno_;
}

void LineInfo::set_file(const char*f)
{
      file_ = f;
}

void LineInfo::set_lineno(unsigned n)
{
      lineno_ = n;
}

/*
 * $Log: LineInfo.cc,v $
 * Revision 1.4.2.1  2005/08/13 00:45:53  steve
 *  Fix compilation warnings/errors with newer compilers.
 *
 * Revision 1.4  2003/01/17 05:49:03  steve
 *  Use stringstream in place of sprintf.
 *
 * Revision 1.3  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2000/11/30 17:31:42  steve
 *  Change LineInfo to store const C strings.
 *
 */

