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


//////// HELPER FUNCTIONS ////////

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


//////// ALL ELEMENTS ////////

void vhdl_element::set_comment(std::string comment)
{
   comment_ = comment;
}

void vhdl_element::emit_comment(std::ofstream &of, int level) const
{
   if (comment_.size() > 0) {
      of << "-- " << comment_;
      newline(of, level);
   }
}


//////// ENTITY ////////

vhdl_entity::vhdl_entity(const char *name, const char *derived_from,
                         vhdl_arch *arch)
   : name_(name), arch_(arch), derived_from_(derived_from)
{

}

vhdl_entity::~vhdl_entity()
{
   delete arch_;
}

void vhdl_entity::emit(std::ofstream &of, int level) const
{
   emit_comment(of, level);
   of << "entity " << name_ << " is";
   // ...ports...
   // newline(indent(level));
   newline(of, level);
   of << "end entity; ";
   blank_line(of, level);  // Extra blank line after entities
   arch_->emit(of, level);
}


//////// ARCHITECTURE ////////

vhdl_arch::vhdl_arch(const char *entity, const char *name)
   : name_(name), entity_(entity)
{
   
}

vhdl_arch::~vhdl_arch()
{
   for (conc_stmt_list_t::iterator it = stmts_.begin();
        it != stmts_.end();
        ++it)
      delete (*it);
   stmts_.clear();
   
   for (decl_list_t::iterator it = decls_.begin();
        it != decls_.end();
        ++it)
      delete (*it);
   decls_.clear();
}

void vhdl_arch::add_stmt(vhdl_conc_stmt *stmt)
{
   stmts_.push_back(stmt);
}

void vhdl_arch::add_decl(vhdl_decl *decl)
{
   decls_.push_back(decl);
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


//////// PROCESS ////////

vhdl_process::vhdl_process(const char *name)
   : name_(name)
{

}

vhdl_process::~vhdl_process()
{
   seq_stmt_list_t::iterator it;
   for (it = stmts_.begin(); it != stmts_.end(); ++it)
      delete (*it);
   stmts_.clear();
}

void vhdl_process::add_stmt(vhdl_seq_stmt* stmt)
{
   stmts_.push_back(stmt);
}

void vhdl_process::emit(std::ofstream &of, int level) const
{
   emit_comment(of, level);
   if (name_.size() > 0)
      of << name_ << ": ";
   of << "process is";  // TODO: sensitivity
   newline(of, level);
   // ...declarations...
   of << "begin";
   newline(of, level);
   // ...statements...
   of << "wait;";   // Just to stop the simulation hanging
   newline(of, level);
   of << "end process;";
   newline(of, level);
}


//////// COMPONENT INSTANTIATION ////////

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


//////// COMPONENT DECLARATIONS ////////

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
