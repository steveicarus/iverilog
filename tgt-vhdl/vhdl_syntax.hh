/*
 *  VHDL abstract syntax elements.
 *
 *  Copyright (C) 2008-2021  Nick Gasson (nick@nickg.me.uk)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef INC_VHDL_SYNTAX_HH
#define INC_VHDL_SYNTAX_HH

#include <inttypes.h>
#include <set>
#include <cassert>
#include "vhdl_element.hh"
#include "vhdl_type.hh"

class vhdl_scope;
class vhdl_entity;
class vhdl_arch;
class vhdl_var_ref;

typedef std::set<vhdl_var_ref*> vhdl_var_set_t;

class vhdl_expr : public vhdl_element {
public:
   explicit vhdl_expr(const vhdl_type* type, bool isconst=false)
      : type_(type), isconst_(isconst) {}
   virtual ~vhdl_expr();

   const vhdl_type *get_type() const { return type_; }
   bool constant() const { return isconst_; }

   vhdl_expr *cast(const vhdl_type *to);
   virtual vhdl_expr *resize(int newwidth);
   virtual vhdl_expr *to_boolean();
   virtual vhdl_expr *to_integer();
   virtual vhdl_expr *to_std_logic();
   virtual vhdl_expr *to_std_ulogic();
   virtual vhdl_expr *to_vector(vhdl_type_name_t name, int w);
   virtual vhdl_expr *to_string();
   virtual void find_vars(vhdl_var_set_t&) {}

protected:
   static void open_parens(std::ostream& of);
   static void close_parens(std::ostream& of);
   static int paren_levels;

   const vhdl_type *type_;
   bool isconst_;
};


/*
 * A scalar or array variable reference.
 */
class vhdl_var_ref : public vhdl_expr {
public:
   vhdl_var_ref(const std::string& name, const vhdl_type *type,
                vhdl_expr *slice = NULL)
      : vhdl_expr(type), name_(name), slice_(slice), slice_width_(0) {}
   ~vhdl_var_ref();

   void emit(std::ostream &of, int level) const;
   const std::string &get_name() const { return name_; }
   void set_name(const std::string &name) { name_ = name; }
   void set_slice(vhdl_expr *s, int w=0);
   void find_vars(vhdl_var_set_t& read);
private:
   std::string name_;
   vhdl_expr *slice_;
   unsigned slice_width_;
};

enum vhdl_binop_t {
   VHDL_BINOP_AND = 0,
   VHDL_BINOP_OR,
   VHDL_BINOP_EQ,
   VHDL_BINOP_NEQ,
   VHDL_BINOP_ADD,
   VHDL_BINOP_SUB,
   VHDL_BINOP_MULT,
   VHDL_BINOP_LT,
   VHDL_BINOP_GT,
   VHDL_BINOP_LEQ,
   VHDL_BINOP_GEQ,
   VHDL_BINOP_SL,
   VHDL_BINOP_SR,
   VHDL_BINOP_XOR,
   VHDL_BINOP_CONCAT,
   VHDL_BINOP_NAND,
   VHDL_BINOP_NOR,
   VHDL_BINOP_XNOR,
   VHDL_BINOP_DIV,
   VHDL_BINOP_MOD,
   VHDL_BINOP_POWER,
   VHDL_BINOP_SRA
};

/*
 * A binary expression contains a list of operands rather
 * than just two: this is to model n-input gates and the
 * like. A second constructor is provided to handle the
 * common case of a true binary expression.
 */
class vhdl_binop_expr : public vhdl_expr {
public:
   vhdl_binop_expr(vhdl_binop_t op, const vhdl_type *type)
      : vhdl_expr(type), op_(op) {}
   vhdl_binop_expr(vhdl_expr *left, vhdl_binop_t op,
                   vhdl_expr *right, const vhdl_type *type);
   ~vhdl_binop_expr();

   void add_expr(vhdl_expr *e);
   void add_expr_front(vhdl_expr *e);
   void emit(std::ostream &of, int level) const;
   void find_vars(vhdl_var_set_t& read);
private:
   std::list<vhdl_expr*> operands_;
   vhdl_binop_t op_;
};


enum vhdl_unaryop_t {
   VHDL_UNARYOP_NOT,
   VHDL_UNARYOP_NEG
};

class vhdl_unaryop_expr : public vhdl_expr {
public:
   vhdl_unaryop_expr(vhdl_unaryop_t op, vhdl_expr *operand,
                     vhdl_type *type)
      : vhdl_expr(type), op_(op), operand_(operand) {}
   ~vhdl_unaryop_expr();

   void emit(std::ostream &of, int level) const;
   void find_vars(vhdl_var_set_t& read);
private:
   vhdl_unaryop_t op_;
   vhdl_expr *operand_;
};


/*
 * An expression like (0 => '1', 2 => '0', others => 'Z')
 */
class vhdl_bit_spec_expr : public vhdl_expr {
public:
   vhdl_bit_spec_expr(vhdl_type *type, vhdl_expr *others)
      : vhdl_expr(type), others_(others) {}
   ~vhdl_bit_spec_expr();

   void add_bit(int bit, vhdl_expr *e);
   void emit(std::ostream &of, int level) const;
private:
   vhdl_expr *others_;
   struct bit_map {
      int bit;
      vhdl_expr *e;
   };
   std::list<bit_map> bits_;
};


class vhdl_const_string : public vhdl_expr {
public:
   explicit vhdl_const_string(const std::string& value)
      : vhdl_expr(vhdl_type::string(), true), value_(value) {}

   void emit(std::ostream &of, int level) const;
private:
   std::string value_;
};

class vhdl_const_bits : public vhdl_expr {
public:
   vhdl_const_bits(const char *value, int width, bool issigned,
                   bool qualify=false);
   void emit(std::ostream &of, int level) const;
   const std::string &get_value() const { return value_; }
   vhdl_expr *to_integer();
   vhdl_expr *to_std_logic();
   vhdl_expr *to_vector(vhdl_type_name_t name, int w);
   vhdl_expr *resize(int w);
private:
   int64_t bits_to_int() const;
   char sign_bit() const;
   bool has_meta_bits() const;

   std::string value_;
   bool qualified_, signed_;
};

class vhdl_const_bit : public vhdl_expr {
public:
   explicit vhdl_const_bit(char bit)
      : vhdl_expr(vhdl_type::std_logic(), true), bit_(bit) {}
   void emit(std::ostream &of, int level) const;
   vhdl_expr *to_boolean();
   vhdl_expr *to_integer();
   vhdl_expr *to_vector(vhdl_type_name_t name, int w);
   vhdl_expr *to_std_ulogic();
private:
   char bit_;
};

enum time_unit_t {
   TIME_UNIT_PS,
   TIME_UNIT_NS,
   TIME_UNIT_US,
   TIME_UNIT_MS
};

class vhdl_const_time : public vhdl_expr {
public:
   explicit vhdl_const_time(uint64_t value, time_unit_t units = TIME_UNIT_NS)
      : vhdl_expr(vhdl_type::time(), true), value_(value), units_(units) {}
   void emit(std::ostream &of, int level) const;
private:
   uint64_t value_;
   time_unit_t units_;
};

class vhdl_const_int : public vhdl_expr {
public:
   explicit vhdl_const_int(int64_t value)
      : vhdl_expr(vhdl_type::integer(), true), value_(value) {}
   void emit(std::ostream &of, int level) const;
   vhdl_expr *to_vector(vhdl_type_name_t name, int w);
private:
   int64_t value_;
};

class vhdl_const_bool : public vhdl_expr {
public:
   explicit vhdl_const_bool(bool value)
      : vhdl_expr(vhdl_type::boolean(), true), value_(value) {}
   void emit(std::ostream &of, int level) const;
private:
   bool value_;
};

class vhdl_expr_list : public vhdl_element {
public:
   ~vhdl_expr_list();

   void emit(std::ostream &of, int level) const;
   bool empty() const { return exprs_.empty(); }
   void add_expr(vhdl_expr *e);
   void find_vars(vhdl_var_set_t& read);
private:
   std::list<vhdl_expr*> exprs_;
};


/*
 * A function call within an expression.
 */
class vhdl_fcall : public vhdl_expr {
public:
   vhdl_fcall(const std::string& name, const vhdl_type *rtype)
      : vhdl_expr(rtype), name_(name) {};
   ~vhdl_fcall() {}

   void add_expr(vhdl_expr *e) { exprs_.add_expr(e); }
   void emit(std::ostream &of, int level) const;
   void find_vars(vhdl_var_set_t& read);
private:
   std::string name_;
   vhdl_expr_list exprs_;
};

/*
 * A concurrent statement appears in architecture bodies/
 */
class vhdl_conc_stmt : public vhdl_element {
public:
   virtual ~vhdl_conc_stmt() {}
};

typedef std::list<vhdl_conc_stmt*> conc_stmt_list_t;

/*
 * A '<value> when <cond>' clause that appears in several
 * statement types.
 */
struct when_part_t {
   vhdl_expr *value, *cond, *delay;
};
typedef std::list<when_part_t> when_list_t;


/*
 * A concurrent signal assignment (i.e. not part of a process).
 * Can have any number of `when' clauses, in which case the original
 * rhs becomes the `else' part.
 */
class vhdl_cassign_stmt : public vhdl_conc_stmt {
public:
   vhdl_cassign_stmt(vhdl_var_ref *lhs, vhdl_expr *rhs)
      : lhs_(lhs), rhs_(rhs), after_(NULL) {}
   ~vhdl_cassign_stmt();

   void emit(std::ostream &of, int level) const;
   void add_condition(vhdl_expr *value, vhdl_expr *cond);
   void set_after(vhdl_expr *a) { after_ = a; }
private:
   vhdl_var_ref *lhs_;
   vhdl_expr *rhs_;
   vhdl_expr *after_;
   when_list_t whens_;
};


class vhdl_with_select_stmt : public vhdl_conc_stmt {
public:
   vhdl_with_select_stmt(vhdl_expr *test, vhdl_var_ref *out)
      : test_(test), out_(out), others_(NULL) {}
   ~vhdl_with_select_stmt();

   void emit(std::ostream &of, int level) const;
   void add_condition(vhdl_expr *value, vhdl_expr *cond, vhdl_expr *delay=NULL);
   void add_default(vhdl_expr* value);
private:
   vhdl_expr *test_;
   vhdl_var_ref *out_;
   when_list_t whens_;
   vhdl_expr* others_;
};


/*
 * Any sequential statement in a process.
 */
class vhdl_seq_stmt : public vhdl_element {
public:
   virtual ~vhdl_seq_stmt() {}

   // Find all the variables that are read or written in the
   // expressions within this statement
   // This is used to clean up the VHDL output
   virtual void find_vars(vhdl_var_set_t& read,
                          vhdl_var_set_t& write) = 0;
};


/*
 * A list of sequential statements. For example inside a
 * process, loop, or if statement.
 */
class stmt_container {
public:
   ~stmt_container();

   void add_stmt(vhdl_seq_stmt *stmt);
   void move_stmts_from(stmt_container *other);
   void emit(std::ostream &of, int level, bool newline=true) const;
   bool empty() const { return stmts_.empty(); }
   void find_vars(vhdl_var_set_t& read, vhdl_var_set_t& write);

   typedef std::list<vhdl_seq_stmt*> stmt_list_t;
   stmt_list_t &get_stmts() { return stmts_; }
private:
   stmt_list_t stmts_;
};


/*
 * Shared between blocking and non-blocking assignment.
 */
class vhdl_abstract_assign_stmt : public vhdl_seq_stmt {
public:
   vhdl_abstract_assign_stmt(vhdl_var_ref *lhs, vhdl_expr *rhs)
      : lhs_(lhs), rhs_(rhs), after_(NULL) {}
   virtual ~vhdl_abstract_assign_stmt();

   void set_after(vhdl_expr *after) { after_ = after; }
   void find_vars(vhdl_var_set_t& read, vhdl_var_set_t& write);
protected:
   vhdl_var_ref *lhs_;
   vhdl_expr *rhs_, *after_;
};


/*
 * Similar to Verilog non-blocking assignment, except the LHS
 * must be a signal not a variable.
 */
class vhdl_nbassign_stmt : public vhdl_abstract_assign_stmt {
public:
   vhdl_nbassign_stmt(vhdl_var_ref *lhs, vhdl_expr *rhs)
      : vhdl_abstract_assign_stmt(lhs, rhs) {}

   void emit(std::ostream &of, int level) const;
};


class vhdl_assign_stmt : public vhdl_abstract_assign_stmt {
public:
   vhdl_assign_stmt(vhdl_var_ref *lhs, vhdl_expr *rhs)
      : vhdl_abstract_assign_stmt(lhs, rhs) {}

   void emit(std::ostream &of, int level) const;
};


enum vhdl_wait_type_t {
   VHDL_WAIT_INDEF,  // Suspend indefinitely
   VHDL_WAIT_FOR,    // Wait for a constant amount of time
   VHDL_WAIT_FOR0,   // Special wait for zero time
   VHDL_WAIT_UNTIL,  // Wait on an expression
   VHDL_WAIT_ON      // Wait on a sensitivity list
};

/*
 * Delay simulation indefinitely, until an event, or for a
 * specified time.
 */
class vhdl_wait_stmt : public vhdl_seq_stmt {
public:
   vhdl_wait_stmt(vhdl_wait_type_t type = VHDL_WAIT_INDEF,
                  vhdl_expr *expr = NULL)
      : type_(type), expr_(expr) {}
   ~vhdl_wait_stmt();

   void emit(std::ostream &of, int level) const;
   void add_sensitivity(const std::string &s) { sensitivity_.push_back(s); }
   vhdl_wait_type_t get_type() const { return type_; }
   void find_vars(vhdl_var_set_t& read, vhdl_var_set_t& write);
private:
   vhdl_wait_type_t type_;
   vhdl_expr *expr_;
   string_list_t sensitivity_;
};


class vhdl_null_stmt : public vhdl_seq_stmt {
public:
   void emit(std::ostream &of, int level) const;
   void find_vars(vhdl_var_set_t&, vhdl_var_set_t&) {}
};


enum vhdl_severity_t {
   SEVERITY_NOTE,
   SEVERITY_WARNING,
   SEVERITY_ERROR,
   SEVERITY_FAILURE
};

class vhdl_report_stmt : public vhdl_seq_stmt {
public:
   explicit vhdl_report_stmt(vhdl_expr *text,
                             vhdl_severity_t severity = SEVERITY_NOTE);
   virtual ~vhdl_report_stmt() {}

   virtual void emit(std::ostream& of, int level) const;
   void find_vars(vhdl_var_set_t& read, vhdl_var_set_t& write);
private:
   vhdl_severity_t severity_;
   vhdl_expr *text_;
};


class vhdl_assert_stmt : public vhdl_report_stmt {
public:
   explicit vhdl_assert_stmt(const char *reason);

   void emit(std::ostream &of, int level) const;
};


class vhdl_if_stmt : public vhdl_seq_stmt {
public:
   explicit vhdl_if_stmt(vhdl_expr *test);
   ~vhdl_if_stmt();

   stmt_container *get_then_container() { return &then_part_; }
   stmt_container *get_else_container() { return &else_part_; }
   stmt_container *add_elsif(vhdl_expr *test);
   void emit(std::ostream &of, int level) const;
   void find_vars(vhdl_var_set_t& read, vhdl_var_set_t& write);
private:
   struct elsif {
      vhdl_expr *test;
      stmt_container *container;
   };

   vhdl_expr *test_;
   stmt_container then_part_, else_part_;
   std::list<elsif> elsif_parts_;
};


/*
 * A single branch in a case statement consisting of an
 * expression part and a statement container.
 */
class vhdl_case_branch : public vhdl_element {
   friend class vhdl_case_stmt;
public:
   explicit vhdl_case_branch(vhdl_expr *when) : when_(when) {}
   ~vhdl_case_branch();

   stmt_container *get_container() { return &stmts_; }
   void emit(std::ostream &of, int level) const;
private:
   vhdl_expr *when_;
   stmt_container stmts_;
};

typedef std::list<vhdl_case_branch*> case_branch_list_t;

class vhdl_case_stmt : public vhdl_seq_stmt {
public:
   explicit vhdl_case_stmt(vhdl_expr *test) : test_(test) {}
   ~vhdl_case_stmt();

   void add_branch(vhdl_case_branch *b) { branches_.push_back(b); }
   void emit(std::ostream &of, int level) const;
   void find_vars(vhdl_var_set_t& read, vhdl_var_set_t& write);
private:
   vhdl_expr *test_;
   case_branch_list_t branches_;
};


class vhdl_loop_stmt : public vhdl_seq_stmt {
public:
   virtual ~vhdl_loop_stmt() {}

   stmt_container *get_container() { return &stmts_; }
   void emit(std::ostream &of, int level) const;
   virtual void find_vars(vhdl_var_set_t& read,
                          vhdl_var_set_t& write);
private:
   stmt_container stmts_;
};


class vhdl_while_stmt : public vhdl_loop_stmt {
public:
   explicit vhdl_while_stmt(vhdl_expr *test) : test_(test) {}
   ~vhdl_while_stmt();

   void emit(std::ostream &of, int level) const;
   void find_vars(vhdl_var_set_t& read, vhdl_var_set_t& write);
private:
   vhdl_expr *test_;
};


class vhdl_for_stmt : public vhdl_loop_stmt {
public:
   vhdl_for_stmt(const char *lname, vhdl_expr *from, vhdl_expr *to)
      : lname_(lname), from_(from), to_(to) {}
   ~vhdl_for_stmt();

   void emit(std::ostream &of, int level) const;
   void find_vars(vhdl_var_set_t& read, vhdl_var_set_t& write);
private:
   const char *lname_;
   vhdl_expr *from_, *to_;
};


/*
 * A procedure call. Which is a statement, unlike a function
 * call which is an expression.
 */
class vhdl_pcall_stmt : public vhdl_seq_stmt {
public:
   explicit vhdl_pcall_stmt(const char *name) : name_(name) {}

   void emit(std::ostream &of, int level) const;
   void add_expr(vhdl_expr *e) { exprs_.add_expr(e); }
   void find_vars(vhdl_var_set_t& read, vhdl_var_set_t& write);
private:
   std::string name_;
   vhdl_expr_list exprs_;
};


/*
 * A declaration of some sort (variable, component, etc.).
 * Declarations have names, which is the identifier of the variable,
 * constant, etc. not the type.
 */
class vhdl_decl : public vhdl_element {
public:
   explicit vhdl_decl(const std::string& name, const vhdl_type *type = NULL,
                      vhdl_expr *initial = NULL)
      : name_(name), type_(type), initial_(initial),
        has_initial_(initial != NULL) {}
   virtual ~vhdl_decl();

   const std::string &get_name() const { return name_; }
   const vhdl_type *get_type() const;
   void set_type(vhdl_type *t) { type_ = t; }
   void set_initial(vhdl_expr *initial);
   bool has_initial() const { return has_initial_; }

   // Return a new reference to this declaration
   vhdl_var_ref* make_ref() const;

   // The different sorts of assignment statement
   // ASSIGN_CONST is used to generate a variable to shadow a
   // constant that cannot be assigned to (e.g. a function parameter)
   enum assign_type_t { ASSIGN_BLOCK, ASSIGN_NONBLOCK, ASSIGN_CONST };

   // Get the sort of assignment statement to generate for
   // assignments to this declaration
   // For some sorts of declarations it doesn't make sense
   // to assign to it so calling assignment_type just raises
   // an assertion failure
   virtual assign_type_t assignment_type() const { assert(false);
                                                   return ASSIGN_BLOCK; }

   // True if this declaration can be read from
   virtual bool is_readable() const { return true; }

   // Modify this declaration so it can be read from
   // This does nothing for most declaration types
   virtual void ensure_readable() {}
protected:
   std::string name_;
   const vhdl_type *type_;
   vhdl_expr *initial_;
   bool has_initial_;
};

typedef std::list<vhdl_decl*> decl_list_t;


/*
 * A forward declaration of a component. At the moment it is assumed
 * that components declarations will only ever be for entities
 * generated by this code generator. This is enforced by making the
 * constructor private (use component_decl_for instead).
 */
class vhdl_component_decl : public vhdl_decl {
public:
   static vhdl_component_decl *component_decl_for(vhdl_entity *ent);

   void emit(std::ostream &of, int level) const;
private:
   explicit vhdl_component_decl(const char *name);

   decl_list_t ports_;
};


class vhdl_type_decl : public vhdl_decl {
public:
   vhdl_type_decl(const std::string& name, const vhdl_type *base)
      : vhdl_decl(name, base) {}
   void emit(std::ostream &of, int level) const;
};

/*
 * A variable declaration inside a process (although this isn't
 * enforced here).
 */
class vhdl_var_decl : public vhdl_decl {
public:
   vhdl_var_decl(const std::string& name, const vhdl_type *type)
      : vhdl_decl(name, type) {}
   void emit(std::ostream &of, int level) const;
   assign_type_t assignment_type() const { return ASSIGN_BLOCK; }
};


/*
 * A signal declaration in architecture.
 */
class vhdl_signal_decl : public vhdl_decl {
public:
   vhdl_signal_decl(const std::string& name, const vhdl_type* type)
      : vhdl_decl(name, type) {}
   virtual void emit(std::ostream &of, int level) const;
   assign_type_t assignment_type() const { return ASSIGN_NONBLOCK; }
};


/*
 * A parameter to a function.
 */
class vhdl_param_decl : public vhdl_decl {
public:
   vhdl_param_decl(const char *name, vhdl_type *type)
      : vhdl_decl(name, type) {}
   void emit(std::ostream &of, int level) const;
   assign_type_t assignment_type() const { return ASSIGN_CONST; }
};

enum vhdl_port_mode_t {
   VHDL_PORT_IN,
   VHDL_PORT_OUT,
   VHDL_PORT_INOUT,
   VHDL_PORT_BUFFER
};

/*
 * A port declaration is like a signal declaration except
 * it has a direction and appears in the entity rather than
 * the architecture.
 */
class vhdl_port_decl : public vhdl_decl {
public:
   vhdl_port_decl(const char *name, vhdl_type *type,
                  vhdl_port_mode_t mode)
      : vhdl_decl(name, type), mode_(mode) {}

   void emit(std::ostream &of, int level) const;
   vhdl_port_mode_t get_mode() const { return mode_; }
   void set_mode(vhdl_port_mode_t m) { mode_ = m; }
   assign_type_t assignment_type() const { return ASSIGN_NONBLOCK; }
   void ensure_readable();
   bool is_readable() const;
private:
   vhdl_port_mode_t mode_;
};

/*
 * A mapping from port name to an expression.
 */
struct port_map_t {
   std::string name;
   vhdl_expr *expr;
};

typedef std::list<port_map_t> port_map_list_t;

/*
 * Instantiation of component. This is really only a placeholder
 * at the moment until the port mappings are worked out.
 */
class vhdl_comp_inst : public vhdl_conc_stmt {
public:
   vhdl_comp_inst(const char *inst_name, const char *comp_name);
   ~vhdl_comp_inst();

   void emit(std::ostream &of, int level) const;
   void map_port(const std::string& name, vhdl_expr *expr);

   const std::string &get_comp_name() const { return comp_name_; }
   const std::string &get_inst_name() const { return inst_name_; }
private:
   std::string comp_name_, inst_name_;
   port_map_list_t mapping_;
};


/*
 * Contains a list of declarations in a hierarchy.
 * A scope can be `initializing' where assignments automatically
 * create initial values for declarations.
 */
class vhdl_scope {
public:
   vhdl_scope();
   ~vhdl_scope();

   void add_decl(vhdl_decl *decl);
   void add_forward_decl(vhdl_decl *decl);
   vhdl_decl *get_decl(const std::string &name) const;
   bool have_declared(const std::string &name) const;
   bool name_collides(const std::string& name) const;
   bool contained_within(const vhdl_scope *other) const;
   vhdl_scope *get_parent() const;

   bool empty() const { return decls_.empty(); }
   const decl_list_t &get_decls() const { return decls_; }
   void set_parent(vhdl_scope *p) { parent_ = p; }

   bool initializing() const { return init_; }
   void set_initializing(bool i);
   bool hoisted_initialiser() const;
   void hoisted_initialiser(bool h);

   void set_allow_signal_assignment(bool b) { sig_assign_ = b; }
   bool allow_signal_assignment() const { return sig_assign_; }
private:
   decl_list_t decls_;
   vhdl_scope *parent_;
   bool init_, sig_assign_;
   bool hoisted_init_;
};


/*
 * Any sort of procedural element: process, function, or
 * procedure. Roughly these map onto Verilog's processes,
 * functions, and tasks.
 */
class vhdl_procedural {
public:
   vhdl_procedural() : contains_wait_stmt_(false) {}
   virtual ~vhdl_procedural() {}

   virtual stmt_container *get_container() { return &stmts_; }
   virtual vhdl_scope *get_scope() { return &scope_; }

   void added_wait_stmt() { contains_wait_stmt_ = true; }
   bool contains_wait_stmt() const { return contains_wait_stmt_; }

   // Managing set of blocking assignment targets in this block
   void add_blocking_target(vhdl_var_ref* ref);
   bool is_blocking_target(vhdl_var_ref* ref) const;

protected:
   stmt_container stmts_;
   vhdl_scope scope_;

   // If this is true then the body contains a `wait' statement
   // embedded in it somewhere
   // If this is the case then we can't use a sensitivity list for
   // the process
   bool contains_wait_stmt_;

   // The set of variable we have performed a blocking
   // assignment to
   std::set<std::string> blocking_targets_;
};


class vhdl_function : public vhdl_decl, public vhdl_procedural {
   friend class vhdl_forward_fdecl;
public:
   vhdl_function(const char *name, vhdl_type *ret_type);

   virtual void emit(std::ostream &of, int level) const;
   vhdl_scope *get_scope() { return &variables_; }
   void add_param(vhdl_param_decl *p) { scope_.add_decl(p); }
private:
   vhdl_scope variables_;
};

class vhdl_forward_fdecl : public vhdl_decl {
public:
   explicit vhdl_forward_fdecl(const vhdl_function *f)
      : vhdl_decl((f->get_name() + "_Forward").c_str()), f_(f) {}

   void emit(std::ostream &of, int level) const;
private:
   const vhdl_function *f_;
};


class vhdl_process : public vhdl_conc_stmt, public vhdl_procedural {
public:
   explicit vhdl_process(const char *name = "") : name_(name) {}

   void emit(std::ostream &of, int level) const;
   void add_sensitivity(const std::string &name);
private:
   std::string name_;
   string_list_t sens_;
};


/*
 * An architecture which implements an entity.
 */
class vhdl_arch : public vhdl_element {
public:
   vhdl_arch(const std::string& entity, const std::string& name)
      : name_(name), entity_(entity) {}
   virtual ~vhdl_arch();

   void emit(std::ostream &of, int level=0) const;
   void add_stmt(vhdl_process *proc);
   void add_stmt(vhdl_conc_stmt *stmt);
   vhdl_scope *get_scope() { return &scope_; }
private:
   conc_stmt_list_t stmts_;
   vhdl_scope scope_;
   std::string name_, entity_;
};

/*
 * An entity defines the ports, parameters, etc. of a module. Each
 * entity is associated with a single architecture (although
 * technically this need not be the case).  Entities are `derived'
 * from instantiations of Verilog module scopes in the hierarchy.
 */
class vhdl_entity : public vhdl_element {
public:
   vhdl_entity(const std::string& name, vhdl_arch *arch, int depth=0);
   virtual ~vhdl_entity();

   void emit(std::ostream &of, int level=0) const;
   void add_port(vhdl_port_decl *decl);
   vhdl_arch *get_arch() const { return arch_; }
   const std::string &get_name() const { return name_; }

   vhdl_scope *get_scope() { return &ports_; }

   void set_time_units(int units, int precision);
   friend vhdl_const_time* scale_time(const vhdl_entity* ent, uint64_t t);

   // Each entity has an associated depth which is how deep in
   // the Verilog module hierarchy it was found
   // This is used to limit the maximum depth of modules emitted
   const int depth;
private:
   std::string name_;
   vhdl_arch *arch_;  // Entity may only have a single architecture
   vhdl_scope ports_;

   // Entities have an associated VHDL time unit
   // This is used to implement the Verilog timescale directive
   time_unit_t time_unit_;
};

typedef std::list<vhdl_entity*> entity_list_t;

#endif
