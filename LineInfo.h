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
#if !defined(WINNT)
#ident "$Id: LineInfo.h,v 1.2 1999/02/01 00:26:48 steve Exp $"
#endif

# include  <cstdio>

class LineInfo {
    public:
      LineInfo() : lineno_(0) { }

      string get_line() const
	    { char buf[8];
	      sprintf(buf, "%u", lineno_);
	      return file_ + ":" + buf;
	    }

      void set_line(const LineInfo&that)
	    { file_ = that.file_;
	      lineno_ = that.lineno_;
	    }

      void set_file(const string&f) { file_ = f; }
      void set_lineno(unsigned n) { lineno_ = n; }

    private:
      string file_;
      unsigned lineno_;
};

/*
 * $Log: LineInfo.h,v $
 * Revision 1.2  1999/02/01 00:26:48  steve
 *  Carry some line info to the netlist,
 *  Dump line numbers for processes.
 *  Elaborate prints errors about port vector
 *  width mismatch
 *  Emit better handles null statements.
 *
 * Revision 1.1  1999/01/25 05:45:56  steve
 *  Add the LineInfo class to carry the source file
 *  location of things. PGate, Statement and PProcess.
 *
 *  elaborate handles module parameter mismatches,
 *  missing or incorrect lvalues for procedural
 *  assignment, and errors are propogated to the
 *  top of the elaboration call tree.
 *
 *  Attach line numbers to processes, gates and
 *  assignment statements.
 *
 */
#endif
