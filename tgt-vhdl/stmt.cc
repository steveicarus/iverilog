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
#include <typeinfo>

/*
 * VHDL has no real equivalent of Verilog's $finish task. The
 * current solution is to use `assert false ...' to terminate
 * the simulator. This isn't great, as the simulator will
 * return a failure exit code when in fact it completed
 * successfully.
 *
 * An alternative is to use the VHPI interface supported by
 * some VHDL simulators and implement the $finish funcitonality
 * in C. This function can be enabled with the flag
 * -puse-vhpi-finish=1. 
 */
static int draw_stask_finish(vhdl_procedural *proc, stmt_container *container,
                             ivl_statement_t stmt)
{
   const char *use_vhpi = ivl_design_flag(get_vhdl_design(), "use-vhpi-finish");
   if (strcmp(use_vhpi, "1") == 0) {
      //get_active_entity()->requires_package("work.Verilog_Support");
      container->add_stmt(new vhdl_pcall_stmt("work.Verilog_Support.Finish"));
   }
   else {
      container->add_stmt(new vhdl_assert_stmt("SIMULATION FINISHED"));
   }
   
   return 0;
}

/*
 * Generate VHDL for system tasks (like $display). Not all of
 * these are supported.
 */
static int draw_stask(vhdl_procedural *proc, stmt_container *container,
                      ivl_statement_t stmt)
{
   const char *name = ivl_stmt_name(stmt);

   if (strcmp(name, "$display") == 0)
      return draw_stask_display(proc, container, stmt, true);
   else if (strcmp(name, "$write") == 0)
      return draw_stask_display(proc, container, stmt, false);
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
static int draw_block(vhdl_procedural *proc, stmt_container *container,
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
static int draw_noop(vhdl_procedural *proc, stmt_container *container,
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
static int draw_nbassign(vhdl_procedural *proc, stmt_container *container,
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

      vhdl_decl *decl = proc->get_scope()->get_decl(signame);
      assert(decl);

      vhdl_expr *rhs_raw = translate_expr(ivl_stmt_rval(stmt));
      if (NULL == rhs_raw)
         return 1;
      vhdl_expr *rhs = rhs_raw->cast(decl->get_type());

      // Where possible, move constant assignments into the
      // declaration as initializers. This optimisation is only
      // performed on assignments of constant values to prevent
      // ordering problems.
      
      // This also has another application: If this is an `inital'
      // process and we haven't yet generated a `wait' statement then
      // moving the assignment to the initialization preserves the
      // expected Verilog behaviour: VHDL does not distinguish
      // `initial' and `always' processes so an `always' process might
      // be activatated before an `initial' process at time 0. The
      // `always' process may then use the uninitialized signal value.
      // The second test ensures that we only try to initialise
      // internal signals not ports
      if (proc->get_scope()->initializing()
          && ivl_signal_port(sig) == IVL_SIP_NONE
          && !decl->has_initial() && rhs->constant()) {

         decl->set_initial(rhs);
      }
      else {
         // The type here can be null as it is never actually needed
         vhdl_var_ref *lval_ref = new vhdl_var_ref(signame, NULL);
         
         vhdl_nbassign_stmt *assign = new vhdl_nbassign_stmt(lval_ref, rhs);
         if (after != NULL)
            assign->set_after(after);
         container->add_stmt(assign);
      }
   }
   else {
      error("Only signals as lvals supported at the moment");
      return 1;
   }
   
   return 0;
}

static int draw_assign(vhdl_procedural *proc, stmt_container *container,
                       ivl_statement_t stmt)
{
   int nlvals = ivl_stmt_lvals(stmt);
   if (nlvals != 1) {
      error("Can only have 1 lval at the moment (found %d)", nlvals);
      return 1;
   }

   ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
   ivl_signal_t sig;
   if ((sig = ivl_lval_sig(lval))) {
      const std::string signame(get_renamed_signal(sig));

      vhdl_decl *decl = proc->get_scope()->get_decl(signame);
      assert(decl);

      vhdl_expr *rhs_raw = translate_expr(ivl_stmt_rval(stmt));
      if (NULL == rhs_raw)
         return 1;
      vhdl_expr *rhs = rhs_raw->cast(decl->get_type());

      bool isvar = strip_var(signame) != signame;

      // As with non-blocking assignment, push constant assignments
      // into the initialisation if we can (but only if this is
      // the first time we assign to this variable).
      if (proc->get_scope()->initializing()
          && ivl_signal_port(sig) == IVL_SIP_NONE
          && rhs->constant() && !decl->has_initial()
          && !isvar) {

         decl->set_initial(rhs);

         // This signal may be used e.g. in a loop test so we need
         // to make a variable as well
         blocking_assign_to(proc, sig);

         // The signal may have been renamed by the above call
         const std::string &renamed = get_renamed_signal(sig);
         
         vhdl_var_ref *lval_ref = new vhdl_var_ref(renamed.c_str(), NULL);
         vhdl_var_ref *sig_ref = new vhdl_var_ref(signame.c_str(), NULL);
         
         vhdl_assign_stmt *assign = new vhdl_assign_stmt(lval_ref, sig_ref);
         container->add_stmt(assign);
      }
      else {
         blocking_assign_to(proc, sig);

         // The signal may have been renamed by the above call
         const std::string &renamed = get_renamed_signal(sig);
         
         // The type here can be null as it is never actually needed
         vhdl_var_ref *lval_ref = new vhdl_var_ref(renamed.c_str(), NULL);
         
         vhdl_assign_stmt *assign = new vhdl_assign_stmt(lval_ref, rhs);
         container->add_stmt(assign);
      }
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
static int draw_delay(vhdl_procedural *proc, stmt_container *container,
                      ivl_statement_t stmt)
{
   // This currently ignores the time units and precision
   // of the enclosing scope
   // A neat way to do this would be to make these values
   // constants in the scope (type is Time), and have the
   // VHDL wait statement compute the value from that.
   // The other solution is to add them as parameters to
   // the vhdl_process class
   vhdl_expr *time;
   if (ivl_statement_type(stmt) == IVL_ST_DELAY) {
      uint64_t value = ivl_stmt_delay_val(stmt);
      time = new vhdl_const_time(value, TIME_UNIT_NS);
   }
   else {
      time = translate_expr(ivl_stmt_delay_expr(stmt));
      if (NULL == time)
         return 1;

      vhdl_type integer(VHDL_TYPE_INTEGER);
      time = time->cast(&integer);

      vhdl_expr *ns1 = new vhdl_const_time(1, TIME_UNIT_NS);
      time = new vhdl_binop_expr(time, VHDL_BINOP_MULT, ns1,
                                 vhdl_type::time());
   }

   // If the sub-statement is an assignment then VHDL lets
   // us put the delay after it, which is more compact and
   // idiomatic
   ivl_statement_t sub_stmt = ivl_stmt_sub_stmt(stmt);
   ivl_statement_type_t type = ivl_statement_type(sub_stmt);
   if (type == IVL_ST_ASSIGN_NB) {
      draw_nbassign(proc, container, sub_stmt, time);
   }
   else {
      // All blocking assignments need to be made visible
      // at this point
      draw_blocking_assigns(proc, container);
      
      vhdl_wait_stmt *wait =
         new vhdl_wait_stmt(VHDL_WAIT_FOR, time);
      container->add_stmt(wait);

      // Expand the sub-statement as well
      // Often this would result in a useless `null' statement which
      // is caught here instead
      if (ivl_statement_type(sub_stmt) != IVL_ST_NOOP)
         draw_stmt(proc, container, sub_stmt);
   }
   
   // Any further assignments occur after simulation time 0
   // so they cannot be used to initialize signal declarations
   // (if this scope is an initial process)
   proc->get_scope()->set_initializing(false);
   
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
   vhdl_var_ref *ref = nexus_to_var_ref(proc->get_scope()->get_parent(), nexus);
   vhdl_fcall *detect = new vhdl_fcall(type, vhdl_type::boolean());
   detect->add_expr(ref);
   test->add_expr(detect);
   proc->add_sensitivity(ref->get_name().c_str());
}

/*
 * A wait statement waits for a level change on a @(..) list of
 * signals.
 */
static int draw_wait(vhdl_procedural *_proc, stmt_container *container,
                     ivl_statement_t stmt)
{
   // Wait statements only occur in processes
   vhdl_process *proc = dynamic_cast<vhdl_process*>(_proc);
   assert(proc);   // Catch not process
   
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
               if (proc->get_scope()->get_parent()->have_declared(signame)) {
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

static int draw_if(vhdl_procedural *proc, stmt_container *container,
                   ivl_statement_t stmt)
{
   vhdl_expr *test = translate_expr(ivl_stmt_cond_expr(stmt));
   if (NULL == test)
      return 1;
   
   vhdl_if_stmt *vhdif = new vhdl_if_stmt(test);

   draw_stmt(proc, vhdif->get_then_container(),
             ivl_stmt_cond_true(stmt));

   ivl_statement_t cond_false_stmt = ivl_stmt_cond_false(stmt);
   if (cond_false_stmt)
      draw_stmt(proc, vhdif->get_else_container(), cond_false_stmt);

   container->add_stmt(vhdif);
   
   return 0;
}

static int draw_case(vhdl_procedural *proc, stmt_container *container,
                     ivl_statement_t stmt)
{
   vhdl_expr *test = translate_expr(ivl_stmt_cond_expr(stmt));
   if (NULL == test)
      return 1;

   // VHDL case expressions are required to be quite simple: variable
   // references or slices. So we may need to create a temporary
   // variable to hold the result of the expression evaluation
   if (typeid(test) != typeid(vhdl_var_ref)) {
      // TODO: Check if this is already declared
      const char *tmp_name = "Verilog_Case_Ex";
      vhdl_type *test_type = new vhdl_type(*test->get_type());
      proc->get_scope()->add_decl(new vhdl_var_decl(tmp_name, test_type));

      vhdl_var_ref *tmp_ref = new vhdl_var_ref(tmp_name, NULL);
      container->add_stmt(new vhdl_assign_stmt(tmp_ref, test));

      test = new vhdl_var_ref(tmp_name, new vhdl_type(*test_type));
   }
   
   vhdl_case_stmt *vhdlcase = new vhdl_case_stmt(test);
   container->add_stmt(vhdlcase);
   
   int nbranches = ivl_stmt_case_count(stmt);
   for (int i = 0; i < nbranches; i++) {
      vhdl_expr *when;
      ivl_expr_t net = ivl_stmt_case_expr(stmt, i);
      if (net) {
         when = translate_expr(net);
         if (NULL == when)
            return 1;
      }
      else
         when = new vhdl_var_ref("others", NULL);      
      
      vhdl_case_branch *branch = new vhdl_case_branch(when);
      vhdlcase->add_branch(branch);
      
      draw_stmt(proc, branch->get_container(), ivl_stmt_case_stmt(stmt, i));
   }
   
   return 0;
}

int draw_while(vhdl_procedural *proc, stmt_container *container,
               ivl_statement_t stmt)
{
   vhdl_expr *test = translate_expr(ivl_stmt_cond_expr(stmt));
   if (NULL == test)
      return 1;

   vhdl_while_stmt *loop = new vhdl_while_stmt(test);
   container->add_stmt(loop);

   draw_stmt(proc, loop->get_container(), ivl_stmt_sub_stmt(stmt));
   
   return 0;
}

/*
 * Generate VHDL statements for the given Verilog statement and
 * add them to the given VHDL process. The container is the
 * location to add statements: e.g. the process body, a branch
 * of an if statement, etc.
 */
int draw_stmt(vhdl_procedural *proc, stmt_container *container,
              ivl_statement_t stmt)
{
   assert(stmt);
   
   switch (ivl_statement_type(stmt)) {
   case IVL_ST_STASK:
      return draw_stask(proc, container, stmt);
   case IVL_ST_BLOCK:
      return draw_block(proc, container, stmt);
   case IVL_ST_NOOP:
      return draw_noop(proc, container, stmt);
   case IVL_ST_ASSIGN:
      return draw_assign(proc, container, stmt);
   case IVL_ST_ASSIGN_NB:
      return draw_nbassign(proc, container, stmt);
   case IVL_ST_DELAY:
   case IVL_ST_DELAYX:
      return draw_delay(proc, container, stmt);
   case IVL_ST_WAIT:
      return draw_wait(proc, container, stmt);
   case IVL_ST_CONDIT:
      return draw_if(proc, container, stmt);
   case IVL_ST_CASE:
      return draw_case(proc, container, stmt);
   case IVL_ST_WHILE:
      return draw_while(proc, container, stmt);
   default:
      error("No VHDL translation for statement at %s:%d (type = %d)",
            ivl_stmt_file(stmt), ivl_stmt_lineno(stmt),
            ivl_statement_type(stmt));
      return 1;            
   }
}
