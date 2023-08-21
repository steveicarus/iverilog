#ifndef IVL_PTask_H
#define IVL_PTask_H
/*
 * Copyright (c) 1999-2021 Stephen Williams (steve@icarus.com)
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

# include  "PScope.h"
# include  "PNamedItem.h"
# include  "StringHeap.h"
# include  <string>
# include  <vector>
# include  <list>
class Design;
class NetExpr;
class NetNet;
class NetScope;
class PChainConstructor;
class PWire;
class Statement;
class PExpr;


class PTaskFunc : public PScope, public PNamedItem {

    public:
      PTaskFunc(perm_string name, LexicalScope*parent);
      ~PTaskFunc();

      bool var_init_needs_explicit_lifetime() const;

      void set_ports(std::vector<pform_tf_port_t>*p);

      void set_this(class_type_t*use_type, PWire*this_wire);

	// If this task is a method of a class, this returns a pointer
	// to the class type.
      inline class_type_t* method_of() const { return this_type_; }


      virtual void elaborate_sig(Design*des, NetScope*scope) const =0;
      virtual void elaborate(Design*des, NetScope*scope) const =0;

      virtual void dump(std::ostream&, unsigned) const =0;

    protected:
	// Elaborate the ports list. Write into the ports vector the
	// NetNet pointers for the ports, and write into the pdefs the
	// default value expressions, if any.
      void elaborate_sig_ports_(Design*des, NetScope*scope,
				std::vector<NetNet*>&ports,
				std::vector<NetExpr*> &pdefs,
				std::vector<perm_string> &port_names) const;

      void dump_ports_(std::ostream&out, unsigned ind) const;

    private:
      class_type_t*this_type_;
      std::vector<pform_tf_port_t>*ports_;
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

      void dump(std::ostream&, unsigned) const;

      SymbolType symbol_type() const;

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
      void set_return(data_type_t*t);

      inline Statement* get_statement() { return statement_; }

	// Push this statement to the front of the existing
	// definition. If the statement is a simple statement, make a
	// block to contain the statements.
      void push_statement_front(Statement*stmt);

	// This is only used if this function is a constructor. In
	// that case, this method looks for a PChainConstructor in the
	// statement and extracts it if found.
      PChainConstructor*extract_chain_constructor();

      void elaborate_scope(Design*des, NetScope*scope) const;

	/* elaborate the ports and return value. */
      void elaborate_sig(Design *des, NetScope*) const;

	/* Elaborate the behavioral statement. */
      void elaborate(Design *des, NetScope*) const;

      bool is_auto() const { return is_auto_; };

      void dump(std::ostream&, unsigned) const;

      SymbolType symbol_type() const;

    private:
      data_type_t* return_type_;
      Statement *statement_;
      bool is_auto_;
};

// A let is like a simple function that is expanded in the compiler
class PLet : public PTaskFunc {
    public:
      typedef struct let_port {
	    data_type_t*type_;
	    perm_string name_;
	    std::list<pform_range_t>*range_;
	    PExpr*def_;

	    void dump(std::ostream&, unsigned) const;
      } let_port_t;

// FIXME: Should the port list be a vector. Check once implemented completely
      explicit PLet(perm_string name, LexicalScope*parent,
                    std::list<let_port_t*>*ports, PExpr*expr);
      ~PLet();

      void elaborate_sig(Design*des, NetScope*scope) const { (void)des; (void)scope; }
      void elaborate(Design*des, NetScope*scope) const { (void)des; (void)scope; }

      void dump(std::ostream&, unsigned) const;

    private:
      std::list<let_port_t*>*ports_;
      PExpr*expr_;
};

#endif /* IVL_PTask_H */
