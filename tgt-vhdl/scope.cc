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
 * Create a VHDL entity for scopes of type IVL_SCT_MODULE.
 */
static vhdl_entity *create_entity_for(ivl_scope_t scope)
{
   assert(ivl_scope_type(scope) == IVL_SCT_MODULE);

   // The type name will become the entity name
   const char *tname = ivl_scope_tname(scope);

   // Verilog does not have the entity/architecture distinction
   // so we always create a pair and associate the architecture
   // with the entity for convenience (this also means that we
   // retain a 1-to-1 mapping of scope to VHDL element)
   vhdl_arch *arch = new vhdl_arch(tname);
   vhdl_entity *ent = new vhdl_entity(tname, arch);

   // Build a comment to add to the entity/architecture
   std::ostringstream ss;
   ss << "Generated from " << ivl_scope_name(scope);
   ss << " (" << ivl_scope_def_file(scope) << ":";
   ss << ivl_scope_def_lineno(scope) << ")";
   
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

   vhdl_entity *ent = find_entity(ivl_scope_tname(scope)); 
   if (NULL == ent)
      ent = create_entity_for(scope);
   assert(ent);

   if (parent != NULL) {
      std::cout << "parent " << ivl_scope_name(parent) << std::endl;
   }
      
   return 0;
}

int draw_scope(ivl_scope_t scope, void *_parent)
{
   ivl_scope_t parent = static_cast<ivl_scope_t>(_parent);
   
   const char *name = ivl_scope_name(scope);
   const char *basename = ivl_scope_basename(scope);

   std::cout << "scope " << name << " (" << basename << ")" << std::endl;

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

