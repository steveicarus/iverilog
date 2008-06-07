/*
 *  VHDL code generation for scopes.
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

#include "vhdl_target.h"
#include "vhdl_element.hh"

#include <iostream>
#include <sstream>
#include <cassert>

/*
 * Declare all signals for a scope in an architecture.
 */
static void declare_signals(vhdl_arch *arch, ivl_scope_t scope)
{
   int nsigs = ivl_scope_sigs(scope);
   for (int i = 0; i < nsigs; i++) {
      ivl_signal_t sig = ivl_scope_sig(scope, i);

      int width = ivl_signal_width(sig);
      vhdl_type *sig_type;
      if (width > 0)
         sig_type = vhdl_scalar_type::std_logic();
      else
         sig_type = vhdl_vector_type::std_logic_vector(width-1, 0);
      vhdl_signal_decl *decl =
         new vhdl_signal_decl(ivl_signal_basename(sig), sig_type);
      arch->add_decl(decl);
   }
}

/*
 * Create a VHDL entity for scopes of type IVL_SCT_MODULE.
 */
static vhdl_entity *create_entity_for(ivl_scope_t scope)
{
   assert(ivl_scope_type(scope) == IVL_SCT_MODULE);

   // The type name will become the entity name
   const char *tname = ivl_scope_tname(scope);

   // Remember the scope name this entity was derived from so
   // the correct processes can be added later
   const char *derived_from = ivl_scope_name(scope);
   
   // Verilog does not have the entity/architecture distinction
   // so we always create a pair and associate the architecture
   // with the entity for convenience (this also means that we
   // retain a 1-to-1 mapping of scope to VHDL element)
   vhdl_arch *arch = new vhdl_arch(tname);
   vhdl_entity *ent = new vhdl_entity(tname, derived_from, arch);

   // Locate all the signals in this module and add them to
   // the architecture
   declare_signals(arch, scope);

   // Build a comment to add to the entity/architecture
   std::ostringstream ss;
   ss << "Generated from Verilog module " << ivl_scope_tname(scope);
   
   arch->set_comment(ss.str());
   ent->set_comment(ss.str());
   
   remember_entity(ent);
   return ent;
}

/*
 * Instantiate an entity in the hierarchy, and possibly create
 * that entity if it hasn't been encountered yet.
 */
static int draw_module(ivl_scope_t scope, ivl_scope_t parent)
{
   assert(ivl_scope_type(scope) == IVL_SCT_MODULE);

   // Maybe we need to create this entity first?
   vhdl_entity *ent = find_entity(ivl_scope_tname(scope)); 
   if (NULL == ent)
      ent = create_entity_for(scope);
   assert(ent);

   // Is this module instantiated inside another?
   if (parent != NULL) {
      vhdl_entity *parent_ent = find_entity(ivl_scope_tname(parent));
      assert(parent_ent != NULL);

      // Make sure we only collect instantiations from *one*
      // example of this module in the hieararchy
      if (parent_ent->get_derived_from() == ivl_scope_name(parent)) {

         vhdl_arch *parent_arch = parent_ent->get_arch();
         assert(parent_arch != NULL);
         
         // Create a forward declaration for it
         if (!parent_arch->have_declared_component(ent->get_name())) {
            vhdl_decl *comp_decl = vhdl_component_decl::component_decl_for(ent);
            parent_arch->add_decl(comp_decl);
         }
         
         // And an instantiation statement
         const char *inst_name = ivl_scope_basename(scope);
         vhdl_comp_inst *inst =
            new vhdl_comp_inst(inst_name, ent->get_name().c_str());
         parent_arch->add_stmt(inst);
      }
      else {
         // Ignore this instantiation (already accounted for)
      }
   }
   
   return 0;
}

int draw_scope(ivl_scope_t scope, void *_parent)
{
   ivl_scope_t parent = static_cast<ivl_scope_t>(_parent);
   
   ivl_scope_type_t type = ivl_scope_type(scope);
   int rc = 0;
   switch (type) {
   case IVL_SCT_MODULE:
      rc = draw_module(scope, parent);
      break;
   default:
      error("No VHDL conversion for %s (at %s)",
            ivl_scope_tname(scope),
            ivl_scope_name(scope));
      break;
   }
   if (rc != 0)
      return rc;
   
   rc = ivl_scope_children(scope, draw_scope, scope);
   if (rc != 0)
      return rc;
   
   return 0;
}

