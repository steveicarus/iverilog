#ifndef __PTask_H
#define __PTask_H
/*
 * Copyright (c) 1999-2008 Stephen Williams (steve@icarus.com)
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

# include  "LineInfo.h"
# include  "PScope.h"
# include  "svector.h"
# include  "StringHeap.h"
# include  <string>
class Design;
class NetScope;
class PWire;
class Statement;
class PExpr;

enum PTaskFuncEnum {
      PTF_NONE,
      PTF_REG,
      PTF_REG_S,
      PTF_INTEGER,
      PTF_REAL,
      PTF_REALTIME,
      PTF_TIME
};

struct PTaskFuncArg {
      PTaskFuncEnum type;
      svector<PExpr*>*range;
};

/*
 * The PTask holds the parsed definitions of a task.
 */
class PTask  : public PScope, public LineInfo {

    public:
      explicit PTask(perm_string name, PScope*parent, bool is_auto);
      ~PTask();

      void set_ports(svector<PWire *>*p);
      void set_statement(Statement *s);

	// Tasks introduce scope, to need to be handled during the
	// scope elaboration pass. The scope passed is my scope,
	// created by the containing scope. I fill it in with stuff if
	// I need to.
      void elaborate_scope(Design*des, NetScope*scope) const;

	// Bind the ports to the regs that are the ports.
      void elaborate_sig(Design*des, NetScope*scope) const;

	// Elaborate the statement to finish off the task definition.
      void elaborate(Design*des, NetScope*scope) const;

      bool is_auto() const { return is_auto_; };

      void dump(ostream&, unsigned) const;

    private:
      svector<PWire*>*ports_;
      Statement*statement_;
      bool is_auto_;

    private: // Not implemented
      PTask(const PTask&);
      PTask& operator=(const PTask&);
};

/*
 * The function is similar to a task (in this context) but there is a
 * single output port and a set of input ports. The output port is the
 * function return value.
 *
 * The output value is not elaborated until elaborate_sig.
 */
class PFunction : public PScope, public LineInfo {

    public:
      explicit PFunction(perm_string name, PScope*parent, bool is_auto);
      ~PFunction();

      void set_ports(svector<PWire *>*p);
      void set_statement(Statement *s);
      void set_return(PTaskFuncArg t);

      void elaborate_scope(Design*des, NetScope*scope) const;

	/* elaborate the ports and return value. */
      void elaborate_sig(Design *des, NetScope*) const;

	/* Elaborate the behavioral statement. */
      void elaborate(Design *des, NetScope*) const;

      bool is_auto() const { return is_auto_; };

      void dump(ostream&, unsigned) const;

    private:
      PTaskFuncArg return_type_;
      svector<PWire *> *ports_;
      Statement *statement_;
      bool is_auto_;
};

#endif
