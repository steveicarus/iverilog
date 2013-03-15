#ifndef __PTask_H
#define __PTask_H
/*
 * Copyright (c) 1999-2008,2010,2012 Stephen Williams (steve@icarus.com)
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
# include  "PScope.h"
# include  "StringHeap.h"
# include  <string>
# include  <vector>
# include  <list>
class Design;
class NetNet;
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
      PTF_TIME,
      PTF_ATOM2,
      PTF_ATOM2_S,
      PTF_STRING,
      PTF_VOID
};

struct PTaskFuncArg {
      PTaskFuncEnum type;
      std::list<pform_range_t>*range;
};

class PTaskFunc : public PScope, public LineInfo {

    public:
      PTaskFunc(perm_string name, LexicalScope*parent);
      ~PTaskFunc();

      void set_ports(std::vector<PWire *>*p);

      void set_this(class_type_t*use_type, PWire*this_wire);

	// If this task is a method of a class, this returns a pointer
	// to the class type.
      inline class_type_t* method_of() const { return this_type_; }

    protected:
      void elaborate_sig_ports_(Design*des, NetScope*scope,
				std::vector<NetNet*>&ports) const;

      void dump_ports_(std::ostream&out, unsigned ind) const;

    private:
      class_type_t*this_type_;
      std::vector<PWire*>*ports_;
};

/*
 * The PTask holds the parsed definitions of a task.
 */
class PTask  : public PTaskFunc {

    public:
      explicit PTask(perm_string name, LexicalScope*parent, bool is_auto);
      ~PTask();

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
class PFunction : public PTaskFunc {

    public:
      explicit PFunction(perm_string name, LexicalScope*parent, bool is_auto);
      ~PFunction();

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
      Statement *statement_;
      bool is_auto_;
};

#endif
