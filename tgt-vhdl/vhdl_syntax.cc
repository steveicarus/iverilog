/*
 *  VHDL abstract syntax elements.
 *
 *  Copyright (C) 2008-2013  Nick Gasson (nick@nickg.me.uk)
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

#include "vhdl_syntax.hh"
#include "vhdl_helper.hh"

#include <cassert>
#include <cstring>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <iomanip>

using namespace std;

vhdl_scope::vhdl_scope()
   : parent_(NULL), init_(false), sig_assign_(true),
     hoisted_init_(false)
{

}

vhdl_scope::~vhdl_scope()
{

}

void vhdl_scope::set_initializing(bool i)
{
   init_ = i;
   if (parent_)
      parent_->set_initializing(i);
}

void vhdl_scope::add_decl(vhdl_decl *decl)
{
   decls_.push_back(decl);
}

void vhdl_scope::add_forward_decl(vhdl_decl *decl)
{
   decls_.push_front(decl);
}

vhdl_decl *vhdl_scope::get_decl(const std::string &name) const
{
   decl_list_t::const_iterator it;
   for (it = decls_.begin(); it != decls_.end(); ++it) {
      if (strcasecmp((*it)->get_name().c_str(), name.c_str()) == 0)
         return *it;
   }

   return parent_ ? parent_->get_decl(name) : NULL;
}

bool vhdl_scope::have_declared(const std::string &name) const
{
   return get_decl(name) != NULL;
}

// True if `name' differs in all but case from another declaration
bool vhdl_scope::name_collides(const string& name) const
{
   const vhdl_decl* decl = get_decl(name);
   if (decl)
      return strcasecmp(decl->get_name().c_str(), name.c_str()) == 0;
   else
      return false;
}

bool vhdl_scope::contained_within(const vhdl_scope *other) const
{
   if (this == other)
      return true;
   else if (NULL == parent_)
      return false;
   else
      return parent_->contained_within(other);
}

vhdl_scope *vhdl_scope::get_parent() const
{
   assert(parent_);
   return parent_;
}

bool vhdl_scope::hoisted_initialiser() const
{
   return hoisted_init_;
}

void vhdl_scope::hoisted_initialiser(bool h)
{
   hoisted_init_ = h;
}

vhdl_entity::vhdl_entity(const string& name, vhdl_arch *arch, int depth__)
   :  depth(depth__), name_(name), arch_(arch),
      time_unit_(TIME_UNIT_NS)
{
   arch->get_scope()->set_parent(&ports_);
}

vhdl_entity::~vhdl_entity()
{

}

void vhdl_entity::add_port(vhdl_port_decl *decl)
{
   ports_.add_decl(decl);
}

void vhdl_entity::emit(std::ostream &of, int level) const
{
   // Pretty much every design will use std_logic so we
   // might as well include it by default
   of << "library ieee;" << std::endl;
   of << "use ieee.std_logic_1164.all;" << std::endl;
   of << "use ieee.numeric_std.all;" << std::endl;
   of << std::endl;

   emit_comment(of, level);
   of << "entity " << name_ << " is";

   if (!ports_.empty()) {
      newline(of, indent(level));
      of << "port (";
      emit_children<vhdl_decl>(of, ports_.get_decls(), indent(level), ";");
      of << ");";
   }

   newline(of, level);
   of << "end entity; ";
   blank_line(of, level);  // Extra blank line after entities
   arch_->emit(of, level);
}

// Return a VHDL time constant scaled to the correct time scale
// for this entity
vhdl_const_time* scale_time(const vhdl_entity* ent, uint64_t t)
{
   return new vhdl_const_time(t, ent->time_unit_);
}

// Work out the best VHDL units to use given the Verilog timescale
void vhdl_entity::set_time_units(int units, int precision)
{
   int vhdl_units = std::min(units, precision);

   if (vhdl_units >= -3)
      time_unit_ = TIME_UNIT_MS;
   else if (vhdl_units >= -6)
      time_unit_ = TIME_UNIT_US;
   else if (vhdl_units >= -9)
      time_unit_ = TIME_UNIT_NS;
   else
      time_unit_ = TIME_UNIT_PS;
}

vhdl_arch::~vhdl_arch()
{

}

void vhdl_arch::add_stmt(vhdl_process *proc)
{
   proc->get_scope()->set_parent(&scope_);
   stmts_.push_back(proc);
}

void vhdl_arch::add_stmt(vhdl_conc_stmt *stmt)
{
   stmts_.push_back(stmt);
}

void vhdl_arch::emit(std::ostream &of, int level) const
{
   emit_comment(of, level);
   of << "architecture " << name_ << " of " << entity_;
   of << " is";
   emit_children<vhdl_decl>(of, scope_.get_decls(), level);
   of << "begin";
   emit_children<vhdl_conc_stmt>(of, stmts_, level);
   of << "end architecture;";
   blank_line(of, level);  // Extra blank line after architectures;
}

void vhdl_procedural::add_blocking_target(vhdl_var_ref* ref)
{
   blocking_targets_.insert(ref->get_name());
}

bool vhdl_procedural::is_blocking_target(vhdl_var_ref* ref) const
{
   return blocking_targets_.find(ref->get_name()) != blocking_targets_.end();
}

void vhdl_process::add_sensitivity(const std::string &name)
{
   sens_.push_back(name);
}

void vhdl_process::emit(std::ostream &of, int level) const
{
   // If there are no statements in the body, this process
   // can't possibly do anything, so don't bother to emit it
   if (stmts_.empty()) {
      of << "-- Removed one empty process";
      newline(of, level);
      return;
   }

   newline(of, level);
   emit_comment(of, level);
   if (! name_.empty())
      of << name_ << ": ";
   of << "process ";

   int num_sens = sens_.size();
   if (num_sens > 0) {
      of << "(";
      string_list_t::const_iterator it;
      for (it = sens_.begin(); it != sens_.end(); ++it) {
         of << *it;
         if (--num_sens > 0)
            of << ", ";
      }
      of << ") ";
   }

   of << "is";
   emit_children<vhdl_decl>(of, scope_.get_decls(), level);
   of << "begin";
   stmts_.emit(of, level);
   of << "end process;";
}

stmt_container::~stmt_container()
{

}

void stmt_container::add_stmt(vhdl_seq_stmt *stmt)
{
   // Add a statement at the end of the block
   stmts_.push_back(stmt);
}

/*
 * Move all the statements from one container into another.
 * This is useful, for example, if we want to wrap a container
 * in an `if' statement.
 */
void stmt_container::move_stmts_from(stmt_container *other)
{
   copy(other->stmts_.begin(), other->stmts_.end(), back_inserter(stmts_));
   other->stmts_.clear();
}

void stmt_container::find_vars(vhdl_var_set_t& read,
                               vhdl_var_set_t& write)
{
   // Iterate over each sub-statement and find all its
   // read/written variables

   for (stmt_list_t::const_iterator it = stmts_.begin();
        it != stmts_.end(); ++it)
      (*it)->find_vars(read, write);
}

void stmt_container::emit(std::ostream &of, int level, bool newline) const
{
   emit_children<vhdl_seq_stmt>(of, stmts_, level, "", newline);
}

vhdl_comp_inst::vhdl_comp_inst(const char *inst_name, const char *comp_name)
   : comp_name_(comp_name), inst_name_(inst_name)
{

}

vhdl_comp_inst::~vhdl_comp_inst()
{

}

void vhdl_comp_inst::map_port(const string& name, vhdl_expr *expr)
{
   port_map_t pmap = { name, expr };
   mapping_.push_back(pmap);
}

void vhdl_comp_inst::emit(std::ostream &of, int level) const
{
   newline(of, level);
   emit_comment(of, level);
   of << inst_name_ << ": " << comp_name_;

   // If there are no ports or generics we don't need to mention them...
   if (! mapping_.empty()) {
      newline(of, indent(level));
      of << "port map (";

      int sz = mapping_.size();
      port_map_list_t::const_iterator it;
      for (it = mapping_.begin(); it != mapping_.end(); ++it) {
         newline(of, indent(indent(level)));
         of << (*it).name << " => ";
         (*it).expr->emit(of, level);
         if (--sz > 0)
            of << ",";
      }
      newline(of, indent(level));
      of << ")";
   }

   of << ";";
}

vhdl_component_decl::vhdl_component_decl(const char *name)
   : vhdl_decl(name)
{

}

/*
 * Create a component declaration for the given entity.
 */
vhdl_component_decl *vhdl_component_decl::component_decl_for(vhdl_entity *ent)
{
   assert(ent != NULL);

   vhdl_component_decl *decl = new vhdl_component_decl
      (ent->get_name().c_str());

   decl->ports_ = ent->get_scope()->get_decls();

   return decl;
}

void vhdl_component_decl::emit(std::ostream &of, int level) const
{
   newline(of, level);
   emit_comment(of, level);
   of << "component " << name_ << " is";

   if (! ports_.empty()) {
      newline(of, indent(level));
      of << "port (";
      emit_children<vhdl_decl>(of, ports_, indent(level), ";");
      of << ");";
   }

   newline(of, level);
   of << "end component;";
}

vhdl_wait_stmt::~vhdl_wait_stmt()
{

}

void vhdl_wait_stmt::find_vars(vhdl_var_set_t& read,
                               vhdl_var_set_t&)
{
   if (expr_)
      expr_->find_vars(read);
}

void vhdl_wait_stmt::emit(std::ostream &of, int level) const
{
   of << "wait";

   switch (type_) {
   case VHDL_WAIT_INDEF:
      break;
   case VHDL_WAIT_FOR:
      assert(expr_);
      of << " for ";
      expr_->emit(of, level);
      break;
   case VHDL_WAIT_FOR0:
      of << " for 0 ns";
      break;
   case VHDL_WAIT_UNTIL:
      assert(expr_);
      of << " until ";
      expr_->emit(of, level);
      break;
   case VHDL_WAIT_ON:
      {
         of << " on ";
         string_list_t::const_iterator it = sensitivity_.begin();
         while (it != sensitivity_.end()) {
            of << *it;
            if (++it != sensitivity_.end())
               of << ", ";
         }
      }
      break;
   }

   of << ";";
   emit_comment(of, level, true);
}

vhdl_decl::~vhdl_decl()
{

}

// Make a reference object to this declaration
vhdl_var_ref* vhdl_decl::make_ref() const
{
   return new vhdl_var_ref(name_, type_);
}

const vhdl_type *vhdl_decl::get_type() const
{
   assert(type_);
   return type_;
}

void vhdl_decl::set_initial(vhdl_expr *initial)
{
   if (!has_initial_) {
      assert(initial_ == NULL);
      initial_ = initial;
      has_initial_ = true;
   }
}

void vhdl_port_decl::emit(std::ostream &of, int level) const
{
   of << name_ << " : ";

   switch (mode_) {
   case VHDL_PORT_IN:
      of << "in ";
      break;
   case VHDL_PORT_OUT:
      of << "out ";
      break;
   case VHDL_PORT_INOUT:
      of << "inout ";
      break;
   case VHDL_PORT_BUFFER:
      of << "buffer ";
      break;
   }

   type_->emit(of, level);
}

// If this is an `out' port make it a `buffer' so we can read from it
void vhdl_port_decl::ensure_readable()
{
   if (mode_ == VHDL_PORT_OUT)
      mode_ = VHDL_PORT_BUFFER;
}

// A port is readable if it is not `out'.
// We also make `buffer' ports not readable for these purposes since
// buffers cannot be directly mapped to outputs without an intermediate
// signal.
bool vhdl_port_decl::is_readable() const
{
   return mode_ != VHDL_PORT_OUT && mode_ != VHDL_PORT_BUFFER;
}

void vhdl_var_decl::emit(std::ostream &of, int level) const
{
   of << "variable " << name_ << " : ";
   type_->emit(of, level);

   if (initial_) {
      of << " := ";
      initial_->emit(of, level);
   }

   of << ";";
   emit_comment(of, level, true);
}

void vhdl_signal_decl::emit(std::ostream &of, int level) const
{
   of << "signal " << name_ << " : ";
   type_->emit(of, level);

   if (initial_) {
      of << " := ";
      initial_->emit(of, level);
   }

   of << ";";
   emit_comment(of, level, true);
}

void vhdl_type_decl::emit(std::ostream &of, int level) const
{
   of << "type " << name_ << " is ";
   of << type_->get_type_decl_string() << ";";
   emit_comment(of, level, true);
}

vhdl_expr::~vhdl_expr()
{

}

void vhdl_expr_list::add_expr(vhdl_expr *e)
{
   exprs_.push_back(e);
}

vhdl_expr_list::~vhdl_expr_list()
{

}

void vhdl_expr_list::find_vars(vhdl_var_set_t& read)
{
   for (list<vhdl_expr*>::const_iterator it = exprs_.begin();
        it != exprs_.end(); ++it)
      (*it)->find_vars(read);
}

void vhdl_expr_list::emit(std::ostream &of, int level) const
{
   of << "(";

   int size = exprs_.size();
   std::list<vhdl_expr*>::const_iterator it;
   for (it = exprs_.begin(); it != exprs_.end(); ++it) {
      (*it)->emit(of, level);
      if (--size > 0)
         of << ", ";
   }

   of << ")";
}

void vhdl_pcall_stmt::emit(std::ostream &of, int level) const
{
   of << name_;
   if (!exprs_.empty())
      exprs_.emit(of, level);
   of << ";";
}

void vhdl_pcall_stmt::find_vars(vhdl_var_set_t& read,
                                vhdl_var_set_t&)
{
   exprs_.find_vars(read);
}

vhdl_var_ref::~vhdl_var_ref()
{

}

void vhdl_var_ref::set_slice(vhdl_expr *s, int w)
{
   assert(type_);

   slice_ = s;
   slice_width_ = w;

   vhdl_type_name_t tname = type_->get_name();
   if (tname == VHDL_TYPE_ARRAY) {
      type_ = type_->get_base();
   }
   else {
      assert(tname == VHDL_TYPE_UNSIGNED || tname == VHDL_TYPE_SIGNED);

      if (w > 0)
         type_ = new vhdl_type(tname, w);
      else
         type_ = vhdl_type::std_logic();
   }
}

void vhdl_var_ref::find_vars(vhdl_var_set_t& read)
{
   read.insert(this);
}

void vhdl_var_ref::emit(std::ostream &of, int level) const
{
   of << name_;
   if (slice_) {
      of << "(";
      if (slice_width_ > 0) {
         slice_->emit(of, level);
         of << " + " << slice_width_ << " downto ";
      }
      slice_->emit(of, level);
      of << ")";
   }
}

void vhdl_const_string::emit(std::ostream &of, int) const
{
   of << "\"" << value_ << "\"";
}

void vhdl_null_stmt::emit(std::ostream &of, int level) const
{
   of << "null;";
   emit_comment(of, level, true);
}

void vhdl_fcall::find_vars(vhdl_var_set_t& read)
{
   exprs_.find_vars(read);
}

void vhdl_fcall::emit(std::ostream &of, int level) const
{
   of << name_;
   exprs_.emit(of, level);
}

vhdl_abstract_assign_stmt::~vhdl_abstract_assign_stmt()
{

}

void vhdl_abstract_assign_stmt::find_vars(vhdl_var_set_t& read,
                                          vhdl_var_set_t& write)
{
   lhs_->find_vars(write);
   rhs_->find_vars(read);
}

void vhdl_nbassign_stmt::emit(std::ostream &of, int level) const
{
   lhs_->emit(of, level);
   of << " <= ";
   rhs_->emit(of, level);

   if (after_) {
      of << " after ";
      after_->emit(of, level);
   }

   of << ";";
}

void vhdl_assign_stmt::emit(std::ostream &of, int level) const
{
   lhs_->emit(of, level);
   of << " := ";
   rhs_->emit(of, level);
   of << ";";
}

vhdl_const_bits::vhdl_const_bits(const char *value, int width, bool issigned,
                                 bool qualify)
   : vhdl_expr(issigned ? vhdl_type::nsigned(width)
               : vhdl_type::nunsigned(width), true),
     qualified_(qualify),
     signed_(issigned)
{
   // Can't rely on value being NULL-terminated
   while (width--)
      value_.push_back(*value++);
}

// True if char is not '1' or '0'
static bool is_meta_bit(char c)
{
   return c != '1' && c != '0';
}

// True if the bit strings contains characters other than '1' and '0'
bool vhdl_const_bits::has_meta_bits() const
{
   return find_if(value_.begin(), value_.end(), is_meta_bit) != value_.end();
}

void vhdl_const_bits::emit(std::ostream &of, int) const
{
   if (qualified_)
      of << (signed_ ? "signed" : "unsigned") << "'(";

   // If it's a width we can write in hex, prefer that over binary
   size_t bits = value_.size();
   int64_t ival = bits_to_int();
   if ((!signed_ || ival >= 0)
       && !has_meta_bits() && bits <= 64 && bits % 4 == 0) {
      of << "X\"" << hex << setfill('0') << setw(bits / 4) << ival;
   }
   else {
      of << "\"";

      std::string::const_reverse_iterator it;
      for (it = value_.rbegin(); it != value_.rend(); ++it)
         of << vl_to_vhdl_bit(*it);
   }

   of << (qualified_ ? "\")" : "\"");
}

void vhdl_const_bit::emit(std::ostream &of, int) const
{
   of << "'" << vl_to_vhdl_bit(bit_) << "'";
}

void vhdl_const_int::emit(std::ostream &of, int) const
{
   of << dec << value_;
   // We need to find a way to display a comment, since $time, etc. add one.
}

void vhdl_const_bool::emit(std::ostream &of, int) const
{
   of << (value_ ? "True" : "False");
}

void vhdl_const_time::emit(std::ostream &of, int) const
{
   of << dec << value_;
   switch (units_) {
   case TIME_UNIT_PS: of << " ps"; break;
   case TIME_UNIT_NS: of << " ns"; break;
   case TIME_UNIT_US: of << " us"; break;
   case TIME_UNIT_MS: of << " ms"; break;
   }
}

vhdl_cassign_stmt::~vhdl_cassign_stmt()
{

}

void vhdl_cassign_stmt::add_condition(vhdl_expr *value, vhdl_expr *cond)
{
   when_part_t when = { value, cond, NULL };
   whens_.push_back(when);
}

void vhdl_cassign_stmt::emit(std::ostream &of, int level) const
{
   lhs_->emit(of, level);
   of << " <= ";
   if (!whens_.empty()) {
      for (std::list<when_part_t>::const_iterator it = whens_.begin();
           it != whens_.end();
           ++it) {
         (*it).value->emit(of, level);
         of << " when ";
         (*it).cond->emit(of, level);
         of << " ";
      }
      of << "else ";
   }
   rhs_->emit(of, level);

   if (after_) {
      of << " after ";
      after_->emit(of, level);
   }

   of << ";";
}

vhdl_report_stmt::vhdl_report_stmt(vhdl_expr *text,
                                   vhdl_severity_t severity)
   : severity_(severity),
     text_(text)
{

}

void vhdl_report_stmt::emit(ostream& of, int level) const
{
   of << "report ";
   text_->emit(of, level);

   if (severity_ != SEVERITY_NOTE) {
      const char *levels[] = { "note", "warning", "error", "failure" };
      of << " severity " << levels[severity_];
   }

   of << ";";
}

void vhdl_report_stmt::find_vars(vhdl_var_set_t& read, vhdl_var_set_t&)
{
   text_->find_vars(read);
}

vhdl_assert_stmt::vhdl_assert_stmt(const char *reason)
   : vhdl_report_stmt(new vhdl_const_string(reason), SEVERITY_FAILURE)
{

}

void vhdl_assert_stmt::emit(std::ostream &of, int level) const
{
   of << "assert false ";  // TODO: Allow arbitrary expression
   vhdl_report_stmt::emit(of, level);
}

vhdl_if_stmt::vhdl_if_stmt(vhdl_expr *test)
{
   // Need to ensure that the expression is Boolean
   vhdl_type boolean(VHDL_TYPE_BOOLEAN);
   test_ = test->cast(&boolean);
}

vhdl_if_stmt::~vhdl_if_stmt()
{

}

stmt_container *vhdl_if_stmt::add_elsif(vhdl_expr *test)
{
   elsif ef = { test, new stmt_container };
   elsif_parts_.push_back(ef);
   return ef.container;
}

void vhdl_if_stmt::emit(std::ostream &of, int level) const
{
   emit_comment(of, level);

   of << "if ";
   test_->emit(of, level);
   of << " then";
   then_part_.emit(of, level);

   std::list<elsif>::const_iterator it;
   for (it = elsif_parts_.begin(); it != elsif_parts_.end(); ++it) {
      of << "elsif ";
      (*it).test->emit(of, level);
      of << " then";
      (*it).container->emit(of, level);
   }

   if (!else_part_.empty()) {
      of << "else";
      else_part_.emit(of, level);
   }
   of << "end if;";
}

void vhdl_if_stmt::find_vars(vhdl_var_set_t& read,
                             vhdl_var_set_t& write)
{
   test_->find_vars(read);

   then_part_.find_vars(read, write);
   else_part_.find_vars(read, write);

   for (list<elsif>::const_iterator it = elsif_parts_.begin();
        it != elsif_parts_.end(); ++it) {
      (*it).test->find_vars(read);
      (*it).container->find_vars(read, write);
   }
}

int vhdl_expr::paren_levels(0);

void vhdl_expr::open_parens(std::ostream& of)
{
   if (paren_levels++ > 0)
      of << "(";
}

void vhdl_expr::close_parens(std::ostream& of)
{
   assert(paren_levels > 0);

   if (--paren_levels > 0)
      of << ")";
}

vhdl_unaryop_expr::~vhdl_unaryop_expr()
{

}

void vhdl_unaryop_expr::find_vars(vhdl_var_set_t& read)
{
   operand_->find_vars(read);
}

void vhdl_unaryop_expr::emit(std::ostream &of, int level) const
{
   open_parens(of);

   switch (op_) {
   case VHDL_UNARYOP_NOT:
      of << "not ";
      break;
   case VHDL_UNARYOP_NEG:
      of << "-";
      break;
   }
   operand_->emit(of, level);

   close_parens(of);
}

vhdl_binop_expr::vhdl_binop_expr(vhdl_expr *left, vhdl_binop_t op,
                                 vhdl_expr *right, const vhdl_type *type)
   : vhdl_expr(type), op_(op)
{
   add_expr(left);
   add_expr(right);
}

vhdl_binop_expr::~vhdl_binop_expr()
{

}

void vhdl_binop_expr::add_expr(vhdl_expr *e)
{
   operands_.push_back(e);
}

void vhdl_binop_expr::add_expr_front(vhdl_expr *e)
{
   operands_.push_front(e);
}

void vhdl_binop_expr::find_vars(vhdl_var_set_t& read)
{
   for (list<vhdl_expr*>::const_iterator it = operands_.begin();
       it != operands_.end(); ++it)
      (*it)->find_vars(read);
}

void vhdl_binop_expr::emit(std::ostream &of, int level) const
{
   open_parens(of);

   assert(! operands_.empty());
   std::list<vhdl_expr*>::const_iterator it = operands_.begin();

   (*it)->emit(of, level);
   while (++it != operands_.end()) {
      const char* ops[] = {
         "and", "or", "=", "/=", "+", "-", "*", "<",
         ">", "<=", ">=", "sll", "srl", "xor", "&",
         "nand", "nor", "xnor", "/", "mod", "**", "sra", NULL
      };

      of << " " << ops[op_] << " ";

      (*it)->emit(of, level);
   }

   close_parens(of);
}

vhdl_bit_spec_expr::~vhdl_bit_spec_expr()
{

}

void vhdl_bit_spec_expr::add_bit(int bit, vhdl_expr *e)
{
   bit_map bm = { bit, e };
   bits_.push_back(bm);
}

void vhdl_bit_spec_expr::emit(std::ostream &of, int level) const
{
   of << "(";

   std::list<bit_map>::const_iterator it;
   it = bits_.begin();
   while (it != bits_.end()) {
      of << (*it).bit << " => ";
      (*it).e->emit(of, level);
      if (++it != bits_.end())
         of << ", ";
   }

   if (others_) {
      of << (bits_.empty() ? "" : ", ") << "others => ";
      others_->emit(of, level);
   }

   of << ")";
}

vhdl_case_branch::~vhdl_case_branch()
{

}

void vhdl_case_branch::emit(std::ostream &of, int level) const
{
   of << "when ";
   when_->emit(of, level);
   of << " =>";
   stmts_.emit(of, indent(level), false);
}

vhdl_case_stmt::~vhdl_case_stmt()
{

}

void vhdl_case_stmt::find_vars(vhdl_var_set_t& read,
                               vhdl_var_set_t& write)
{
   test_->find_vars(read);

   for (case_branch_list_t::const_iterator it = branches_.begin();
        it != branches_.end(); ++it) {
      (*it)->when_->find_vars(read);
      (*it)->stmts_.find_vars(read, write);
   }
}

void vhdl_case_stmt::emit(std::ostream &of, int level) const
{
   of << "case ";
   test_->emit(of, level);
   of << " is";
   newline(of, indent(level));

   case_branch_list_t::const_iterator it;
   int n = branches_.size();
   for (it = branches_.begin(); it != branches_.end(); ++it) {
      (*it)->emit(of, level);
      if (--n > 0)
         newline(of, indent(level));
      else
         newline(of, level);
   }

   of << "end case;";
}

vhdl_while_stmt::~vhdl_while_stmt()
{

}

void vhdl_while_stmt::find_vars(vhdl_var_set_t& read,
                                vhdl_var_set_t& write)
{
   test_->find_vars(read);

   vhdl_loop_stmt::find_vars(read, write);
}

void vhdl_while_stmt::emit(std::ostream &of, int level) const
{
   of << "while ";
   test_->emit(of, level);
   of << " ";
   vhdl_loop_stmt::emit(of, level);
}

void vhdl_loop_stmt::find_vars(vhdl_var_set_t& read,
                               vhdl_var_set_t& write)
{
   stmts_.find_vars(read, write);
}

void vhdl_loop_stmt::emit(std::ostream &of, int level) const
{
   of << "loop";
   stmts_.emit(of, level);
   of << "end loop;";
}

vhdl_for_stmt::~vhdl_for_stmt()
{

}


void vhdl_for_stmt::find_vars(vhdl_var_set_t& read,
                              vhdl_var_set_t& write)
{
   from_->find_vars(read);
   to_->find_vars(write);

   vhdl_loop_stmt::find_vars(read, write);
}

void vhdl_for_stmt::emit(std::ostream &of, int level) const
{
   of << "for " << lname_ << " in ";
   from_->emit(of, level);
   of << " to ";
   to_->emit(of, level);
   of << " ";
   vhdl_loop_stmt::emit(of, level);
}

vhdl_function::vhdl_function(const char *name, vhdl_type *ret_type)
   : vhdl_decl(name, ret_type)
{
   // A function contains two scopes:
   //  scope_ = The parameters
   //  variables_ = Local variables
   // A call to get_scope returns variables_ whose parent is scope_
   variables_.set_parent(&scope_);
}

void vhdl_function::emit(std::ostream &of, int level) const
{
   newline(of, level);
   emit_comment(of, level);
   of << "function " << name_ << " (";
   emit_children<vhdl_decl>(of, scope_.get_decls(), level, ";");
   of << ") ";
   newline(of, level);
   of << "return " << type_->get_string() << " is";
   emit_children<vhdl_decl>(of, variables_.get_decls(), level);
   of << "begin";
   stmts_.emit(of, level);
   of << "  return " << name_ << "_Result;";
   newline(of, level);
   of << "end function;";
}

void vhdl_forward_fdecl::emit(std::ostream &of, int level) const
{
   of << "function " << f_->get_name() << " (";
   emit_children<vhdl_decl>(of, f_->scope_.get_decls(), level, ";");
   of << ") ";
   newline(of, level);
   of << "return " << f_->type_->get_string() << ";";
   newline(of, level);
}

void vhdl_param_decl::emit(std::ostream &of, int level) const
{
   of << name_ << " : ";
   type_->emit(of, level);
}

vhdl_with_select_stmt::~vhdl_with_select_stmt()
{

}

void vhdl_with_select_stmt::emit(std::ostream &of, int level) const
{
   of << "with ";
   test_->emit(of, level);
   of << " select";
   emit_comment(of, level, true);
   newline(of, indent(level));

   out_->emit(of, level);
   of << " <= ";

   when_list_t::const_iterator it = whens_.begin();
   while (it != whens_.end()) {
      (*it).value->emit(of, level);
      if ((*it).delay) {
         of << " after ";
         (*it).delay->emit(of, level);
      }
      of << " when ";
      (*it).cond->emit(of, level);

      if (++it != whens_.end() || others_ != NULL) {
         of << ",";
         newline(of, indent(level));
      }
      else
         of << ";";
   }

   if (others_) {
      others_->emit(of, level);
      of << " when others;";
   }
}

void vhdl_with_select_stmt::add_condition(vhdl_expr *value, vhdl_expr *cond, vhdl_expr *delay)
{
   when_part_t when = { value, cond, delay };
   whens_.push_back(when);
}

void vhdl_with_select_stmt::add_default(vhdl_expr* value)
{
   others_ = value;
}
