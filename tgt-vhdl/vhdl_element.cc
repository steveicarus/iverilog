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

#include "vhdl_element.hh"

#include <algorithm>
#include <cassert>
#include <typeinfo>
#include <iostream>


static const int VHDL_INDENT = 2;  // Spaces to indent

static int indent(int level)
{
   return level + VHDL_INDENT;
}

/*
 * Emit a newline and indent to the correct level.
 */
static void newline(std::ofstream &of, int level)
{
   of << std::endl;
   while (level--)
      of << ' ';
}

static void blank_line(std::ofstream &of, int level)
{
   of << std::endl;
   newline(of, level);
}

template <class T>
void emit_children(std::ofstream &of,
                   const std::list<T*> &children,
                   int level) 
{      
   // Don't indent if there are no children
   if (children.size() == 0)
      newline(of, level);
   else {
      typename std::list<T*>::const_iterator it;
      for (it = children.begin(); it != children.end(); ++it) {
         newline(of, indent(level));
         (*it)->emit(of, indent(level));
      }
      newline(of, level);
   }
}

template <class T>
void delete_children(std::list<T*> &children)
{
   typename std::list<T*>::iterator it;
   for (it = children.begin(); it != children.end(); ++it)
      delete *it;
   children.clear();
}

void vhdl_element::set_comment(std::string comment)
{
   comment_ = comment;
}

/*
 * Draw the comment for any element. The comment is either on
 * a line before the element (end_of_line is false) or at the
 * end of the line containing the element (end_of_line is true).
 */
void vhdl_element::emit_comment(std::ofstream &of, int level,
                                bool end_of_line) const
{
   if (comment_.size() > 0) {
      if (end_of_line)
         of << "  ";
      of << "-- " << comment_;
      if (!end_of_line)
         newline(of, level);
   }
}

vhdl_entity::vhdl_entity(const char *name, const char *derived_from,
                         vhdl_arch *arch)
   : name_(name), arch_(arch), derived_from_(derived_from)
{
   arch->parent_ = this;
}

vhdl_entity::~vhdl_entity()
{   
   delete arch_;
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

void vhdl_entity::emit(std::ofstream &of, int level) const
{
   // Pretty much every design will use std_logic so we
   // might as well include it by default
   of << "library ieee;" << std::endl;
   of << "use ieee.std_logic_1164.all;" << std::endl;
   
   for (std::list<std::string>::const_iterator it = uses_.begin();
        it != uses_.end();
        ++it)
      of << "use " << *it << ".all;" << std::endl;
   of << std::endl;
   
   emit_comment(of, level);
   of << "entity " << name_ << " is";
   // ...ports...
   // newline(indent(level));
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
   return NULL;
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
   : name_(name)
{

}

vhdl_process::~vhdl_process()
{
   delete_children<vhdl_seq_stmt>(stmts_);
   delete_children<vhdl_decl>(decls_);
}

void vhdl_process::add_stmt(vhdl_seq_stmt* stmt)
{
   stmts_.push_back(stmt);
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
   emit_children<vhdl_seq_stmt>(of, stmts_, level);
   of << "end process;";
   newline(of, level);
}

vhdl_comp_inst::vhdl_comp_inst(const char *inst_name, const char *comp_name)
   : comp_name_(comp_name), inst_name_(inst_name)
{
   
}

void vhdl_comp_inst::emit(std::ofstream &of, int level) const
{
   // If there are no ports or generics we don't need to mention them...
   emit_comment(of, level);
   of << inst_name_ << ": " << comp_name_ << ";";
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

   return decl;
}

void vhdl_component_decl::emit(std::ofstream &of, int level) const
{
   emit_comment(of, level);
   of << "component " << name_ << " is";
   // ...ports...
   newline(of, level);
   of << "end component;";
}

void vhdl_wait_stmt::emit(std::ofstream &of, int level) const
{
   // TODO: There are lots of different types of `wait'
   of << "wait;";
}

/*
 * Create a deep copy of this type, so it can appear in more
 * than one place in the AST.
 */
vhdl_type *vhdl_scalar_type::clone() const
{
   return new vhdl_scalar_type(name_.c_str());
}

vhdl_scalar_type *vhdl_scalar_type::std_logic()
{
   return new vhdl_scalar_type("std_logic");
}

vhdl_scalar_type *vhdl_scalar_type::string()
{
   return new vhdl_scalar_type("String");
}

vhdl_scalar_type *vhdl_scalar_type::line()
{
   return new vhdl_scalar_type("Line");
}

void vhdl_scalar_type::emit(std::ofstream &of, int level) const
{
   of << name_;
}

vhdl_var_decl::~vhdl_var_decl()
{
   delete type_;
}

void vhdl_var_decl::emit(std::ofstream &of, int level) const
{
   of << "variable " << name_ << " : ";
   type_->emit(of, level);
   of << ";";
   emit_comment(of, level, true);
}

vhdl_signal_decl::~vhdl_signal_decl()
{
   delete type_;
}

void vhdl_signal_decl::emit(std::ofstream &of, int level) const
{
   of << "signal " << name_ << " : ";
   type_->emit(of, level);
   of << ";";
   emit_comment(of, level, true);
}

vhdl_expr::~vhdl_expr()
{
   delete type_;
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

void vhdl_nbassign_stmt::emit(std::ofstream &of, int level) const
{
   lhs_->emit(of, level);
   of << " <= ";
   rhs_->emit(of, level);
   of << ";";
}
