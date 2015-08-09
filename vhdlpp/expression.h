#ifndef IVL_expression_H
#define IVL_expression_H
/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2015 / Stephen Williams (steve@icarus.com),
 * @author Maciej Suminski (maciej.suminski@cern.ch)
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
# include  "entity.h"
# include  <inttypes.h>
# include  <list>
# include  <memory>
# include  <vector>

class prange_t;
class Entity;
class ScopeBase;
class SubprogramHeader;
class VType;
class VTypeArray;
class VTypePrimitive;
class ExpName;

struct ExprVisitor {
    virtual ~ExprVisitor() {};
    virtual void operator() (Expression*s) = 0;
};

/*
 * The Expression class represents parsed expressions from the parsed
 * VHDL input. The Expression class is a virtual class that holds more
 * specific derived expression types.
 */
class Expression : public LineInfo {

    public:
      Expression();
      virtual ~Expression() =0;

	// Returns a deep copy of the expression.
      virtual Expression*clone() const =0;

	// This virtual method handles the special case of elaborating
	// an expression that is the l-value of a sequential variable
	// assignment. This generates an error for most cases, but
	// expressions that are valid l-values return 0 and set any
	// flags needed to indicate their status as writable variables.
      virtual int elaborate_lval(Entity*ent, ScopeBase*scope,
				 bool is_sequ);

	// This virtual method probes the expression to get the most
	// constrained type for the expression. For a given instance,
	// this may be called before the elaborate_expr method.
      virtual const VType*probe_type(Entity*ent, ScopeBase*scope) const;

	// The fit_type virtual method is used by the ExpConcat class
	// to probe the type of operands. The atype argument is the
	// type of the ExpConcat expression itself. This expression
	// returns its type as interpreted in this context. Really,
	// this is mostly about helping aggregate expressions within
	// concatenations to figure out their type.
      virtual const VType*fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const;

	// This virtual method elaborates an expression. The ltype is
	// the type of the lvalue expression, if known, and can be
	// used to calculate the type for the expression being
	// elaborated.
      virtual int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);

	// Return the type that this expression would be if it were an
	// l-value. This should only be called after elaborate_lval is
	// called and only if elaborate_lval succeeded.
      inline const VType*peek_type(void) const { return type_; }

	// This virtual method writes a VHDL-accurate representation
	// of this expression to the designated stream. This is used
	// for writing parsed types to library files.
      virtual void write_to_stream(std::ostream&fd) const =0;

	// The emit virtual method is called by architecture emit to
	// output the generated code for the expression. The derived
	// class fills in the details of what exactly happened.
      virtual int emit(ostream&out, Entity*ent, ScopeBase*scope) =0;

	// The emit_package virtual message is similar, but is called
	// in a package context and to emit SV packages.
      virtual int emit_package(std::ostream&out);

	// The evaluate virtual method tries to evaluate expressions
	// to constant literal values. Return true and set the val
	// argument if the evaluation works, or return false if it
	// cannot be done.
      virtual bool evaluate(ScopeBase*scope, int64_t&val) const;
      virtual bool evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const;

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
      virtual ostream& dump_inline(ostream&out) const;

	// Recursively visits a tree of expressions (useful of complex expressions).
      virtual void visit(ExprVisitor& func) { func(this); }

    protected:
	// This function is called by the derived class during
	// elaboration to set the type of the current expression that
	// elaboration assigns to this expression.
      void set_type(const VType*);

    private:
      const VType*type_;

    private: // Not implemented
      Expression(const Expression&);
      Expression& operator = (const Expression&);
};

/*
 * Checks before cloning if the other expression actually exists (!=NULL).
 */
static inline Expression*safe_clone(const Expression*other) {
      return (other ? other->clone() : NULL);
}

static inline void FILE_NAME(Expression*tgt, const LineInfo*src)
{
      tgt->set_line(*src);
}

static inline ostream& operator <<(ostream&out, const Expression&exp)
{
      return exp.dump_inline(out);
}

class ExpUnary : public Expression {

    public:
      ExpUnary(Expression*op1);
      virtual ~ExpUnary() =0;

      inline const Expression*peek_operand() const { return operand1_; }

      const VType*fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void visit(ExprVisitor& func);

    protected:
      inline void write_to_stream_operand1(std::ostream&fd) const
      { operand1_->write_to_stream(fd); }

      int emit_operand1(ostream&out, Entity*ent, ScopeBase*scope);
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

      inline const Expression* peek_operand1(void) const { return operand1_; }
      inline const Expression* peek_operand2(void) const { return operand2_; }

      const VType*probe_type(Entity*ent, ScopeBase*scope) const;
      void visit(ExprVisitor& func);

    protected:

      int elaborate_exprs(Entity*, ScopeBase*, const VType*);
      int emit_operand1(ostream&out, Entity*ent, ScopeBase*scope);
      int emit_operand2(ostream&out, Entity*ent, ScopeBase*scope);

      bool eval_operand1(ScopeBase*scope, int64_t&val) const;
      bool eval_operand2(ScopeBase*scope, int64_t&val) const;

      inline void write_to_stream_operand1(std::ostream&out) const
          { operand1_->write_to_stream(out); }
      inline void write_to_stream_operand2(std::ostream&out) const
          { operand2_->write_to_stream(out); }

      void dump_operands(ostream&out, int indent = 0) const;

    private:
      virtual const VType*resolve_operand_types_(const VType*t1, const VType*t2) const;

    private:
      Expression*operand1_;
      Expression*operand2_;
};

class ExpAggregate : public Expression {

    public:
	// A "choice" is only part of an element. It is the thing that
	// is used to identify an element of the aggregate. It can
	// represent the index (or range) of an array, or the name of
	// a record member.
      class choice_t {
	  public:
	      // Create an "others" choice
	    choice_t();
	      // Create a simple_expression choice
	    explicit choice_t(Expression*exp);
	      // Create a named choice
	    explicit choice_t(perm_string name);
	      // discreate_range choice
	    explicit choice_t(prange_t*ran);

	    choice_t(const choice_t&other);

	    ~choice_t();

	      // true if this represents an "others" choice
	    bool others() const;
	      // Return expression if this represents a simple_expression.
	    Expression*simple_expression(bool detach_flag =true);
	      // Return prange_t if this represents a range_expression
	    prange_t*range_expressions(void);

	    void write_to_stream(std::ostream&fd);
	    void dump(ostream&out, int indent) const;

	  private:
	    std::auto_ptr<Expression>expr_;
	    std::auto_ptr<prange_t>  range_;
	  private: // not implemented
	    choice_t& operator= (const choice_t&);
      };

      struct choice_element {
	    choice_element() : choice(), expr() {}

	    choice_element(const choice_element&other) {
	        choice = other.choice ? new choice_t(*other.choice) : NULL;
	        expr = safe_clone(other.expr);
	    }

	    choice_t*choice;
	    Expression*expr;
	    bool alias_flag;
      };

	// Elements are the syntactic items in an aggregate
	// expression. Each element expressions a bunch of fields
	// (choices) and binds them to a single expression
      class element_t {
	  public:
	    explicit element_t(std::list<choice_t*>*fields, Expression*val);
	    element_t(const element_t&other);
	    ~element_t();

	    size_t count_choices() const { return fields_.size(); }
	    void map_choices(choice_element*dst);

	    inline Expression* extract_expression() { return val_; }
	    void write_to_stream(std::ostream&fd) const;

	    void dump(ostream&out, int indent) const;

	  private:
	    std::vector<choice_t*>fields_;
	    Expression*val_;
	  private: // not implemented
	    element_t& operator = (const element_t&);
      };

    public:
      ExpAggregate(std::list<element_t*>*el);
      ~ExpAggregate();

      Expression*clone() const;

      const VType*probe_type(Entity*ent, ScopeBase*scope) const;
      const VType*fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;
      void visit(ExprVisitor& func);

    private:
      int elaborate_expr_array_(Entity*ent, ScopeBase*scope, const VTypeArray*ltype);
      int elaborate_expr_record_(Entity*ent, ScopeBase*scope, const VTypeRecord*ltype);
      int emit_array_(ostream&out, Entity*ent, ScopeBase*scope, const VTypeArray*ltype);
      int emit_record_(ostream&out, Entity*ent, ScopeBase*scope, const VTypeRecord*ltype);

    private:
	// This is the elements as directly parsed.
      std::vector<element_t*> elements_;

	// These are the elements after elaboration. This form is
	// easier to check and emit.
      std::vector<choice_element> aggregate_;
};

class ExpArithmetic : public ExpBinary {

    public:
      enum fun_t { PLUS, MINUS, MULT, DIV, MOD, REM, POW, xCONCAT };

    public:
      ExpArithmetic(ExpArithmetic::fun_t op, Expression*op1, Expression*op2);
      ~ExpArithmetic();

      Expression*clone() const {
          return new ExpArithmetic(fun_, peek_operand1()->clone(), peek_operand2()->clone());
      }

      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      virtual bool evaluate(ScopeBase*scope, int64_t&val) const;
      void dump(ostream&out, int indent = 0) const;

    private:
      const VType* resolve_operand_types_(const VType*t1, const VType*t2) const;

    private:
      fun_t fun_;
};

class ExpAttribute : public Expression {

    public:
      ExpAttribute(ExpName*base, perm_string name);
      ~ExpAttribute();

      Expression*clone() const;

      inline perm_string peek_attribute() const { return name_; }
      inline const ExpName* peek_base() const { return base_; }

      const VType*probe_type(Entity*ent, ScopeBase*scope) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
	// Some attributes can be evaluated at compile time
      bool evaluate(ScopeBase*scope, int64_t&val) const;
      bool evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const;
      void dump(ostream&out, int indent = 0) const;
      void visit(ExprVisitor& func);

    private:
      ExpName*base_;
      perm_string name_;
};

class ExpBitstring : public Expression {

    public:
      explicit ExpBitstring(const char*);
      ExpBitstring(const ExpBitstring&other) : Expression() { value_ = other.value_; }
      ~ExpBitstring();

      Expression*clone() const { return new ExpBitstring(*this); }

      const VType*fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;

    private:
      std::vector<char>value_;
};


class ExpCharacter : public Expression {

    public:
      ExpCharacter(char val);
      ExpCharacter(const ExpCharacter&other) : Expression() { value_ = other.value_; }
      ~ExpCharacter();

      Expression*clone() const { return new ExpCharacter(*this); }

      const VType*fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      bool is_primary(void) const;
      void dump(ostream&out, int indent = 0) const;

      char value() const { return value_; }

    private:
      int emit_primitive_bit_(ostream&out, Entity*ent, ScopeBase*scope,
			      const VTypePrimitive*etype);

    private:
      char value_;
};

class ExpConcat : public Expression {

    public:
      ExpConcat(Expression*op1, Expression*op2);
      ~ExpConcat();

      Expression*clone() const {
          return new ExpConcat(operand1_->clone(), operand2_->clone());
      }

      const VType*probe_type(Entity*ent, ScopeBase*scope) const;
      const VType*fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      virtual bool evaluate(ScopeBase*scope, int64_t&val) const;
      bool is_primary(void) const;
      void dump(ostream&out, int indent = 0) const;
      void visit(ExprVisitor& func);

    private:
      int elaborate_expr_array_(Entity*ent, ScopeBase*scope, const VTypeArray*ltype);

    private:
      Expression*operand1_;
      Expression*operand2_;
};

/*
 * The conditional expression represents the VHDL when-else
 * expressions. Note that by the VHDL syntax rules, these cannot show
 * up other than at the root of an expression.
 */
class ExpConditional : public Expression {

    public:
      class case_t : public LineInfo {
	  public:
	    case_t(Expression*cond, std::list<Expression*>*tru);
	    case_t(const case_t&other);
	    ~case_t();

	    inline Expression*condition() { return cond_; }
	    inline void set_condition(Expression*cond) { cond_ = cond; }

	    int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*lt);
	    int emit_option(ostream&out, Entity*ent, ScopeBase*scope);
	    int emit_default(ostream&out, Entity*ent, ScopeBase*scope);
	    void dump(ostream&out, int indent = 0) const;
            std::list<Expression*>& extract_true_clause() { return true_clause_; }
            void visit(ExprVisitor& func);

	  private:
	    Expression*cond_;
	    std::list<Expression*> true_clause_;
      };

    public:
      ExpConditional(Expression*cond, std::list<Expression*>*tru,
		     std::list<case_t*>*options);
      virtual ~ExpConditional();

      virtual Expression*clone() const;

      const VType*probe_type(Entity*ent, ScopeBase*scope) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;
      void visit(ExprVisitor& func);

    protected:
      std::list<case_t*> options_;
};

/*
 * Expression to handle selected assignments (with .. select target <= value when ..)
 */
class ExpSelected : public ExpConditional {
    public:
      ExpSelected(Expression*selector, std::list<case_t*>*options);
      ~ExpSelected();

      Expression*clone() const;

    private:
      Expression*selector_;
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

      Expression*clone() const { return new ExpEdge(fun_, peek_operand()->clone()); }

      inline fun_t edge_fun() const { return fun_; }

      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;

    private:
      fun_t fun_;
};

class ExpFunc : public Expression {

    public:
      explicit ExpFunc(perm_string nn);
      ExpFunc(perm_string nn, std::list<Expression*>*args);
      ~ExpFunc();

      Expression*clone() const;

      const VType*fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const;
      inline perm_string func_name() const { return name_; }
      inline size_t func_args() const { return argv_.size(); }
      inline const Expression*func_arg(size_t idx) const { return argv_[idx]; }
      const VType*func_ret_type() const;

    public: // Base methods
      const VType*probe_type(Entity*ent, ScopeBase*scope) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;
      void visit(ExprVisitor& func); // NOTE: does not handle expressions in subprogram

    private:
      perm_string name_;
      std::vector<Expression*> argv_;
      SubprogramHeader*def_;
};

class ExpInteger : public Expression {

    public:
      ExpInteger(int64_t val);
      ExpInteger(const ExpInteger&other) : Expression(), value_(other.value_) {}
      ~ExpInteger();

      Expression*clone() const { return new ExpInteger(*this); }

      const VType*probe_type(Entity*ent, ScopeBase*scope) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      int emit_package(std::ostream&out);
      bool is_primary(void) const { return true; }
      bool evaluate(ScopeBase*scope, int64_t&val) const;
      void dump(ostream&out, int indent = 0) const;
      virtual ostream& dump_inline(ostream&out) const;

    private:
      int64_t value_;
};

class ExpReal : public Expression {

    public:
      ExpReal(double val);
      ExpReal(const ExpReal&other) : Expression(), value_(other.value_) {}
      ~ExpReal();

      Expression*clone() const { return new ExpReal(*this); }

      const VType*probe_type(Entity*ent, ScopeBase*scope) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      int emit_package(std::ostream&out);
      bool is_primary(void) const;
      void dump(ostream&out, int indent = 0) const;
      virtual ostream& dump_inline(ostream&out) const;

    private:
      double value_;
};

class ExpLogical : public ExpBinary {

    public:
      enum fun_t { AND, OR, NAND, NOR, XOR, XNOR };

    public:
      ExpLogical(ExpLogical::fun_t ty, Expression*op1, Expression*op2);
      ~ExpLogical();

      Expression*clone() const {
          return new ExpLogical(fun_, peek_operand1()->clone(), peek_operand2()->clone());
      }

      inline fun_t logic_fun() const { return fun_; }

      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
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
      ExpName(perm_string nn, std::list<Expression*>*indices);
      ExpName(perm_string nn, Expression*msb, Expression*lsb);
      ExpName(ExpName*prefix, perm_string nn);
      ExpName(ExpName*prefix, perm_string nn, Expression*msb, Expression*lsb);
      ~ExpName();

    public: // Base methods
      Expression*clone() const {
          return new ExpName(static_cast<ExpName*>(safe_clone(prefix_.get())),
                  name_, safe_clone(index_), safe_clone(lsb_));
      }
      int elaborate_lval(Entity*ent, ScopeBase*scope, bool);
      int elaborate_rval(Entity*ent, ScopeBase*scope, const InterfacePort*);
      const VType* probe_type(Entity*ent, ScopeBase*scope) const;
      const VType* fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*host) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      bool is_primary(void) const;
      bool evaluate(ScopeBase*scope, int64_t&val) const;
      bool evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const;
      bool symbolic_compare(const Expression*that) const;
      void dump(ostream&out, int indent = 0) const;
      inline const char* name() const { return name_; }
      inline const perm_string& peek_name() const { return name_; }
      void set_range(Expression*msb, Expression*lsb);
      void visit(ExprVisitor& func);

    private:
      class index_t {
      public:
          index_t(Expression*idx, Expression*size, Expression*offset = NULL) :
            idx_(idx), size_(size), offset_(offset) {}
          ~index_t() {
                delete idx_;
                delete size_;
                delete offset_;
          }

          int emit(ostream&out, Entity*ent, ScopeBase*scope);

      private:
          Expression*idx_;
          Expression*size_;
          Expression*offset_;
      };

      const VType* elaborate_adjust_type_with_range_(Entity*ent, ScopeBase*scope, const VType*type);

      int elaborate_lval_(Entity*ent, ScopeBase*scope, bool, ExpName*suffix);
      const VType* probe_prefix_type_(Entity*ent, ScopeBase*scope) const;
      const VType* probe_prefixed_type_(Entity*ent, ScopeBase*scope) const;

      int emit_as_prefix_(ostream&out, Entity*ent, ScopeBase*scope);

	// There are some workarounds required for constant arrays/records, as
	// they are currently emitted as flat localparams (without any type
	// information). The following workarounds adjust the access indices
	// to select appropriate parts of the localparam.
      bool try_workarounds_(ostream&out, Entity*ent, ScopeBase*scope,
                            list<index_t*>&indices, int&data_size);

      bool check_const_array_workaround_(const VTypeArray*arr, ScopeBase*scope,
                                         list<index_t*>&indices, int&data_size) const;

      bool check_const_record_workaround_(const VTypeRecord*rec, ScopeBase*scope,
                                          list<index_t*>&indices, int&data_size) const;

      int emit_workaround_(ostream&out, Entity*ent, ScopeBase*scope,
                           const list<index_t*>&indices, int field_size);

    private:
      std::auto_ptr<ExpName> prefix_;
      perm_string name_;
      Expression*index_;
      Expression*lsb_;
};

class ExpNameALL : public ExpName {

    public:
      ExpNameALL() : ExpName(perm_string()) { }

    public:
      int elaborate_lval(Entity*ent, ScopeBase*scope, bool);
      const VType* probe_type(Entity*ent, ScopeBase*scope) const;
      void dump(ostream&out, int indent =0) const;
};

class ExpRelation : public ExpBinary {

    public:
      enum fun_t { EQ, LT, GT, NEQ, LE, GE };

      inline fun_t relation_fun(void) const { return fun_; }

    public:
      ExpRelation(ExpRelation::fun_t ty, Expression*op1, Expression*op2);
      ~ExpRelation();

      Expression*clone() const {
          return new ExpRelation(fun_, peek_operand1()->clone(), peek_operand2()->clone());
      }

      const VType* probe_type(Entity*ent, ScopeBase*scope) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;

    private:
      fun_t fun_;
};

class ExpShift : public ExpBinary {
    public:
      enum shift_t { SRL, SLL, SRA, SLA, ROL, ROR };

    public:
      ExpShift(ExpShift::shift_t op, Expression*op1, Expression*op2);

      Expression*clone() const {
          return new ExpShift(shift_, peek_operand1()->clone(), peek_operand2()->clone());
      }

      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      virtual bool evaluate(ScopeBase*scope, int64_t&val) const;
      void dump(ostream&out, int indent = 0) const;

    private:
      shift_t shift_;
};

class ExpString : public Expression {

    public:
      explicit ExpString(const char*);
      ExpString(const ExpString&other) : Expression(), value_(other.value_) {}
      ~ExpString();

      Expression*clone() const { return new ExpString(*this); }

      const VType*fit_type(Entity*ent, ScopeBase*scope, const VTypeArray*atype) const;
      int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      bool is_primary(void) const;
      void dump(ostream&out, int indent = 0) const;
      const std::vector<char>& get_value() const { return value_; }

    private:
      int emit_as_array_(ostream&out, Entity*ent, ScopeBase*scope, const VTypeArray*arr);

    private:
      std::vector<char> value_;
};

class ExpUAbs : public ExpUnary {

    public:
      ExpUAbs(Expression*op1);
      ~ExpUAbs();

      Expression*clone() const { return new ExpUAbs(peek_operand()->clone()); }

      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;
};

class ExpUNot : public ExpUnary {

    public:
      ExpUNot(Expression*op1);
      ~ExpUNot();

      Expression*clone() const { return new ExpUNot(peek_operand()->clone()); }

      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;
};

/*
 * Class that wraps other expressions to cast them to other types.
 */
class ExpCast : public Expression {

    public:
      ExpCast(Expression*base, const VType*type);
      ~ExpCast();

      Expression*clone() const { return new ExpCast(base_->clone(), type_->clone()); }

      inline int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*) {
            return base_->elaborate_expr(ent, scope, type_);
      }
      void write_to_stream(std::ostream&fd) const;
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;
      void visit(ExprVisitor& func);

    private:
      Expression*base_;
      const VType*type_;
};

/*
 * Class that handles 'new' statement. VHDL is not capable of dynamic memory
 * allocation, but it is useful for emitting some cases.
 */
class ExpNew : public Expression {

    public:
      ExpNew(Expression*size);
      ~ExpNew();

      Expression*clone() const { return new ExpNew(size_->clone()); }

      // There is no 'new' in VHDL - do not emit anything
      void write_to_stream(std::ostream&) const {};
      int emit(ostream&out, Entity*ent, ScopeBase*scope);
      void dump(ostream&out, int indent = 0) const;
      void visit(ExprVisitor& func);

    private:
      Expression*size_;
};

class ExpTime : public Expression {
    public:
        typedef enum { FS, PS, NS, US, MS, S } timeunit_t;

        ExpTime(uint64_t amount, timeunit_t unit);

        Expression*clone() const { return new ExpTime(amount_, unit_); }

        int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
        void write_to_stream(std::ostream&) const;
        int emit(ostream&out, Entity*ent, ScopeBase*scope);
        bool evaluate(ScopeBase*scope, int64_t&val) const;
        bool evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const;
        void dump(ostream&out, int indent = 0) const;

    private:
        // Returns the time value expressed in femtoseconds
        double to_fs() const;
        uint64_t amount_;
        timeunit_t unit_;
};

#endif /* IVL_expression_H */
