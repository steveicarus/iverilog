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
#include <map>

/*
 * Implementing blocking assignment is a little tricky since
 * the semantics are a little different to VHDL:
 *
 * In Verilog a blocking assignment (=) can be used anywhere
 * non-blocking assignment (<=) can be. In VHDL blocking
 * assignment (:=) can only be used with variables, and
 * non-blocking assignment (<=) can only be used with signals.
 * All Verilog variables are translated into signals in the
 * VHDL architecture. This means we cannot use the VHDL :=
 * operator directly. Furthermore, VHDL variables can only
 * be declared within processes, so it wouldn't help to
 * make all Verilog variables VHDL variables.
 *
 * The solution is to generate a VHDL variable in a process
 * whenever a blocking assignment is made to a signal. The
 * assignment is made to this variable instead, and
 * g_assign_vars below remembers the temporary variables
 * that have been generated. Any subsequent blocking assignments
 * are made to the same variable. At either the end of the
 * process or a `wait' statement, the temporaries are assigned
 * back to the signals, and the temporaries are forgotten. This
 * has exactly the same (external) behaviour as the Verilog
 * blocking assignment, since no external process will be able
 * to observe that the assignment wasn't made immediately.
 *
 * For example:
 *
 *   initial begin
 *     a = 5;
 *     b = a + 3;
 *   end
 *
 * Is translated to:
 *
 *   process is
 *     variable a_Var : Some_Type;
 *     variable b_Var : Some_Type;
 *   begin
 *     a_Var := 5; 
 *     b_Var := a_Var + 3;
 *     a <= a_Var;
 *     b <= b_Var;
 *   end process;
 */
typedef std::map<std::string, ivl_signal_t> var_temp_set_t;
static var_temp_set_t g_assign_vars;

/*
 * Called whenever a blocking assignment is made to sig.
 */
void blocking_assign_to(vhdl_process *proc, ivl_signal_t sig)
{
   std::string var(get_renamed_signal(sig));
   std::string tmpname(var + "_Var");
      
   if (g_assign_vars.find(var) == g_assign_vars.end()) {
      // This is the first time a non-blocking assignment
      // has been made to this signal: create a variable
      // to shadow it.
      if (!proc->have_declared_var(tmpname)) {
         vhdl_decl *decl = proc->get_parent()->get_decl(var);
         assert(decl);
         vhdl_type *type = new vhdl_type(*decl->get_type());
         
         proc->add_decl(new vhdl_var_decl(tmpname.c_str(), type));
      }
         
      rename_signal(sig, tmpname);
      g_assign_vars[tmpname] = sig;
   }
}

/*
 * Assign all _Var variables to the corresponding signals. This makes
 * the new values visible outside the current process. This should be
 * called before any `wait' statement or the end of the process.
 */
void draw_blocking_assigns(vhdl_process *proc)
{
   var_temp_set_t::const_iterator it;
   for (it = g_assign_vars.begin(); it != g_assign_vars.end(); ++it) {
      std::string stripped(strip_var((*it).first));

      vhdl_decl *decl = proc->get_decl(stripped);
      assert(decl);
      vhdl_type *type = new vhdl_type(*decl->get_type());
      
      vhdl_var_ref *lhs = new vhdl_var_ref(stripped.c_str(), NULL);
      vhdl_expr *rhs = new vhdl_var_ref((*it).first.c_str(), type);

      // TODO: I'm not sure this will work properly if, e.g., the delay
      // is inside a `if' statement
      proc->get_container()->add_stmt(new vhdl_nbassign_stmt(lhs, rhs));

      // Undo the renaming (since the temporary is no longer needed)
      rename_signal((*it).second, stripped);
   }

   g_assign_vars.clear();
}

/*
 * Remove _Var from the end of a string, if it is present.
 */
std::string strip_var(const std::string &str)
{
   std::string result(str);
   size_t pos = result.find("_Var");
   if (pos != std::string::npos)
      result.erase(pos, 4);
   return result;
}

/*
 * Convert a Verilog process to VHDL and add it to the architecture
 * of the given entity.
 */
static int generate_vhdl_process(vhdl_entity *ent, ivl_process_t proc)
{
   // Create a new process and store it in the entity's
   // architecture. This needs to be done first or the
   // parent link won't be valid (and draw_stmt needs this
   // to add information to the architecture)
   vhdl_process *vhdl_proc = new vhdl_process();
   ent->get_arch()->add_stmt(vhdl_proc);

   // If this is an initial process, push signal initialisation
   // into the declarations
   if (ivl_process_type(proc) == IVL_PR_INITIAL)
      vhdl_proc->set_initial(true);
   
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
   if (ivl_process_type(proc) == IVL_PR_INITIAL
       && !vhdl_proc->get_container()->empty()) {
      vhdl_wait_stmt *wait = new vhdl_wait_stmt();
      vhdl_proc->get_container()->add_stmt(wait);
   }
   
   // Add a comment indicating where it came from
   ivl_scope_t scope = ivl_process_scope(proc);
   const char *type = ivl_process_type(proc) == IVL_PR_INITIAL
      ? "initial" : "always";
   std::ostringstream ss;
   ss << "Generated from " << type << " process in ";
   ss << ivl_scope_tname(scope);
   vhdl_proc->set_comment(ss.str());

   // Output any remaning blocking assignments
   draw_blocking_assigns(vhdl_proc);

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
   if (ent->get_derived_from() == scope_name)
      return generate_vhdl_process(ent, proc);
   else
      return 0;
}
