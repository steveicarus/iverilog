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
#ident "$Id: PTask.h,v 1.4 1999/08/25 22:22:41 steve Exp $"
#endif

# include  "LineInfo.h"
# include  "svector.h"
# include  <string>
class Design;
class PWire;
class Statement;

/*
 * The PTask holds the parsed definitions of a task.
 */
class PTask  : public LineInfo {

    public:
      explicit PTask(svector<PWire*>*p, Statement*s);
      ~PTask();

      virtual void elaborate(Design*des, const string&path) const;
      void dump(ostream&, unsigned) const;

    private:
      svector<PWire*>*ports_;
      Statement*statement_;

    private: // Not implemented
      PTask(const PTask&);
      PTask& operator=(const PTask&);
};

/*
 * The function is similar to a task (in this context) but there is a
 * single output port and a set of input ports. The output port is the
 * function return value.
 */
class PFunction : public LineInfo {

    public:
      explicit PFunction(svector<PWire *>*p, Statement *s);
      ~PFunction();

      void set_output(PWire*);

      virtual void elaborate(Design *des, const string &path) const;
      void dump(ostream&, unsigned) const;

    private:
      PWire*out_;
      svector<PWire *> *ports_;
      Statement *statement_;
};

/*
 * $Log: PTask.h,v $
 * Revision 1.4  1999/08/25 22:22:41  steve
 *  elaborate some aspects of functions.
 *
 * Revision 1.3  1999/07/31 19:14:47  steve
 *  Add functions up to elaboration (Ed Carter)
 *
 * Revision 1.2  1999/07/24 02:11:19  steve
 *  Elaborate task input ports.
 *
 * Revision 1.1  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 */
#endif
