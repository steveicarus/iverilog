#ifndef __PTask_H
#define __PTask_H
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
#ident "$Id: PTask.h,v 1.1 1999/07/03 02:12:51 steve Exp $"
#endif

# include  "LineInfo.h"
# include  <string>
class Design;
class Statement;

/*
 * The PTask holds the parsed definitions of a task.
 */
class PTask  : public LineInfo {

    public:
      explicit PTask(Statement*s);
      ~PTask();

      virtual void elaborate(Design*des, const string&path) const;
      void dump(ostream&, unsigned) const;

    private:
      Statement*statement_;

    private: // Not implemented
      PTask(const PTask&);
      PTask& operator=(const PTask&);
};

/*
 * $Log: PTask.h,v $
 * Revision 1.1  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 */
#endif
