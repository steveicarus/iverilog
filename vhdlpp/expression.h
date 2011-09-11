#ifndef __expression_H
#define __expression_H
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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
# include  "entity.h"
# include  <inttypes.h>
# include  <list>
# include  <vector>

class Entity;
class Architecture;
class ScopeBase;
class VType;
class VTypeArray;
class VTypePrimitive;

class ExpName;

/*
 * The Expression class represents parsed expressions from the parsed
 * VHDL input. The Expression class is a virtual class that holds more
 * specific derived expression types.
 */
class Expression : public LineInfo {

    public:
      Expression();
      virtual ~Expression() =0;

	// This virtual method handles the special case of elaborating
	// an expression that is the l-value of a sequential variable
	// assignment. This generates an error for most cases, but
	// expressions that are valid l-values return 0 and set any
	// flags needed to indicate their status as writable variables.
      virtual int elaborate_lval(Entity*ent, Architecture*arc,
				 bool is_sequ);

	// This virtual method probes the expression to get the most
	// constrained type for the expression. For a given instance,
	// this may be called before the elaborate_expr method.
      virtual const VType*probe_type(Entity*ent, Architecture*arc) const;

	// This virtual method elaborates an expression. The ltype is
	// the type of the lvalue expression, if known, and can be
	// used to calculate the type for the expression being
	// elaborated.
      virtual int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);

	// Return the type that this expression would be if it were an
	// l-value. This should only be called after elaborate_lval is
	// called and only if elaborate_lval succeeded.
      inline const VType*peek_type(void) const { return type_; }

	// The emit virtual method is called by architecture emit to
	// output the generated code for the expression. The derived
	// class fills in the details of what exactly happened.
      virtual int emit(ostream&out, Entity*ent, Architecture*arc) =0;

	// The evaluate virtual method tries to evaluate expressions
	// to constant literal values. Return true and set the val
	// argument if the evaluation works, or return false if it
	// cannot be done.
      virtual bool evaluate(ScopeBase*scope, int64_t&val) const;

	// The symbolic compare returns true if the two expressions
	// are equal without actually calculating the value.
      virtual bool symbolic_compare(const Expression*that) const;

	// This method returns true if the drawn Verilog for this
	// expression is a primary. A containing expression can use
	// this method to know if it needs to wrap parentheses. This
	// is somewhat optional, so it is better to return false if
	// not certain. The default implementation does return false.
      virtual bool is_primary(void) const;

	// Debug dump of the expression.
      virtual void dump(ostream&out, int indent = 0) const =0;

    protected:
      void set_type(const VType*);

    private:
      const VType*type_;

    private: // Not implemented
      Expression(const Expression&);
      Expression& operator = (const Expression&);
};

static inline void FILE_NAME(Expression*tgt, const LineInfo*src)
{
      tgt->set_line(*src);
}

class ExpUnary : public Expression {

    public:
      ExpUnary(Expression*op1);
      virtual ~ExpUnary() =0;

    protected:
      int emit_operand1(ostream&out, Entity*ent, Architecture*arc);
      void dump_operand1(ostream&out, int indent = 0) const;

    private:
      Expression*operand1_;
};

/*
 * This is an abstract class that collects some of the common features
 * of binary operators.
 */
class ExpBinary : public Expression {

    public:
      ExpBinary(Expression*op1, Expression*op2);
      virtual ~ExpBinary() =0;

      const Expression* peek_operand1(void) const { return operand1_; }
      const Expression* peek_operand2(void) const { return operand2_; }

      const VType*probe_type(Entity*ent, Architecture*arc) const;

    protected:

      int elaborate_exprs(Entity*, Architecture*, const VType*);
      int emit_operand1(ostream&out, Entity*ent, Architecture*arc);
      int emit_operand2(ostream&out, Entity*ent, Architecture*arc);

      bool eval_operand1(ScopeBase*scope, int64_t&val) const;
      bool eval_operand2(ScopeBase*scope, int64_t&val) const;

      void dump_operands(ostream&out, int indent = 0) const;

    private:
      Expression*operand1_;
      Expression*operand2_;
};

class ExpAggregate : public Expression {

    public:
      class choice_t {
	  public:
	      // Create an "others" choice
	    choice_t();
	      // Create a simple_expression choice
	    explicit choice_t(Expression*exp);
	      // Create a named choice
	    explicit choice_t(perm_string name);
	    ~choice_t();

	      // true if this represents an "others" choice
	    bool others() const;
	      // Return expression if this reprents simple_expression.
	    Expression*simple_expression(bool detach_flag =true);

	    void dump(ostream&out, int indent) const;

	  private:
	    Expression*expr_;
	  private: // not implemented
	    choice_t(const choice_t&);
	    choice_t& operator= (const choice_t&);
      };

      struct choice_element {
	    choice_t*choice;
	    Expression*expr;
	    bool alias_flag;
      };

      class element_t {
	  public:
	    explicit element_t(std::list<choice_t*>*fields, Expression*val);
	    ~element_t();

	    size_t count_choices() const { return fields_.size(); }
	    void map_choices(choice_element*dst);

	    void dump(ostream&out, int indent) const;

	  private:
	    std::vector<choice_t*>fields_;
	    Expression*val_;
	  private: // not implemented
	    element_t(const element_t&);
	    element_t& operator = (const element_t&);
      };

    public:
      ExpAggregate(std::list<element_t*>*el);
      ~ExpAggregate();

      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;

    private:
      int elaborate_expr_array_(Entity*ent, Architecture*arc, const VTypeArray*ltype);

    private:
	// This is the elements as directly parsed.
      std::vector<element_t*> elements_;

	// These are the elements after elaboration. This form is
	// easier to check and emit.
      std::vector<choice_element> aggregate_;
};

class ExpArithmetic : public ExpBinary {

    public:
      enum fun_t { PLUS, MINUS, MULT, DIV, MOD, REM, POW, CONCAT };

    public:
      ExpArithmetic(ExpArithmetic::fun_t op, Expression*op1, Expression*op2);
      ~ExpArithmetic();

      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      virtual bool evaluate(ScopeBase*scope, int64_t&val) const;
      void dump(ostream&out, int indent = 0) const;

    private:
      int emit_concat_(ostream&out, Entity*ent, Architecture*arc);

    private:
      fun_t fun_;
};

class ExpAttribute : public Expression {

    public:
      ExpAttribute(ExpName*base, perm_string name);
      ~ExpAttribute();

      inline perm_string peek_attribute() const { return name_; }
      inline const ExpName* peek_base() const { return base_; }

      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;

    private:
      ExpName*base_;
      perm_string name_;
};

class ExpBitstring : public Expression {

    public:
      explicit ExpBitstring(const char*);
      ~ExpBitstring();

      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;

    private:
      std::vector<char>value_;
};


class ExpCharacter : public Expression {

    public:
      ExpCharacter(char val);
      ~ExpCharacter();

      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      bool is_primary(void) const;
      void dump(ostream&out, int indent = 0) const;

      char value() const { return value_; }

    private:
      int emit_primitive_bit_(ostream&out, Entity*ent, Architecture*arc,
			      const VTypePrimitive*etype);

    private:
      char value_;
};

/*
 * The conditional expression represents the VHDL when-else
 * expressions. Note that by the VHDL syntax rules, these cannot show
 * up other then at the root of an expression.
 */
class ExpConditional : public Expression {

    public:
      ExpConditional(Expression*cond, std::list<Expression*>*tru,
		     std::list<Expression*>*els);
      ~ExpConditional();

      const VType*probe_type(Entity*ent, Architecture*arc) const;
      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;

    private:
      Expression*cond_;
      std::list<Expression*> true_clause_;
      std::list<Expression*> else_clause_;
};

/*
 * This is a special expression type that represents posedge/negedge
 * expressions in sensitivity lists.
 */
class ExpEdge : public ExpUnary {

    public:
      enum fun_t { NEGEDGE, ANYEDGE, POSEDGE };

    public:
      explicit ExpEdge(ExpEdge::fun_t ty, Expression*op);
      ~ExpEdge();

      inline fun_t edge_fun() const { return fun_; }

      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;

    private:
      fun_t fun_;
};
class ExpFunc : public Expression {

    public:
      explicit ExpFunc(perm_string nn);
      ExpFunc(perm_string nn, Expression*arg);
      ~ExpFunc();

    public: // Base methods
      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;

    private:
      perm_string name_;
      std::vector<Expression*> argv_;
};

class ExpInteger : public Expression {

    public:
      ExpInteger(int64_t val);
      ~ExpInteger();

      const VType*probe_type(Entity*ent, Architecture*arc) const;
      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      bool is_primary(void) const;
      bool evaluate(ScopeBase*scope, int64_t&val) const;
      void dump(ostream&out, int indent = 0) const;

    private:
      int64_t value_;
};

class ExpLogical : public ExpBinary {

    public:
      enum fun_t { AND, OR, NAND, NOR, XOR, XNOR };

    public:
      ExpLogical(ExpLogical::fun_t ty, Expression*op1, Expression*op2);
      ~ExpLogical();

      inline fun_t logic_fun() const { return fun_; }

      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;

    private:
      fun_t fun_;
};

/*
 * The ExpName class represents an expression that is an identifier or
 * other sort of name. The ExpNameALL is a special case of ExpName
 * that represents the "all" keyword is contexts that can handle it.
 */
class ExpName : public Expression {

    public:
      explicit ExpName(perm_string nn);
      ExpName(perm_string nn, Expression*index);
      ExpName(perm_string nn, Expression*msb, Expression*lsb);
      ~ExpName();

    public: // Base methods
      int elaborate_lval(Entity*ent, Architecture*arc, bool);
      int elaborate_rval(Entity*ent, Architecture*arc, const InterfacePort*);
      const VType* probe_type(Entity*ent, Architecture*arc) const;
      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      bool is_primary(void) const;
      bool evaluate(ScopeBase*scope, int64_t&val) const;
      bool symbolic_compare(const Expression*that) const;
      void dump(ostream&out, int indent = 0) const;
      const char* name() const;

    private:
      perm_string name_;
      Expression*index_;
      Expression*lsb_;
};

class ExpNameALL : public ExpName {

    public:
      ExpNameALL() : ExpName(perm_string()) { }

    public:
      int elaborate_lval(Entity*ent, Architecture*arc, bool);
      const VType* probe_type(Entity*ent, Architecture*arc) const;
      void dump(ostream&out, int indent =0) const;
};

class ExpRelation : public ExpBinary {

    public:
      enum fun_t { EQ, LT, GT, NEQ, LE, GE };

      inline fun_t relation_fun(void) const { return fun_; }

    public:
      ExpRelation(ExpRelation::fun_t ty, Expression*op1, Expression*op2);
      ~ExpRelation();

      const VType* probe_type(Entity*ent, Architecture*arc) const;
      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;

    private:
      fun_t fun_;
};

class ExpString : public Expression {

    public:
      explicit ExpString(const char*);
      ~ExpString();

      int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      bool is_primary(void) const;
      void dump(ostream&out, int indent = 0) const;

    private:
      int emit_as_array_(ostream&out, Entity*ent, Architecture*arc, const VTypeArray*arr);

    private:
      std::vector<char> value_;
};

class ExpUAbs : public ExpUnary {

    public:
      ExpUAbs(Expression*op1);
      ~ExpUAbs();

      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;
};

class ExpUNot : public ExpUnary {

    public:
      ExpUNot(Expression*op1);
      ~ExpUNot();

      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent = 0) const;
};

#endif
