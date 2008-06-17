/*
 *  VHDL abstract syntax elements.
 *
 *  Copyright (C) 2008  Nick Gasson (nick@nickg.me.uk)
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
#include <iostream>

vhdl_entity::vhdl_entity(const char *name, const char *derived_from,
                         vhdl_arch *arch)
   : name_(name), arch_(arch), derived_from_(derived_from)
{
   arch->parent_ = this;
}

vhdl_entity::~vhdl_entity()
{   
   delete arch_;
   delete_children<vhdl_decl>(ports_);
}

/*
 * Add a package to the list of `use' statements before
 * the entity.
 */
void vhdl_entity::requires_package(const char *spec)
{
   std::string pname(spec);
   std::list<std::string>::iterator it;
   for (it = uses_.begin(); it != uses_.end(); ++it) {
      if (*it == pname)
         return;
   }
   uses_.push_back(spec);
}

/*
 * Find a port declaration by name
 */
vhdl_decl *vhdl_entity::get_decl(const std::string &name) const
{
   decl_list_t::const_iterator it;
   for (it = ports_.begin(); it != ports_.end(); ++it) {
      if ((*it)->get_name() == name)
         return *it;
   }
   return NULL;
}

void vhdl_entity::add_port(vhdl_port_decl *decl)
{
   ports_.push_back(decl);
}

void vhdl_entity::emit(std::ofstream &of, int level) const
{
   // Pretty much every design will use std_logic so we
   // might as well include it by default
   of << "library ieee;" << std::endl;
   of << "use ieee.std_logic_1164.all;" << std::endl;
   of << "use ieee.numeric_std.all;" << std::endl;
   
   for (std::list<std::string>::const_iterator it = uses_.begin();
        it != uses_.end();
        ++it)
      of << "use " << *it << ".all;" << std::endl;
   of << std::endl;
   
   emit_comment(of, level);
   of << "entity " << name_ << " is";

   if (ports_.size() > 0) {
      newline(of, indent(level));
      of << "port (";
      emit_children<vhdl_decl>(of, ports_, indent(level), ";");
      of << ");";
   }
   
   newline(of, level);
   of << "end entity; ";
   blank_line(of, level);  // Extra blank line after entities
   arch_->emit(of, level);
}

vhdl_arch::vhdl_arch(const char *entity, const char *name)
   : parent_(NULL), name_(name), entity_(entity)
{
   
}

vhdl_arch::~vhdl_arch()
{
   delete_children<vhdl_conc_stmt>(stmts_);
   delete_children<vhdl_decl>(decls_);
}

void vhdl_arch::add_stmt(vhdl_conc_stmt *stmt)
{
   stmt->parent_ = this;
   stmts_.push_back(stmt);
}

void vhdl_arch::add_decl(vhdl_decl *decl)
{
   decls_.push_back(decl);
}

vhdl_entity *vhdl_arch::get_parent() const
{
   assert(parent_);
   return parent_;
}

void vhdl_arch::emit(std::ofstream &of, int level) const
{
   emit_comment(of, level);
   of << "architecture " << name_ << " of " << entity_;
   of << " is";
   emit_children<vhdl_decl>(of, decls_, level);
   of << "begin";
   emit_children<vhdl_conc_stmt>(of, stmts_, level);
   of << "end architecture;";
   blank_line(of, level);  // Extra blank line after architectures;
}

vhdl_decl *vhdl_arch::get_decl(const std::string &name) const
{
   decl_list_t::const_iterator it;
   for (it = decls_.begin(); it != decls_.end(); ++it) {
      if ((*it)->get_name() == name)
         return *it;
   }

   // Maybe it's a port rather than an internal signal?
   assert(parent_);
   return parent_->get_decl(name);
}

/*
 * True if component `name' has already been declared in this
 * architecture. This is a bit of hack, since it uses typeid
 * to distinguish between components and other declarations.
 */
bool vhdl_arch::have_declared_component(const std::string &name) const
{
   std::string comp_typename(typeid(vhdl_component_decl).name());
   decl_list_t::const_iterator it;
   for (it = decls_.begin(); it != decls_.end(); ++it) {
      if (comp_typename == typeid(**it).name()
          && (*it)->get_name() == name)
         return true;
   }
   return false;
}

/*
 * True if any declaration of `name' has been added to the
 * architecture.
 */
bool vhdl_arch::have_declared(const std::string &name) const
{
   return get_decl(name) != NULL;
}

vhdl_arch *vhdl_conc_stmt::get_parent() const
{
   assert(parent_);
   return parent_;
}

vhdl_process::vhdl_process(const char *name)
   : name_(name), initial_(false)
{

}

vhdl_process::~vhdl_process()
{
   delete_children<vhdl_decl>(decls_);
}

void vhdl_process::add_decl(vhdl_decl* decl)
{
   decls_.push_back(decl);
}

void vhdl_process::add_sensitivity(const char *name)
{
   sens_.push_back(name);
}

bool vhdl_process::have_declared_var(const std::string &name) const
{
   decl_list_t::const_iterator it;
   for (it = decls_.begin(); it != decls_.end(); ++it) {
      if ((*it)->get_name() == name)
         return true;
   }
   return false;
}

void vhdl_process::emit(std::ofstream &of, int level) const
{
   // If there are no statements in the body, this process
   // can't possibly do anything, so don't bother to emit it
   if (stmts_.empty()) {
      of << "-- Removed one empty process";
      newline(of, level);
      return;
   }
   
   emit_comment(of, level);
   if (name_.size() > 0)
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
   emit_children<vhdl_decl>(of, decls_, level);
   of << "begin";
   stmts_.emit(of, level);
   of << "end process;";
   newline(of, level);
}

stmt_container::~stmt_container()
{
   delete_children<vhdl_seq_stmt>(stmts_);
}

void stmt_container::add_stmt(vhdl_seq_stmt *stmt)
{
   stmts_.push_back(stmt);
}

void stmt_container::emit(std::ofstream &of, int level) const
{
   emit_children<vhdl_seq_stmt>(of, stmts_, level);  
}

vhdl_comp_inst::vhdl_comp_inst(const char *inst_name, const char *comp_name)
   : comp_name_(comp_name), inst_name_(inst_name)
{
   
}

vhdl_comp_inst::~vhdl_comp_inst()
{
   port_map_list_t::iterator it;
   for (it = mapping_.begin(); it != mapping_.end(); ++it) {
      delete (*it).expr;
   }
   mapping_.clear();
}

void vhdl_comp_inst::map_port(const char *name, vhdl_expr *expr)
{
   port_map_t pmap = { name, expr };
   mapping_.push_back(pmap);
}

void vhdl_comp_inst::emit(std::ofstream &of, int level) const
{
   emit_comment(of, level);
   of << inst_name_ << ": " << comp_name_;

   // If there are no ports or generics we don't need to mention them...
   if (mapping_.size() > 0) {
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
   newline(of, level);
}

vhdl_component_decl::vhdl_component_decl(const char *name)
   : vhdl_decl(name)
{

}

/*
 * Create a component declaration for the given entity.
 */
vhdl_component_decl *vhdl_component_decl::component_decl_for(const vhdl_entity *ent)
{
   assert(ent != NULL);

   vhdl_component_decl *decl = new vhdl_component_decl
      (ent->get_name().c_str());

   decl->ports_ = ent->get_ports();
   
   return decl;
}

void vhdl_component_decl::emit(std::ofstream &of, int level) const
{
   emit_comment(of, level);
   of << "component " << name_ << " is";

   if (ports_.size() > 0) {
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
   if (expr_ != NULL)
      delete expr_;
}

void vhdl_wait_stmt::emit(std::ofstream &of, int level) const
{
   of << "wait";

   switch (type_) {
   case VHDL_WAIT_INDEF:
      break;
   case VHDL_WAIT_FOR_NS:
      assert(expr_);
      of << " for ";
      expr_->emit(of, level);
      of << " ns";
      break;
   }
   
   of << ";";
}

vhdl_decl::~vhdl_decl()
{
   if (type_ != NULL)
      delete type_;
   if (initial_ != NULL)
      delete initial_;
}

void vhdl_decl::set_initial(vhdl_expr *initial)
{
   if (initial_ != NULL)
      delete initial_;
   initial_ = initial;
}

void vhdl_port_decl::emit(std::ofstream &of, int level) const
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
   }
   
   type_->emit(of, level);
}

void vhdl_var_decl::emit(std::ofstream &of, int level) const
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

void vhdl_signal_decl::emit(std::ofstream &of, int level) const
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

vhdl_expr::~vhdl_expr()
{
   if (type_ != NULL)
      delete type_;
}

/*
 * The default cast just assumes there's a VHDL cast function to
 * do the job for us.
 */
vhdl_expr *vhdl_expr::cast(const vhdl_type *to)
{
   if (to->get_name() == type_->get_name())
      return this;
   else if (to->get_name() == VHDL_TYPE_BOOLEAN) {
      // '1' is true all else are false
      vhdl_const_bit *one = new vhdl_const_bit('1');
      return new vhdl_binop_expr
         (this, VHDL_BINOP_EQ, one, vhdl_type::boolean());
   }
   else {
      vhdl_fcall *conv =
         new vhdl_fcall(to->get_string().c_str(), new vhdl_type(*to));
      conv->add_expr(this);
      
      return conv;
   }
}

void vhdl_expr_list::add_expr(vhdl_expr *e)
{
   exprs_.push_back(e);
}

vhdl_expr_list::~vhdl_expr_list()
{
   delete_children<vhdl_expr>(exprs_);
}

void vhdl_expr_list::emit(std::ofstream &of, int level) const
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

void vhdl_pcall_stmt::emit(std::ofstream &of, int level) const
{
   of << name_;
   if (!exprs_.empty())
      exprs_.emit(of, level);
   of << ";";
}

void vhdl_var_ref::emit(std::ofstream &of, int level) const
{
   of << name_;
}

void vhdl_const_string::emit(std::ofstream &of, int level) const
{
   // In some instances a string literal can be ambiguous between
   // a String type and some other types (e.g. std_logic_vector)
   // The explicit cast to String removes this ambiguity (although
   // isn't always strictly necessary) 
   of << "String'(\"" << value_ << "\")";
}

void vhdl_null_stmt::emit(std::ofstream &of, int level) const
{
   of << "null;";
}

void vhdl_fcall::emit(std::ofstream &of, int level) const
{
   of << name_;
   exprs_.emit(of, level);
}

vhdl_nbassign_stmt::~vhdl_nbassign_stmt()
{
   delete lhs_;
   delete rhs_;
   if (after_)
      delete after_;
}

void vhdl_nbassign_stmt::emit(std::ofstream &of, int level) const
{
   lhs_->emit(of, level);
   of << " <= ";
   rhs_->emit(of, level);

   if (after_) {
      of << " after ";
      after_->emit(of, level);
      of << " ns";
   }
   
   of << ";";
}

vhdl_const_bits::vhdl_const_bits(const char *value, int width)   
   : vhdl_expr(vhdl_type::nsigned(width))
{
   // Can't rely on value being NULL-terminated
   while (width--)
      value_.push_back(*value++);
}

vhdl_expr *vhdl_const_bits::cast(const vhdl_type *to)
{  
   if (to->get_name() == VHDL_TYPE_STD_LOGIC) {
      // VHDL won't let us cast directly between a vector and
      // a scalar type
      // But we don't need to here as we have the bits available

      // Take the least significant bit
      char lsb = value_[0];

      return new vhdl_const_bit(lsb);
   }
   else
      return vhdl_expr::cast(to);
}

void vhdl_const_bits::emit(std::ofstream &of, int level) const
{
   of << "signed'(\"";

   // The bits appear to be in reverse order
   std::string::const_reverse_iterator it;
   for (it = value_.rbegin(); it != value_.rend(); ++it)
      of << vl_to_vhdl_bit(*it);

   of << "\")";
}

void vhdl_const_bit::emit(std::ofstream &of, int level) const
{
   of << "'" << vl_to_vhdl_bit(bit_) << "'";
}

void vhdl_const_int::emit(std::ofstream &of, int level) const
{
   of << value_;
}

vhdl_cassign_stmt::~vhdl_cassign_stmt()
{
   delete lhs_;
   delete rhs_;
}

void vhdl_cassign_stmt::emit(std::ofstream &of, int level) const
{
   lhs_->emit(of, level);
   of << " <= ";
   rhs_->emit(of, level);
   of << ";";
}

void vhdl_assert_stmt::emit(std::ofstream &of, int level) const
{
   of << "assert false";  // TODO: Allow arbitrary expression 
   of << " report \"" << reason_ << "\" severity failure;";
}

vhdl_if_stmt::vhdl_if_stmt(vhdl_expr *test)
{
   // Need to ensure that the expression is Boolean
   vhdl_type boolean(VHDL_TYPE_BOOLEAN);
   test_ = test->cast(&boolean);
}

vhdl_if_stmt::~vhdl_if_stmt()
{
   delete test_;
}

void vhdl_if_stmt::emit(std::ofstream &of, int level) const
{
   of << "if ";
   test_->emit(of, level);
   of << " then";
   then_part_.emit(of, level);
   if (!else_part_.empty()) {
      of << "else";
      else_part_.emit(of, level);
   }
   of << "end if;";
}

vhdl_unaryop_expr::~vhdl_unaryop_expr()
{
   delete operand_;
}

void vhdl_unaryop_expr::emit(std::ofstream &of, int level) const
{
   switch (op_) {
   case VHDL_UNARYOP_NOT:
      of << "not ";
      break;
   }
   operand_->emit(of, level);
}

vhdl_binop_expr::vhdl_binop_expr(vhdl_expr *left, vhdl_binop_t op,
                                 vhdl_expr *right, vhdl_type *type)
   : vhdl_expr(type), op_(op)
{
   add_expr(left);
   add_expr(right);
}

vhdl_binop_expr::~vhdl_binop_expr()
{
   delete_children<vhdl_expr>(operands_);
}

void vhdl_binop_expr::add_expr(vhdl_expr *e)
{
   operands_.push_back(e);
}

void vhdl_binop_expr::emit(std::ofstream &of, int level) const
{
   // Expressions are fully parenthesized to remove any
   // ambiguity in the output

   of << "(";

   assert(operands_.size() > 0);   
   std::list<vhdl_expr*>::const_iterator it = operands_.begin();

   (*it)->emit(of, level);
   while (++it != operands_.end()) {
      switch (op_) {
      case VHDL_BINOP_AND:
         of << " and ";
         break;
      case VHDL_BINOP_OR:
         of << " or ";
         break;
      case VHDL_BINOP_EQ:
         of << " = ";
         break;
      case VHDL_BINOP_NEQ:
         of << " /= ";
         break;
      case VHDL_BINOP_ADD:
         of << " + ";
         break;
      case VHDL_BINOP_SUB:
         of << " - ";
         break;
      case VHDL_BINOP_MULT:
         of << " * ";
         break;
      }

      (*it)->emit(of, level);
   }      

   of << ")";
}





