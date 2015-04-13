#ifndef IVL_architec_H
#define IVL_architec_H
/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
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

# include  "StringHeap.h"
# include  "LineInfo.h"
# include  "scope.h"
# include  <list>
# include  <map>

class ComponentInstantiation;
class Entity;
class Expression;
class ExpName;
class GenerateStatement;
class SequentialStmt;
class Signal;
class named_expr_t;
class prange_t;

/*
 * The Architecture class carries the contents (name, statements,
 * etc.) of a parsed VHDL architecture. These objects are ultimately
 * put into entities.
 */
class Architecture : public Scope, public LineInfo {

    public:
	// Architectures contain concurrent statements, that are
	// derived from this nested class.
      class Statement : public LineInfo {

	  public:
	    Statement();
	    virtual ~Statement() =0;

	    virtual int elaborate(Entity*ent, Architecture*arc);
	    virtual int emit(ostream&out, Entity*ent, Architecture*arc);
	    virtual void dump(ostream&out, int indent = 0) const;
      };

    public:
	// Create an architecture from its name and its statements.
	// NOTE: The statement list passed in is emptied.
      Architecture(perm_string name, const ActiveScope&ref,
		   std::list<Architecture::Statement*>&s);
      ~Architecture();

      perm_string get_name() const { return name_; }

	// Sets the currently processed component (to be able to reach its parameters).
      void set_cur_component(ComponentInstantiation*component) { cur_component_ = component; }
      bool find_constant(perm_string by_name, const VType*&typ, Expression*&exp) const;

	// Elaborate this architecture in the context of the given entity.
      int elaborate(Entity*entity);

	// These methods are used while in the scope of a generate
	// block to mark that a name is a genvar at this point.
      const VType* probe_genvar_type(perm_string);
      void push_genvar_type(perm_string gname, const VType*gtype);
      void pop_genvar_type(void);

	// These methods are used during EMIT to check for names that
	// are genvar names.
      const GenerateStatement* probe_genvar_emit(perm_string);
      void push_genvar_emit(perm_string gname, const GenerateStatement*);
      void pop_genvar_emit(void);

	// Emit this architecture to the given out file in the context
	// of the specified entity. This method is used by the
	// elaborate code to display generated code to the specified
	// output.
      int emit(ostream&out, Entity*entity);

	// The dump method writes a debug display to the given output.
      void dump(ostream&out, perm_string of_entity, int indent = 0) const;

    private:
      perm_string name_;
	// Concurrent statements local to this architecture
      std::list<Architecture::Statement*> statements_;

      struct genvar_type_t {
	    perm_string name;
	    const VType*vtype;
      };
      std::list<genvar_type_t> genvar_type_stack_;

      struct genvar_emit_t {
	    perm_string name;
	    const GenerateStatement*gen;
      };
      std::list<genvar_emit_t> genvar_emit_stack_;

      // Currently processed component (or NULL if none).
      ComponentInstantiation*cur_component_;

    private: // Not implemented
};

/*
 * This is a base class for various generate statement types. It holds
 * the generate statement name, and a list of substatements.
 */
class GenerateStatement : public Architecture::Statement {

    public:
      GenerateStatement(perm_string gname, std::list<Architecture::Statement*>&s);
      ~GenerateStatement();

      inline perm_string get_name() const { return name_; }

    protected:
      int elaborate_statements(Entity*ent, Architecture*arc);
      int emit_statements(ostream&out, Entity*ent, Architecture*arc);
      void dump_statements(ostream&out, int indent) const;

    private:
      perm_string name_;
      std::list<Architecture::Statement*> statements_;
};

class ForGenerate : public GenerateStatement {

    public:
      ForGenerate(perm_string gname, perm_string genvar,
		  prange_t*rang, std::list<Architecture::Statement*>&s);
      ~ForGenerate();

      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int ident =0) const;

    private:
      perm_string genvar_;
      Expression*lsb_;
      Expression*msb_;
};

class IfGenerate : public GenerateStatement {

    public:
      IfGenerate(perm_string gname, Expression*cond,
		 std::list<Architecture::Statement*>&s);
      ~IfGenerate();

      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*entity, Architecture*arc);

    private:
      Expression*cond_;
};

/*
 * The SignalAssignment class represents the
 * concurrent_signal_assignment that is placed in an architecture.
 */
class SignalAssignment  : public Architecture::Statement {

    public:
      SignalAssignment(ExpName*target, std::list<Expression*>&rval);
      SignalAssignment(ExpName*target, Expression*rval);
      ~SignalAssignment();

      virtual int elaborate(Entity*ent, Architecture*arc);
      virtual int emit(ostream&out, Entity*entity, Architecture*arc);
      virtual void dump(ostream&out, int ident =0) const;

    private:
      ExpName*lval_;
      std::list<Expression*> rval_;
};

class ComponentInstantiation  : public Architecture::Statement {

    public:
      ComponentInstantiation(perm_string iname, perm_string cname,
			     std::list<named_expr_t*>*parms,
			     std::list<named_expr_t*>*ports);
      ~ComponentInstantiation();

      virtual int elaborate(Entity*ent, Architecture*arc);
      virtual int emit(ostream&out, Entity*entity, Architecture*arc);
      virtual void dump(ostream&out, int indent =0) const;

	// Returns the expression that initializes a generic (or NULL if not found).
      Expression*find_generic_map(perm_string by_name) const;

      inline perm_string instance_name() const { return iname_; }
      inline perm_string component_name() const { return cname_; }

    private:
      perm_string iname_;
      perm_string cname_;

      std::map<perm_string,Expression*> generic_map_;
      std::map<perm_string,Expression*> port_map_;
};

class ProcessStatement : public Architecture::Statement {

    public:
      ProcessStatement(perm_string iname,
		       std::list<Expression*>*sensitivity_list,
		       std::list<SequentialStmt*>*statement_list);
      ~ProcessStatement();

      virtual int elaborate(Entity*ent, Architecture*arc);
      virtual int emit(ostream&out, Entity*entity, Architecture*arc);
      virtual void dump(ostream&out, int indent =0) const;

    private:
      int rewrite_as_always_edge_(Entity*ent, Architecture*arc);
      int extract_anyedge_(Entity*ent, Architecture*arc);

    private:
      perm_string iname_;

      std::list<Expression*> sensitivity_list_;
      std::list<SequentialStmt*> statements_list_;

};

#endif /* IVL_architec_H */
