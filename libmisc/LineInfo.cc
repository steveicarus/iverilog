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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "LineInfo.h"
# include  <sstream>

using namespace std;

LineInfo::LineInfo()
: lineno_(0)
{
}

LineInfo::~LineInfo()
{
}

string LineInfo::get_fileline() const
{
      ostringstream buf;
      buf << (file_.str()? file_.str() : "") << ":" << lineno_;

      string res = buf.str();
      return res;
}

void LineInfo::set_line(const LineInfo&that)
{
      file_ = that.file_;
      lineno_ = that.lineno_;
}

void LineInfo::set_file(perm_string f)
{
      file_ = f;
}

void LineInfo::set_lineno(unsigned n)
{
      lineno_ = n;
}
