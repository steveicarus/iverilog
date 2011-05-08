#ifndef __architec_H
#define __architec_H
/*
 * Copyright (c) 2011Stephen Williams (steve@icarus.com)
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
# include  "LineInfo.h"
# include  "scope.h"
# include  <list>
# include  <map>

class ComponentBase;
class Entity;
class Expression;
class ExpName;
class Signal;
class named_expr_t;

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

	  private:

	  private: // Not implemented
      };

    public:
	// Create an architecture from its name and its statements.
	// NOTE: The statement list passed in is emptied.
      Architecture(perm_string name, const ScopeBase&ref,
		   std::list<Architecture::Statement*>&s);
      ~Architecture();

      perm_string get_name() const { return name_; }

	// Elaborate this architecture in the context of the given entity.
      int elaborate(Entity*entity);

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

    private: // Not implemented
};

/*
 * The SignalAssignment class represents the
 * concurrent_signal_assignment that is placed in an architecture.
 */
class SignalAssignment  : public Architecture::Statement {

    public:
      SignalAssignment(ExpName*target, std::list<Expression*>&rval);
      ~SignalAssignment();

      virtual int emit(ostream&out, Entity*entity, Architecture*arc);
      virtual void dump(ostream&out, int ident =0) const;

    private:
      ExpName*lval_;
      std::list<Expression*> rval_;
};

class ComponentInstantiation  : public Architecture::Statement {

    public:
      ComponentInstantiation(perm_string iname, perm_string cname,
			     std::list<named_expr_t*>*ports);
      ~ComponentInstantiation();

      virtual int elaborate(Entity*ent, Architecture*arc);
      virtual int emit(ostream&out, Entity*entity, Architecture*arc);
      virtual void dump(ostream&out, int indent =0) const;

    private:
      perm_string iname_;
      perm_string cname_;

      std::map<perm_string,Expression*> port_map_;
};

class ProcessStatement : public Architecture::Statement {

    public:
      ProcessStatement(perm_string iname);
      ~ProcessStatement();

    private:
      perm_string iname_;

};

#endif
