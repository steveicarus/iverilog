/*
 *  VHDL code generation for processes.
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
#include <cassert>
#include <sstream>

/*
 * Convert a Verilog process to VHDL and add it to the architecture
 * of the given entity.
 */
static int generate_vhdl_process(vhdl_entity *ent, ivl_process_t proc)
{
   vhdl_process *vhdl_proc = new vhdl_process();

   // TODO: Add statements

   // Add a comment indicating where it came from
   ivl_scope_t scope = ivl_process_scope(proc);
   const char *type = ivl_process_type(proc) == IVL_PR_INITIAL
      ? "initial" : "always";
   std::ostringstream ss;
   ss << "Generated from " << type << " process in scope ";
   ss << ivl_scope_name(scope);
   vhdl_proc->set_comment(ss.str());

   // Store it in the entity's architecture
   ent->get_arch()->add_stmt(vhdl_proc);
   
   return 0;
}

int draw_process(ivl_process_t proc, void *cd)
{
   ivl_scope_t scope = ivl_process_scope(proc);
   const char *scope_name = ivl_scope_name(scope);

   // A process should occur in a module scope, therefore it
   // should have already been assigned a VHDL entity
   assert(ivl_scope_type(scope) == IVL_SCT_MODULE);
   vhdl_entity *ent = find_entity(ivl_scope_tname(scope));
   assert(ent != NULL);

   // If the scope this process belongs to is the same as the
   // VHDL entity was generated from, then create a VHDL process
   // from this Verilog process. This ensures that each process
   // is translated at most once, no matter how many times it
   // appears in the hierarchy.
   if (ent->get_derived_from() == scope_name) {
      std::cout << "New process encountered in " << scope_name << std::endl;
      return generate_vhdl_process(ent, proc);
   }
   else {
      std::cout << "Ignoring already seen process in ";
      std::cout << scope_name << std::endl;
      return 0;
   }
}
