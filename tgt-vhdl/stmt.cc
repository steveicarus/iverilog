/*
 *  VHDL code generation for statements.
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

#include <iostream>
#include <cstring>
#include <cassert>
#include <sstream>

/*
 * Generate VHDL for the $display system task.
 * This is implemented using the functions in std.textio. Each
 * parameter is written to a line variable in the process and
 * then the line is written to the special variable `Output'
 * (which represents the console). Subsequent $displays will
 * use the same line variable.
 *
 * It's possible, although quite unlikely, that there will be
 * name collision with an existing variable called
 * `Verilog_Display_Line' -- do something about this?
 */
static int draw_stask_display(vhdl_process *proc, stmt_container *container,
                              ivl_statement_t stmt)
{
   // Add the package requirement to the containing entity
   proc->get_parent()->get_parent()->requires_package("std.textio");

   const char *display_line = "Verilog_Display_Line";
   
   if (!proc->have_declared_var(display_line)) {
      vhdl_var_decl *line_var =
         new vhdl_var_decl(display_line, vhdl_type::line());
      line_var->set_comment("For generating $display output");
      proc->add_decl(line_var);
   }
    
   // Write the data into the line
   int count = ivl_stmt_parm_count(stmt);
   for (int i = 0; i < count; i++) {
      // $display may have an empty parameter, in which case
      // the expression will be null
      // The behaviour here seems to be to output a space
      ivl_expr_t net = ivl_stmt_parm(stmt, i);
      vhdl_expr *e = NULL;
      if (net) {
         vhdl_expr *base = translate_expr(net);
         if (NULL == base)
            return 1;
         
         // Need to add a call to Type'Image for types not
         // supported by std.textio
         if (base->get_type()->get_name() != VHDL_TYPE_STRING) {
            std::string name(base->get_type()->get_string());
            name += "'Image";
            
            vhdl_fcall *cast
               = new vhdl_fcall(name.c_str(), vhdl_type::string());
            cast->add_expr(base);
            e = cast;
         }
         else
            e = base;
      }
      else
         e = new vhdl_const_string(" ");

      vhdl_pcall_stmt *write = new vhdl_pcall_stmt("Write");
      vhdl_var_ref *ref =
         new vhdl_var_ref(display_line, vhdl_type::line());
      write->add_expr(ref);
      write->add_expr(e);

      container->add_stmt(write);
   }

   // WriteLine(Output, Verilog_Display_Line)
   vhdl_pcall_stmt *write_line = new vhdl_pcall_stmt("WriteLine");
   vhdl_var_ref *output_ref =
      new vhdl_var_ref("std.textio.Output", new vhdl_type(VHDL_TYPE_FILE));
   write_line->add_expr(output_ref);
   vhdl_var_ref *ref =
      new vhdl_var_ref(display_line, vhdl_type::line());
   write_line->add_expr(ref);
   container->add_stmt(write_line);
   
   return 0;
}

/*
 * VHDL has no real equivalent of Verilog's $finish task. The
 * current solution is to use `assert false ...' to terminate
 * the simulator. This isn't great, as the simulator will
 * return a failure exit code when in fact it completed
 * successfully.
 */
static int draw_stask_finish(vhdl_process *proc, stmt_container *container,
                             ivl_statement_t stmt)
{
   container->add_stmt(new vhdl_assert_stmt("SIMULATION FINISHED"));
   return 0;
}

/*
 * Generate VHDL for system tasks (like $display). Not all of
 * these are supported.
 */
static int draw_stask(vhdl_process *proc, stmt_container *container,
                      ivl_statement_t stmt)
{
   const char *name = ivl_stmt_name(stmt);

   if (strcmp(name, "$display") == 0)
      return draw_stask_display(proc, container, stmt);
   else if (strcmp(name, "$finish") == 0)
      return draw_stask_finish(proc, container, stmt);
   else {
      error("No VHDL translation for system task %s", name);
      return 0;
   }
}

/*
 * Generate VHDL for a block of Verilog statements. This doesn't
 * actually do anything, other than recursively translate the
 * block's statements and add them to the process. This is OK as
 * the stmt_container class behaves like a Verilog block.
 */
static int draw_block(vhdl_process *proc, stmt_container *container,
                      ivl_statement_t stmt)
{
   int count = ivl_stmt_block_count(stmt);
   for (int i = 0; i < count; i++) {
      if (draw_stmt(proc, container, ivl_stmt_block_stmt(stmt, i)) != 0)
         return 1;
   }
   return 0;
}

/*
 * A no-op statement. This corresponds to a `null' statement in
 * VHDL.
 */
static int draw_noop(vhdl_process *proc, stmt_container *container,
                     ivl_statement_t stmt)
{
   container->add_stmt(new vhdl_null_stmt());
   return 0;
}

/*
 * A non-blocking assignment inside a process. The semantics for
 * this are essentially the same as VHDL's non-blocking signal
 * assignment.
 */
static int draw_nbassign(vhdl_process *proc, stmt_container *container,
                         ivl_statement_t stmt, vhdl_expr *after = NULL)
{
   int nlvals = ivl_stmt_lvals(stmt);
   if (nlvals != 1) {
      error("Can only have 1 lval at the moment (found %d)", nlvals);
      return 1;
   }

   ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
   ivl_signal_t sig;
   if ((sig = ivl_lval_sig(lval))) {
      const char *signame = get_renamed_signal(sig).c_str();

      vhdl_decl *decl = proc->get_parent()->get_decl(signame);
      assert(decl);

      vhdl_expr *rhs_raw = translate_expr(ivl_stmt_rval(stmt));
      if (NULL == rhs_raw)
         return 1;
      vhdl_expr *rhs = rhs_raw->cast(decl->get_type());

      // The type here can be null as it is never actually needed
      vhdl_var_ref *lval_ref = new vhdl_var_ref(signame, NULL);

      vhdl_nbassign_stmt *nbassign = new vhdl_nbassign_stmt(lval_ref, rhs);
      if (after != NULL)
         nbassign->set_after(after);
      container->add_stmt(nbassign);
   }
   else {
      error("Only signals as lvals supported at the moment");
      return 1;
   }
   
   return 0;
}

/*
 * Delay statements are equivalent to the `wait for' form of the
 * VHDL wait statement.
 */
static int draw_delay(vhdl_process *proc, stmt_container *container,
                      ivl_statement_t stmt)
{
   uint64_t value = ivl_stmt_delay_val(stmt);

   // This currently ignores the time units and precision
   // of the enclosing scope
   // A neat way to do this would be to make these values
   // constants in the scope (type is Time), and have the
   // VHDL wait statement compute the value from that.
   // The other solution is to add them as parameters to
   // the vhdl_process class
   vhdl_expr *time = new vhdl_const_int(value);

   // If the sub-statement is an assignment then VHDL lets
   // us put the delay after it, which is more compact and
   // idiomatic
   ivl_statement_t sub_stmt = ivl_stmt_sub_stmt(stmt);
   ivl_statement_type_t type = ivl_statement_type(sub_stmt);
   if (type == IVL_ST_ASSIGN_NB) {
      draw_nbassign(proc, container, sub_stmt, time);
   }
   else {
      vhdl_wait_stmt *wait =
         new vhdl_wait_stmt(VHDL_WAIT_FOR_NS, time);
      container->add_stmt(wait);

      // Expand the sub-statement as well
      // Often this would result in a useless `null' statement which
      // is caught here instead
      if (ivl_statement_type(sub_stmt) != IVL_ST_NOOP)
         draw_stmt(proc, container, sub_stmt);
   
   }
   
   return 0;
}

/*
 * Make edge detectors from the signals in `nexus' and add them
 * to the expression `test'. Also adds the signals to the process
 * sensitivity list. Type should be one of `rising_edge' or
 * `falling_edge'.
 */
static void edge_detector(ivl_nexus_t nexus, vhdl_process *proc,
                          vhdl_binop_expr *test, const char *type)
{
   int nptrs = ivl_nexus_ptrs(nexus);
   for (int j = 0; j < nptrs; j++) {
      ivl_nexus_ptr_t nexus_ptr = ivl_nexus_ptr(nexus, j);
      
      ivl_signal_t sig;
      if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
         const char *signame = get_renamed_signal(sig).c_str();
         
         vhdl_fcall *detect
            = new vhdl_fcall(type, vhdl_type::boolean());
         detect->add_expr
            (new vhdl_var_ref(signame, vhdl_type::std_logic()));
         test->add_expr(detect);
         proc->add_sensitivity(signame);
         break;
      }
   }
}


/*
 * A wait statement waits for a level change on a @(..) list of
 * signals.
 */
static int draw_wait(vhdl_process *proc, stmt_container *container,
                     ivl_statement_t stmt)
{
   ivl_statement_t sub_stmt = ivl_stmt_sub_stmt(stmt);
   
   int nevents = ivl_stmt_nevent(stmt);
   for (int i = 0; i < nevents; i++) {
      ivl_event_t event = ivl_stmt_events(stmt, i);

      // A list of the non-edge triggered signals to they can
      // be added to the edge-detecting `if' statement later
      string_list_t non_edges;

      int nany = ivl_event_nany(event);
      for (int i = 0; i < nany; i++) {
         ivl_nexus_t nexus = ivl_event_any(event, i);

         int nptrs = ivl_nexus_ptrs(nexus);
         for (int j = 0; j < nptrs; j++) {
            ivl_nexus_ptr_t nexus_ptr = ivl_nexus_ptr(nexus, j);

            ivl_signal_t sig;
            if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
               const char *signame = ivl_signal_basename(sig);

               // Only add this signal to the sensitivity if it's part
               // of the containing architecture (i.e. it has already
               // been declared)
               if (proc->get_parent()->have_declared(signame)) {
                  proc->add_sensitivity(signame);
                  non_edges.push_back(signame);
                  break;
               }
            }
            else {
               // Ignore all other types of nexus pointer
            }
         }
      }

      int nneg = ivl_event_nneg(event);
      int npos = ivl_event_npos(event);
      if (nneg + npos > 0) {
         vhdl_binop_expr *test =
            new vhdl_binop_expr(VHDL_BINOP_OR, vhdl_type::boolean());

         // Generate falling_edge(..) calls for each negedge event
         for (int i = 0; i < nneg; i++)
            edge_detector(ivl_event_neg(event, i), proc, test, "falling_edge");
        
         // Generate rising_edge(..) calls for each posedge event
         for (int i = 0; i < npos; i++)
            edge_detector(ivl_event_pos(event, i), proc, test, "rising_edge");
        
         // Add Name'Event terms for each non-edge-triggered signal
         string_list_t::iterator it;
         for (it = non_edges.begin(); it != non_edges.end(); ++it) {
            test->add_expr
               (new vhdl_var_ref((*it + "'Event").c_str(),
                                 vhdl_type::boolean()));
         }

         vhdl_if_stmt *edge_det = new vhdl_if_stmt(test);
         container->add_stmt(edge_det);
         
         draw_stmt(proc, edge_det->get_then_container(), sub_stmt);
      }
      else {
         // Don't bother generating an edge detector if there
         // are no edge-triggered events
         draw_stmt(proc, container, sub_stmt);
      }
   }
   
   return 0;
}

static int draw_if(vhdl_process *proc, stmt_container *container,
                   ivl_statement_t stmt)
{
   vhdl_expr *test = translate_expr(ivl_stmt_cond_expr(stmt));
   if (NULL == test)
      return 1;
   
   vhdl_if_stmt *vhdif = new vhdl_if_stmt(test);

   draw_stmt(proc, vhdif->get_then_container(),
             ivl_stmt_cond_true(stmt));
   draw_stmt(proc, vhdif->get_else_container(),
             ivl_stmt_cond_false(stmt));

   container->add_stmt(vhdif);
   
   return 0;
}

/*
 * Generate VHDL statements for the given Verilog statement and
 * add them to the given VHDL process. The container is the
 * location to add statements: e.g. the process body, a branch
 * of an if statement, etc.
 */
int draw_stmt(vhdl_process *proc, stmt_container *container,
              ivl_statement_t stmt)
{
   switch (ivl_statement_type(stmt)) {
   case IVL_ST_STASK:
      return draw_stask(proc, container, stmt);
   case IVL_ST_BLOCK:
      return draw_block(proc, container, stmt);
   case IVL_ST_NOOP:
      return draw_noop(proc, container, stmt);
   case IVL_ST_ASSIGN_NB:
      return draw_nbassign(proc, container, stmt);
   case IVL_ST_DELAY:
      return draw_delay(proc, container, stmt);
   case IVL_ST_WAIT:
      return draw_wait(proc, container, stmt);
   case IVL_ST_CONDIT:
      return draw_if(proc, container, stmt);
   default:
      error("No VHDL translation for statement at %s:%d (type = %d)",
            ivl_stmt_file(stmt), ivl_stmt_lineno(stmt),
            ivl_statement_type(stmt));
      return 1;            
   }
}
