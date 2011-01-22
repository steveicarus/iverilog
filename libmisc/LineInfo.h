#ifndef __LineInfo_H
#define __LineInfo_H
/*
 * Copyright (c) 1999-2009 Stephen Williams (steve@icarus.com)
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

# include  "StringHeap.h"
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
      virtual ~LineInfo();

	// Get a fully formatted file/lineno
      string get_fileline() const;
	// Set the file/line fro another LineInfo object.
      void set_line(const LineInfo&that);

	// Access parts of LineInfo data
      void set_file(perm_string f);
      void set_lineno(unsigned n);

      perm_string get_file() const { return file_; }
      unsigned  get_lineno() const { return lineno_; }
    private:
      perm_string file_;
      unsigned lineno_;
};

#endif
