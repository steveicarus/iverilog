/*
 *  VHDL code generation for processes.
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

#include "vhdl_target.h"
#include "vhdl_element.hh"
#include "state.hh"

#include <iostream>
#include <cassert>
#include <sstream>

/*
 * Check to see if the process should have a name.
 */
static const char * get_process_name(ivl_process_t proc)
{
   const char* name = "";
   // Look for always @(...) begin : <name> to find the name
   if (ivl_process_type(proc) == IVL_PR_ALWAYS) {
      ivl_statement_t stmt = ivl_process_stmt(proc);
      if (ivl_statement_type(stmt) == IVL_ST_WAIT) {
	 stmt = ivl_stmt_sub_stmt(stmt);
	 if (ivl_statement_type(stmt) == IVL_ST_BLOCK) {
	    ivl_scope_t proc_scope = ivl_stmt_block_scope(stmt);
	    if (proc_scope) name = ivl_scope_basename(proc_scope);
	 }
      }
   }

   return name;
}

/*
 * Convert a Verilog process to VHDL and add it to the architecture
 * of the given entity.
 */
static int generate_vhdl_process(vhdl_entity *ent, ivl_process_t proc)
{
   set_active_entity(ent);

   // Create a new process and store it in the entity's
   // architecture. This needs to be done first or the
   // parent link won't be valid (and draw_stmt needs this
   // to add information to the architecture)
   vhdl_process *vhdl_proc = new vhdl_process(get_process_name(proc));
   ent->get_arch()->add_stmt(vhdl_proc);

   // If this is an initial process, push signal initialisation
   // into the declarations
   vhdl_proc->get_scope()->set_initializing
      (ivl_process_type(proc) == IVL_PR_INITIAL);

   ivl_statement_t stmt = ivl_process_stmt(proc);
   int rc = draw_stmt(vhdl_proc, vhdl_proc->get_container(), stmt);
   if (rc != 0)
      return rc;

   // Initial processes are translated to VHDL processes with
   // no sensitivity list and and indefinite wait statement at
   // the end
   // However, if no statements were added to the container
   // by draw_stmt, don't bother adding a wait as `emit'
   // will optimise the process out of the output
   bool is_initial = ivl_process_type(proc) == IVL_PR_INITIAL;
   bool is_empty = vhdl_proc->get_container()->empty();

   if (is_initial && !is_empty) {
      vhdl_wait_stmt *wait = new vhdl_wait_stmt();
      vhdl_proc->get_container()->add_stmt(wait);
   }

   // Add a comment indicating where it came from
   ivl_scope_t scope = ivl_process_scope(proc);
   const char *type = ivl_process_type(proc) == IVL_PR_INITIAL
      ? "initial" : "always";
   std::ostringstream ss;
   ss << "Generated from " << type << " process in "
      << ivl_scope_tname(scope) << " ("
      << ivl_process_file(proc) << ":"
      << ivl_process_lineno(proc) << ")";
   vhdl_proc->set_comment(ss.str());

   set_active_entity(NULL);
   return 0;
}

extern "C" int draw_process(ivl_process_t proc, void *)
{
   ivl_scope_t scope = ivl_process_scope(proc);

   if (!is_default_scope_instance(scope))
      return 0;  // Ignore this process at it's not in a scope that
                 // we're using to generate code

   debug_msg("Translating process in scope type %s (%s:%d)",
             ivl_scope_tname(scope), ivl_process_file(proc),
             ivl_process_lineno(proc));

   // Skip over any generate and begin scopes until we find
   // the module that contains them - this is where we will
   // generate the process
   while (ivl_scope_type(scope) == IVL_SCT_GENERATE
      || ivl_scope_type(scope) == IVL_SCT_BEGIN)
      scope = ivl_scope_parent(scope);

   assert(ivl_scope_type(scope) == IVL_SCT_MODULE);
   vhdl_entity *ent = find_entity(scope);
   assert(ent != NULL);

   return generate_vhdl_process(ent, proc);
}
