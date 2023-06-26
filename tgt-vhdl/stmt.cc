/*
 *  VHDL code generation for statements.
 *
 *  Copyright (C) 2008-2023  Nick Gasson (nick@nickg.me.uk)
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
#include "state.hh"

#include <iostream>
#include <cstring>
#include <cassert>
#include <sstream>
#include <typeinfo>
#include <limits>
#include <set>
#include <algorithm>

using namespace std;

static void emit_wait_for_0(vhdl_procedural *proc, stmt_container *container,
                            ivl_statement_t stmt, vhdl_expr *expr);

/*
 * VHDL has no real equivalent of Verilog's $finish task. The
 * current solution is to use `assert false ...' to terminate
 * the simulator. This isn't great, as the simulator will
 * return a failure exit code when in fact it completed
 * successfully.
 *
 * An alternative is to use the VHPI interface supported by
 * some VHDL simulators and implement the $finish functionality
 * in C. This function can be enabled with the flag
 * -puse-vhpi-finish=1.
 */
static int draw_stask_finish(vhdl_procedural *, stmt_container *container,
                             ivl_statement_t)
{
   const char *use_vhpi = ivl_design_flag(get_vhdl_design(), "use-vhpi-finish");
   if (strcmp(use_vhpi, "1") == 0) {
      //get_active_entity()->requires_package("work.Verilog_Support");
      container->add_stmt(new vhdl_pcall_stmt("work.Verilog_Support.Finish"));
   }
   else {
      container->add_stmt(
         new vhdl_report_stmt(new vhdl_const_string("SIMULATION FINISHED"),
                              SEVERITY_FAILURE));
   }

   return 0;
}

static char parse_octal(const char *p)
{
   assert(*p && *(p+1) && *(p+2));
   assert(isdigit(*p) && isdigit(*(p+1)) && isdigit(*(p+2)));

   return (*p - '0') * 64
      + (*(p+1) - '0') * 8
      + (*(p+2) - '0') * 1;
}

// Generate VHDL report statements for Verilog $display/$write
static int draw_stask_display(vhdl_procedural *proc,
                              stmt_container *container,
                              ivl_statement_t stmt)
{
   vhdl_binop_expr *text = new vhdl_binop_expr(VHDL_BINOP_CONCAT,
                                               vhdl_type::string());

   const int count = ivl_stmt_parm_count(stmt);
   int i = 0;
   while (i < count) {
      // $display may have an empty parameter, in which case
      // the expression will be null
      // The behaviour here seems to be to output a space
      ivl_expr_t net = ivl_stmt_parm(stmt, i++);
      if (net == NULL) {
         text->add_expr(new vhdl_const_string(" "));
         continue;
      }

      if (ivl_expr_type(net) == IVL_EX_STRING) {
         ostringstream ss;
         for (const char *p = ivl_expr_string(net); *p; p++) {
            if (*p == '\\') {
               // Octal escape
               char ch = parse_octal(p+1);
               if (ch == '\n') {
                  // Is there a better way of handling newlines?
                  // Maybe generate another report statement
               }
               else
                  ss << ch;
               p += 3;
            }
            else if (*p == '%' && *(++p) != '%') {
               // Flush the output string up to this point
               text->add_expr(new vhdl_const_string(ss.str()));
               ss.str("");

               // Skip over width for now
               while (isdigit(*p)) ++p;

               switch (*p) {
               case 'm':
                  // TODO: we can get the module name via attributes
                  cerr << "Warning: no VHDL translation for %m format code"
                       << endl;
                  break;
               default:
                  {
                     assert(i < count);
                     ivl_expr_t netp = ivl_stmt_parm(stmt, i++);
                     assert(netp);

                     vhdl_expr *base = translate_expr(netp);
                     if (NULL == base)
                        return 1;

                     emit_wait_for_0(proc, container, stmt, base);

                     text->add_expr(base->cast(text->get_type()));
                  }
               }
            }
            else
               ss << *p;
         }

         // Emit any non-empty string data left in the buffer
         if (!ss.str().empty())
            text->add_expr(new vhdl_const_string(ss.str()));
      }
      else {
         vhdl_expr *base = translate_expr(net);
         if (NULL == base)
            return 1;

         emit_wait_for_0(proc, container, stmt, base);

         text->add_expr(base->cast(text->get_type()));
      }
   }

   if (count == 0)
      text->add_expr(new vhdl_const_string(""));

   container->add_stmt(new vhdl_report_stmt(text));
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
      return draw_stask_display(proc, container, stmt);
   else if (strcmp(name, "$write") == 0)
      return draw_stask_display(proc, container, stmt);
   else if (strcmp(name, "$finish") == 0)
      return draw_stask_finish(proc, container, stmt);
   else {
      vhdl_seq_stmt *result = new vhdl_null_stmt();
      ostringstream ss;
      ss << "Unsupported system task " << name << " omitted here ("
         << ivl_stmt_file(stmt) << ":" << ivl_stmt_lineno(stmt) << ")";
      result->set_comment(ss.str());
      container->add_stmt(result);
      cerr << "Warning: no VHDL translation for system task " << name << endl;
      return 0;
   }
}

/*
 * Generate VHDL for a block of Verilog statements. If this block
 * doesn't have its own scope then this function does nothing, other
 * than recursively translate the block's statements and add them
 * to the process. This is OK as the stmt_container class behaves
 * like a Verilog block.
 *
 * If this block has its own scope with local variables then these
 * are added to the process as local variables and the statements
 * are generated as above.
 */
static int draw_block(vhdl_procedural *proc, stmt_container *container,
                      ivl_statement_t stmt, bool is_last)
{
   ivl_scope_t block_scope = ivl_stmt_block_scope(stmt);
   if (block_scope) {
      int nsigs = ivl_scope_sigs(block_scope);
      for (int i = 0; i < nsigs; i++) {
         ivl_signal_t sig = ivl_scope_sig(block_scope, i);
         remember_signal(sig, proc->get_scope());

         vhdl_type* type = vhdl_type::type_for(ivl_signal_width(sig),
                                               ivl_signal_signed(sig));
         proc->get_scope()->add_decl
            (new vhdl_var_decl(make_safe_name(sig), type));
      }
   }

   int count = ivl_stmt_block_count(stmt);
   for (int i = 0; i < count; i++) {
      ivl_statement_t stmt_i = ivl_stmt_block_stmt(stmt, i);
      if (draw_stmt(proc, container, stmt_i, is_last && i == count - 1) != 0)
         return 1;
   }
   return 0;
}

/*
 * A no-op statement. This corresponds to a `null' statement in VHDL.
 */
static int draw_noop(vhdl_procedural *, stmt_container *container,
                     ivl_statement_t)
{
   container->add_stmt(new vhdl_null_stmt());
   return 0;
}

static vhdl_var_ref *make_assign_lhs(ivl_lval_t lval, vhdl_scope *scope)
{
   ivl_signal_t sig = ivl_lval_sig(lval);
   if (!sig) {
      error("Only signals as lvals supported at the moment");
      return NULL;
   }

   vhdl_expr *base = NULL;
   ivl_expr_t e_off = ivl_lval_part_off(lval);
   if (NULL == e_off)
      e_off = ivl_lval_idx(lval);
   if (e_off) {
      if ((base = translate_expr(e_off)) == NULL)
         return NULL;

      vhdl_type integer(VHDL_TYPE_INTEGER);
      base = base->cast(&integer);
   }

   unsigned lval_width = ivl_lval_width(lval);

   string signame(get_renamed_signal(sig));
   vhdl_decl *decl = scope->get_decl(signame);
   assert(decl);

   // Verilog allows assignments to elements that are constant in VHDL:
   // function parameters, for example
   // To work around this we generate a local variable to shadow the
   // constant and assign to that
   if (decl->assignment_type() == vhdl_decl::ASSIGN_CONST) {
      const string shadow_name = signame + "_Shadow";
      vhdl_var_decl* shadow_decl =
         new vhdl_var_decl(shadow_name, decl->get_type());
      shadow_decl->set_initial
         (new vhdl_var_ref(signame, decl->get_type()));
      scope->add_decl(shadow_decl);

      // Make sure all future references to this signal use the
      // shadow variable
      rename_signal(sig, shadow_name);

      // ...and use this new variable as the assignment LHS
      decl = shadow_decl;
   }

   vhdl_type *ltype = new vhdl_type(*decl->get_type());
   vhdl_var_ref *lval_ref = new vhdl_var_ref(decl->get_name(), ltype);
   if (base) {
      if (decl->get_type()->get_name() == VHDL_TYPE_ARRAY)
         lval_ref->set_slice(base, 0);
      else if (ivl_signal_width(sig) > 1)
         lval_ref->set_slice(base, lval_width - 1);
   }

   return lval_ref;
}

static bool assignment_lvals(ivl_statement_t stmt, vhdl_procedural *proc,
                             list<vhdl_var_ref*> &lvals)
{
   int nlvals = ivl_stmt_lvals(stmt);
   for (int i = 0; i < nlvals; i++) {
      ivl_lval_t lval = ivl_stmt_lval(stmt, i);
      vhdl_var_ref *lhs = make_assign_lhs(lval, proc->get_scope());
      if (NULL == lhs)
         return false;

      lvals.push_back(lhs);
   }

   return true;
}

/*
 * Generate the right sort of assignment statement for assigning
 * `lhs' to `rhs'.
 */
static vhdl_abstract_assign_stmt *
assign_for(vhdl_decl::assign_type_t atype, vhdl_var_ref *lhs, vhdl_expr *rhs)
{
   switch (atype) {
   case vhdl_decl::ASSIGN_BLOCK:
   case vhdl_decl::ASSIGN_CONST:
      return new vhdl_assign_stmt(lhs, rhs);
   case vhdl_decl::ASSIGN_NONBLOCK:
      return new vhdl_nbassign_stmt(lhs, rhs);
   }
   assert(false);
   return NULL;
}

/*
 * Check that this assignment type is valid within the context of `proc'.
 * For example, a <= assignment is not valid within a function.
 */
bool check_valid_assignment(vhdl_decl::assign_type_t atype, vhdl_procedural *proc,
                            ivl_statement_t stmt)
{
   if (atype == vhdl_decl::ASSIGN_NONBLOCK &&
       !proc->get_scope()->allow_signal_assignment()) {
      error("Unable to translate assignment at %s:%d\n"
            "  Translating this would require generating a non-blocking (<=)\n"
            "  assignment in a VHDL context where this is disallowed (e.g.\n"
            "  a function).", ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
      return false;
   }
   else
      return true;
}

// Generate a "wait for 0 ns" statement to emulate the behaviour of
// Verilog blocking assignment using VHDL signals. This is only generated
// if we read from the target of a blocking assignment in the same
// process (i.e. it is only generated when required, not for every
// blocking assignment). An example:
//
//   begin
//     x = 5;
//     if (x == 2)
//       y = 7;
//   end
//
// Becomes:
//
//   x <= 5;
//   wait for 0 ns;    -- Required to implement assignment semantics
//   if x = 2 then
//     y <= 7;         -- No need for wait here, not read
//   end if;
//
static void emit_wait_for_0(vhdl_procedural *proc,
                            stmt_container *container,
                            ivl_statement_t stmt,
                            vhdl_expr *expr)
{
   vhdl_var_set_t read;
   expr->find_vars(read);

   bool need_wait_for_0 = false;
   for (vhdl_var_set_t::const_iterator it = read.begin();
        it != read.end(); ++it) {
      if (proc->is_blocking_target(*it))
         need_wait_for_0 = true;
   }

   stmt_container::stmt_list_t &stmts = container->get_stmts();
   bool last_was_wait =
      !stmts.empty() && dynamic_cast<vhdl_wait_stmt*>(stmts.back());

   if (need_wait_for_0 && !last_was_wait) {
      debug_msg("Generated wait-for-0 for %s:%d",
                ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));

      vhdl_seq_stmt *wait = new vhdl_wait_stmt(VHDL_WAIT_FOR0);

      ostringstream ss;
      ss << "Read target of blocking assignment ("
         << ivl_stmt_file(stmt)
         << ":" << ivl_stmt_lineno(stmt) << ")";
      wait->set_comment(ss.str());

      container->add_stmt(wait);
      proc->added_wait_stmt();
   }
}

// Generate an assignment of type T for the Verilog statement stmt.
// If a statement was generated then `assign_type' will contain the
// type of assignment that was generated; this should be initialised
// to some sensible default.
void make_assignment(vhdl_procedural *proc, stmt_container *container,
                     ivl_statement_t stmt, bool emul_blocking,
                     vhdl_decl::assign_type_t& assign_type)
{
   list<vhdl_var_ref*> lvals;
   if (!assignment_lvals(stmt, proc, lvals))
      return;

   vhdl_expr *rhs, *rhs2 = NULL;
   ivl_expr_t rval = ivl_stmt_rval(stmt);
   if (ivl_expr_type(rval) == IVL_EX_TERNARY) {
      rhs = translate_expr(ivl_expr_oper2(rval));
      rhs2 = translate_expr(ivl_expr_oper3(rval));
      if (rhs2 == NULL)
         return;
   }
   else
      rhs = translate_expr(rval);
   if (rhs == NULL)
      return;

   emit_wait_for_0(proc, container, stmt, rhs);
   if (rhs2)
      emit_wait_for_0(proc, container, stmt, rhs2);

   if (lvals.size() == 1) {
      vhdl_var_ref *lhs = lvals.front();
      rhs = rhs->cast(lhs->get_type());

      ivl_expr_t i_delay;
      vhdl_expr *after = NULL;
      if ((i_delay = ivl_stmt_delay_expr(stmt)) != NULL) {
         after = translate_time_expr(i_delay);
         if (after == NULL)
            return;

         emit_wait_for_0(proc, container, stmt, after);
      }

      // Find the declaration of the LHS so we know what type
      // of assignment statement to generate (is it a signal,
      // a variable, etc?)
      vhdl_decl *decl = proc->get_scope()->get_decl(lhs->get_name());
      assign_type = decl->assignment_type();

      if (assign_type == vhdl_decl::ASSIGN_NONBLOCK && emul_blocking)
          proc->add_blocking_target(lhs);

      // A small optimisation is to expand ternary RHSs into an
      // if statement (eliminates a function call and produces
      // more idiomatic code)
      if (ivl_expr_type(rval) == IVL_EX_TERNARY) {
         rhs2 = rhs2->cast(lhs->get_type());
         vhdl_var_ref *lhs2 =
            make_assign_lhs(ivl_stmt_lval(stmt, 0), proc->get_scope());

         vhdl_expr *test = translate_expr(ivl_expr_oper1(rval));
         if (NULL == test)
            return;

         emit_wait_for_0(proc, container, stmt, test);

         if (!check_valid_assignment(decl->assignment_type(), proc, stmt))
            return;

         vhdl_if_stmt *vhdif = new vhdl_if_stmt(test);

         // True part
         {
            vhdl_abstract_assign_stmt *a =
               assign_for(decl->assignment_type(), lhs, rhs);
            if (after)
               a->set_after(after);
            vhdif->get_then_container()->add_stmt(a);
         }

         // False part
         {
            vhdl_abstract_assign_stmt *a =
               assign_for(decl->assignment_type(), lhs2, rhs2);
            if (after)
               a->set_after(translate_time_expr(i_delay));
            vhdif->get_else_container()->add_stmt(a);
         }

         container->add_stmt(vhdif);
         return;
      }

      // Where possible, move constant assignments into the
      // declaration as initialisers. This optimisation is only
      // performed on assignments of constant values to prevent
      // ordering problems.

      // This also has another application: If this is an `initial'
      // process and we haven't yet generated a `wait' statement then
      // moving the assignment to the initialization preserves the
      // expected Verilog behaviour: VHDL does not distinguish
      // `initial' and `always' processes so an `always' process might
      // be activated before an `initial' process at time 0. The
      // `always' process may then use the uninitialised signal value.
      // The second test ensures that we only try to initialise
      // internal signals not ports
      ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
      if (proc->get_scope()->initializing()
          && ivl_signal_port(ivl_lval_sig(lval)) == IVL_SIP_NONE
          && !decl->has_initial()
          && rhs->constant()
          && decl->get_type()->get_name() != VHDL_TYPE_ARRAY) {

         // If this assignment is not in the top-level container
         // it will not be made on all paths through the code
         // This precludes any future extraction of an initialiser
         if (container != proc->get_container())
            decl->set_initial(NULL);   // Default initial value
         else {
            decl->set_initial(rhs);
            proc->get_scope()->hoisted_initialiser(true);
            delete lhs;
            return;
         }
      }

      if (!check_valid_assignment(decl->assignment_type(), proc, stmt))
         return;

      vhdl_abstract_assign_stmt *a =
         assign_for(decl->assignment_type(), lhs, rhs);
      container->add_stmt(a);

      a->set_after(after);
   }
   else {
      // Multiple lvals are implemented by first assigning the complete
      // RHS to a temporary, and then assigning each lval in turn as
      // bit-selects of the temporary

      static int tmp_count = 0;
      ostringstream ss;
      ss << "Verilog_Assign_Tmp_" << tmp_count++;

      vhdl_decl* tmp_decl = new vhdl_var_decl(ss.str(), rhs->get_type());
      proc->get_scope()->add_decl(tmp_decl);

      container->add_stmt(new vhdl_assign_stmt(tmp_decl->make_ref(), rhs));

      list<vhdl_var_ref*>::iterator it;
      int width_so_far = 0;
      for (it = lvals.begin(); it != lvals.end(); ++it) {
         vhdl_var_ref *tmp_rhs = tmp_decl->make_ref();

         int lval_width = (*it)->get_type()->get_width();
         vhdl_expr *slice_base = new vhdl_const_int(width_so_far);
         tmp_rhs->set_slice(slice_base, lval_width - 1);

         ivl_expr_t i_delay;
         vhdl_expr *after = NULL;
         if ((i_delay = ivl_stmt_delay_expr(stmt)) != NULL) {
            after = translate_time_expr(i_delay);
            if (after == NULL)
               return;

            emit_wait_for_0(proc, container, stmt, after);
         }

         // Find the declaration of the LHS so we know what type
         // of assignment statement to generate (is it a signal,
         // a variable, etc?)
         vhdl_decl *decl = proc->get_scope()->get_decl((*it)->get_name());
         assign_type = decl->assignment_type();

         if (!check_valid_assignment(decl->assignment_type(), proc, stmt))
            return;

         vhdl_abstract_assign_stmt *a =
            assign_for(decl->assignment_type(), *it, tmp_rhs);
         if (after)
            a->set_after(after);

         container->add_stmt(a);

         width_so_far += lval_width;

         if (assign_type == vhdl_decl::ASSIGN_NONBLOCK && emul_blocking)
            proc->add_blocking_target(*it);
      }
   }
}

/*
 * A non-blocking assignment inside a process. The semantics for
 * this are essentially the same as VHDL's non-blocking signal
 * assignment.
 */
static int draw_nbassign(vhdl_procedural *proc, stmt_container *container,
                         ivl_statement_t stmt)
{
   assert(proc->get_scope()->allow_signal_assignment());

   vhdl_decl::assign_type_t ignored;
   make_assignment(proc, container, stmt, false, ignored);

   return 0;
}

static int draw_assign(vhdl_procedural *proc, stmt_container *container,
                       ivl_statement_t stmt)
{
   vhdl_decl::assign_type_t assign_type = vhdl_decl::ASSIGN_NONBLOCK;
   bool emulate_blocking = proc->get_scope()->allow_signal_assignment();

   make_assignment(proc, container, stmt, emulate_blocking, assign_type);

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
      time = scale_time(get_active_entity(), value);
   }
   else {
      time = translate_time_expr(ivl_stmt_delay_expr(stmt));
      if (NULL == time)
         return 1;
   }

   ivl_statement_t sub_stmt = ivl_stmt_sub_stmt(stmt);
   vhdl_wait_stmt *wait =
      new vhdl_wait_stmt(VHDL_WAIT_FOR, time);

   // Remember that we needed a wait statement so if this is
   // a process it cannot have a sensitivity list
   proc->added_wait_stmt();

   container->add_stmt(wait);

   // Expand the sub-statement as well
   // Often this would result in a useless `null' statement which
   // is caught here instead
   if (ivl_statement_type(sub_stmt) != IVL_ST_NOOP)
      draw_stmt(proc, container, sub_stmt);

   // Any further assignments occur after simulation time 0
   // so they cannot be used to initialise signal declarations
   // (if this scope is an initial process)
   proc->get_scope()->set_initializing(false);

   return 0;
}

/*
 * Build a set of all the nexuses referenced by signals in `expr'.
 */
static void get_nexuses_from_expr(ivl_expr_t expr, set<ivl_nexus_t> &out)
{
   switch (ivl_expr_type(expr)) {
   case IVL_EX_SIGNAL:
      out.insert(ivl_signal_nex(ivl_expr_signal(expr), 0));
      break;
   case IVL_EX_TERNARY:
      get_nexuses_from_expr(ivl_expr_oper3(expr), out);
      // fallthrough
   case IVL_EX_BINARY:
      get_nexuses_from_expr(ivl_expr_oper2(expr), out);
      // fallthrough
   case IVL_EX_UNARY:
      get_nexuses_from_expr(ivl_expr_oper1(expr), out);
      break;
   default:
      break;
   }
}

/*
 * Attempt to identify common forms of wait statements and produce
 * more idiomatic VHDL than would be produced by the generic
 * draw_wait function. The main application of this is a input to
 * synthesis tools that don't synthesise the full VHDL language.
 * If none of these patterns are matched, the function returns false
 * and the default draw_wait is used.
 *
 * Current patterns:
 *   always @(posedge A or posedge B)
 *     if (A)
 *        ...
 *     else
 *        ...
 *
 *   This is assumed to be the template for a FF with asynchronous
 *   reset. A is assumed to be the reset as it is dominant. This will
 *   produce the following VHDL:
 *
 *   process (A, B) is
 *   begin
 *     if A = '1' then
 *       ...
 *     else if rising_edge(B) then
 *       ...
 *     end if;
 *   end process;
 */
static bool draw_synthesisable_wait(vhdl_process *proc, stmt_container *container,
                                    ivl_statement_t stmt)
{
   // At the moment this only detects FFs with an asynchronous reset
   // All other code will fall back on the default draw_wait

   // Store a set of the edge triggered signals
   // The second item is true if this is positive-edge
   set<ivl_nexus_t> edge_triggered;

   const int nevents = ivl_stmt_nevent(stmt);

   for (int i = 0; i < nevents; i++) {
      ivl_event_t event = ivl_stmt_events(stmt, i);

      if (ivl_event_nany(event) > 0)
         return false;

      int npos = ivl_event_npos(event);
      for (int j = 0; j < npos; j++)
         edge_triggered.insert(ivl_event_pos(event, j));

      int nneg = ivl_event_nneg(event);
      for (int j = 0; j < nneg; j++)
         edge_triggered.insert(ivl_event_neg(event, j));
   }

   // If we're edge-sensitive to less than two signals this doesn't
   // match the expected template, so use the default draw_wait
   if (edge_triggered.size() < 2)
      return false;

   // Now check to see if the immediately embedded statement is an `if'
   ivl_statement_t sub_stmt = ivl_stmt_sub_stmt(stmt);
   if (ivl_statement_type(sub_stmt) != IVL_ST_CONDIT)
      return false;

   // The if should have two branches: one is the reset branch and
   // one is the clocked branch
   if (ivl_stmt_cond_false(sub_stmt) == NULL)
      return false;

   // Check the first branch of the if statement
   // If it matches exactly one of the edge-triggered signals then assume
   // this is the (dominant) reset branch
   set<ivl_nexus_t> test_nexuses;
   get_nexuses_from_expr(ivl_stmt_cond_expr(sub_stmt), test_nexuses);

   // If the test is not a simple function of one variable then this
   // template will not work
   if (test_nexuses.size() != 1)
      return false;

   // Now subtracting this set from the set of edge triggered events
   // should leave just one nexus, which is hopefully the clock.
   // If not, then we fall back on the default draw_wait
   set<ivl_nexus_t> clock_net;
   set_difference(edge_triggered.begin(), edge_triggered.end(),
                  test_nexuses.begin(), test_nexuses.end(),
                  inserter(clock_net, clock_net.begin()));

   if (clock_net.size() != 1)
      return false;

   // Build a VHDL `if' statement to model this
   vhdl_expr *reset_test = translate_expr(ivl_stmt_cond_expr(sub_stmt));
   vhdl_if_stmt *body = new vhdl_if_stmt(reset_test);

   // Draw the reset branch
   draw_stmt(proc, body->get_then_container(), ivl_stmt_cond_true(sub_stmt));

   // Build a test for the clock event
   vhdl_fcall *edge = NULL;
   ivl_nexus_t the_clock_net = *clock_net.begin();
   for (int i = 0; i < nevents; i++) {
      ivl_event_t event = ivl_stmt_events(stmt, i);

      const unsigned npos = ivl_event_npos(event);
      for (unsigned j = 0; j < npos; j++) {
         if (ivl_event_pos(event, j) == the_clock_net)
            edge = new vhdl_fcall("rising_edge", vhdl_type::boolean());
      }

      const unsigned nneg = ivl_event_nneg(event);
      for (unsigned j = 0; j < nneg; j++)
         if (ivl_event_neg(event, j) == the_clock_net)
            edge = new vhdl_fcall("falling_edge", vhdl_type::boolean());
   }
   assert(edge);

   edge->add_expr(nexus_to_var_ref(proc->get_scope(), *clock_net.begin()));

   // Draw the clocked branch
   // For an asynchronous reset we just want this around the else branch,
   stmt_container *else_container = body->add_elsif(edge);

   draw_stmt(proc, else_container, ivl_stmt_cond_false(sub_stmt));

   if (proc->contains_wait_stmt()) {
      // Expanding the body produced a `wait' statement which can't
      // be included in a sensitised process so undo all this work
      // and fall back on the default draw_wait
      delete body;
      return false;
   }
   else
      container->add_stmt(body);

   // Add all the edge triggered signals to the sensitivity list
   for (set<ivl_nexus_t>::const_iterator it = edge_triggered.begin();
        it != edge_triggered.end(); ++it) {
      // Get the signal that represents this nexus in this scope
      vhdl_var_ref *ref = nexus_to_var_ref(proc->get_scope(), *it);

      proc->add_sensitivity(ref->get_name());

      // Don't need the reference any more
      delete ref;
   }

   // Don't bother with the default draw_wait
   return true;
}

/*
 * A wait statement waits for a level change on a @(..) list of
 * signals. The logic here might seem a little bit convoluted,
 * it attempts to always produce something that will simulate
 * correctly, and tries to produce something that will also
 * synthesise correctly (although not at the expense of simulation
 * accuracy).
 *
 * The difficulty stems from VHDL's restriction that a process with
 * a sensitivity list may not contain any `wait' statements: we need
 * to generate these to accurately model some Verilog statements.
 *
 * The steps followed are:
 *  1) Determine whether this is the top-level statement in the process
 *  2) If this is top-level, call draw_synthesisable_wait to see if the
 *     process and wait statement match any templates for which we know
 *     how to produce good, idiomatic synthesisable VHDL (e.g. FF with
 *     async reset)
 *  3) Determine whether the process is combinatorial (purely level
 *     sensitive), or sequential (edge sensitive)
 *  4) Draw all of the statements in the body
 *  5) One of the following will be true:
 *     A) The process is combinatorial, top-level, and there are
 *        no `wait' statements in the body: add all the level-sensitive
 *        signals to the VHDL sensitivity list
 *     B) The process is combinatorial, and there *are* `wait'
 *        statements in the body or it is not top-level: generate
 *        a VHDL `wait-on' statement at the end of the body containing
 *        the level-sensitive signals
 *     C) The process is sequential, top-level, and there are
 *        no `wait' statements in the body: build an `if' statement
 *        with the edge-detecting expression and wrap the process
 *        in it.
 *     D) The process is sequential, there *are* `wait' statements
 *        in the body, or it is not top-level: generate a VHDL
 *        `wait-until' with the edge-detecting expression and add
 *        it before the body of the wait event.
 */
static int draw_wait(vhdl_procedural *_proc, stmt_container *container,
                     ivl_statement_t stmt)
{
   // Wait statements only occur in processes
   vhdl_process *proc = dynamic_cast<vhdl_process*>(_proc);
   assert(proc);   // Catch not process

   // If this container is the top-level statement (i.e. it is the
   // first thing inside a process) then we can extract these
   // events out into the sensitivity list as long as we haven't
   // promoted any preceding assignments to initialisers
   bool is_top_level =
      container == proc->get_container()
      && container->empty()
      && !proc->get_scope()->hoisted_initialiser();

   // See if this can be implemented in a more idiomatic way before we
   // fall back on the generic translation
   if (is_top_level && draw_synthesisable_wait(proc, container, stmt))
      return 0;

   int nevents = ivl_stmt_nevent(stmt);

   bool combinatorial = true;  // True if no negedge/posedge events
   for (int i = 0; i < nevents; i++) {
      ivl_event_t event = ivl_stmt_events(stmt, i);
      if (ivl_event_npos(event) > 0 || ivl_event_nneg(event) > 0)
         combinatorial = false;
   }

   if (combinatorial) {
      // If the process has no wait statement in its body then
      // add all the events to the sensitivity list, otherwise
      // build a wait-on statement at the end of the process

      draw_stmt(proc, container, ivl_stmt_sub_stmt(stmt), true);

      vhdl_wait_stmt *wait = NULL;
      if (proc->contains_wait_stmt() || !is_top_level)
         wait = new vhdl_wait_stmt(VHDL_WAIT_ON);

      for (int i = 0; i < nevents; i++) {
         ivl_event_t event = ivl_stmt_events(stmt, i);

         int nany = ivl_event_nany(event);
         for (int j = 0; j < nany; j++) {
            ivl_nexus_t nexus = ivl_event_any(event, j);
            vhdl_var_ref *ref = nexus_to_var_ref(proc->get_scope(), nexus);

            if (wait)
               wait->add_sensitivity(ref->get_name());
            else
               proc->add_sensitivity(ref->get_name());
            delete ref;
         }
      }

      if (wait)
         container->add_stmt(wait);
   }
   else {
      // Build a test expression to represent the edge event
      // If this process contains no `wait' statements and this
      // is the top-level container, then we
      // wrap it in an `if' statement with this test and add the
      // edge triggered signals to the sensitivity, otherwise
      // build a `wait until' statement at the top of the process
      vhdl_binop_expr *test =
         new vhdl_binop_expr(VHDL_BINOP_OR, vhdl_type::boolean());

      stmt_container tmp_container;
      draw_stmt(proc, &tmp_container, ivl_stmt_sub_stmt(stmt), true);

      for (int i = 0; i < nevents; i++) {
         ivl_event_t event = ivl_stmt_events(stmt, i);

         int nany = ivl_event_nany(event);
         for (int j = 0; j < nany; j++) {
            ivl_nexus_t nexus = ivl_event_any(event, j);
            vhdl_var_ref *ref = nexus_to_var_ref(proc->get_scope(), nexus);

            ref->set_name(ref->get_name() + "'Event");
            test->add_expr(ref);

            if (!proc->contains_wait_stmt() && is_top_level)
               proc->add_sensitivity(ref->get_name());
         }

         int nneg = ivl_event_nneg(event);
         for (int j = 0; j < nneg; j++) {
            ivl_nexus_t nexus = ivl_event_neg(event, j);
            vhdl_var_ref *ref = nexus_to_var_ref(proc->get_scope(), nexus);
            vhdl_fcall *detect =
               new vhdl_fcall("falling_edge", vhdl_type::boolean());
            detect->add_expr(ref);

            test->add_expr(detect);

            if (!proc->contains_wait_stmt() && is_top_level)
               proc->add_sensitivity(ref->get_name());
         }

         int npos = ivl_event_npos(event);
         for (int j = 0; j < npos; j++) {
            ivl_nexus_t nexus = ivl_event_pos(event, j);
            vhdl_var_ref *ref = nexus_to_var_ref(proc->get_scope(), nexus);
            vhdl_fcall *detect =
               new vhdl_fcall("rising_edge", vhdl_type::boolean());
            detect->add_expr(ref);

            test->add_expr(detect);

            if (!proc->contains_wait_stmt() && is_top_level)
               proc->add_sensitivity(ref->get_name());
         }
      }

      if (proc->contains_wait_stmt() || !is_top_level) {
         container->add_stmt(new vhdl_wait_stmt(VHDL_WAIT_UNTIL, test));
         container->move_stmts_from(&tmp_container);
      }
      else {
         // Wrap the whole process body in an `if' statement to detect
         // the edge event
         vhdl_if_stmt *edge_detect = new vhdl_if_stmt(test);

         // Move all the statements from the process body into the `if'
         // statement
         edge_detect->get_then_container()->move_stmts_from(&tmp_container);

         container->add_stmt(edge_detect);
      }

   }

   return 0;
}

static int draw_if(vhdl_procedural *proc, stmt_container *container,
                   ivl_statement_t stmt, bool is_last)
{
   vhdl_expr *test = translate_expr(ivl_stmt_cond_expr(stmt));
   if (NULL == test)
      return 1;

   emit_wait_for_0(proc, container, stmt, test);

   vhdl_if_stmt *vhdif = new vhdl_if_stmt(test);
   container->add_stmt(vhdif);

   ivl_statement_t cond_true_stmt = ivl_stmt_cond_true(stmt);
   if (cond_true_stmt)
      draw_stmt(proc, vhdif->get_then_container(), cond_true_stmt, is_last);

   ivl_statement_t cond_false_stmt = ivl_stmt_cond_false(stmt);
   if (cond_false_stmt)
      draw_stmt(proc, vhdif->get_else_container(), cond_false_stmt, is_last);

   return 0;
}

static vhdl_var_ref *draw_case_test(vhdl_procedural *proc, stmt_container *container,
                                    ivl_statement_t stmt)
{
   vhdl_expr *test = translate_expr(ivl_stmt_cond_expr(stmt));
   if (NULL == test)
      return NULL;

   // VHDL case expressions are required to be quite simple: variable
   // references or slices. So we may need to create a temporary
   // variable to hold the result of the expression evaluation
   if (typeid(*test) != typeid(vhdl_var_ref)) {
      const char *tmp_name = "Verilog_Case_Ex";
      vhdl_type *test_type = new vhdl_type(*test->get_type());

      if (!proc->get_scope()->have_declared(tmp_name)) {
         proc->get_scope()->add_decl
            (new vhdl_var_decl(tmp_name, new vhdl_type(*test_type)));
      }

      vhdl_var_ref *tmp_ref = new vhdl_var_ref(tmp_name, NULL);
      container->add_stmt(new vhdl_assign_stmt(tmp_ref, test));

      return new vhdl_var_ref(tmp_name, test_type);
   }
   else
      return dynamic_cast<vhdl_var_ref*>(test);
}

static int draw_case(vhdl_procedural *proc, stmt_container *container,
                     ivl_statement_t stmt, bool is_last)
{
   vhdl_var_ref *test = draw_case_test(proc, container, stmt);
   if (NULL == test)
      return 1;

   vhdl_case_stmt *vhdlcase = new vhdl_case_stmt(test);
   container->add_stmt(vhdlcase);

   // VHDL is more strict than Verilog about covering every
   // possible case. So make sure we add an 'others' branch
   // if there isn't a default one.
   bool have_others = false;

   int nbranches = ivl_stmt_case_count(stmt);
   for (int i = 0; i < nbranches; i++) {
      vhdl_expr *when;
      ivl_expr_t net = ivl_stmt_case_expr(stmt, i);
      if (net) {
         when = translate_expr(net)->cast(test->get_type());
         if (NULL == when)
            return 1;
      }
      else {
         when = new vhdl_var_ref("others", NULL);
         have_others = true;
      }

      vhdl_case_branch *branch = new vhdl_case_branch(when);
      vhdlcase->add_branch(branch);

      ivl_statement_t stmt_i = ivl_stmt_case_stmt(stmt, i);
      draw_stmt(proc, branch->get_container(), stmt_i, is_last);
   }

   if (!have_others) {
      vhdl_case_branch *others =
         new vhdl_case_branch(new vhdl_var_ref("others", NULL));
      others->get_container()->add_stmt(new vhdl_null_stmt());
      vhdlcase->add_branch(others);
   }

   return 0;
}

/*
 * Check to see if the given number (expression) can be represented
 * accurately in a long value.
 */
static bool number_is_long(ivl_expr_t expr)
{
   ivl_expr_type_t type = ivl_expr_type(expr);

   assert(type == IVL_EX_NUMBER || type == IVL_EX_ULONG);

   // Make sure the ULONG can be represented correctly in a long.
   if (type == IVL_EX_ULONG) {
      unsigned long val = ivl_expr_uvalue(expr);
      if (val > static_cast<unsigned>(numeric_limits<long>::max())) {
         return false;
      }
      return true;
   }

   // Check to see if the number actually fits in a long.
   unsigned nbits = ivl_expr_width(expr);
   if (nbits >= 8*sizeof(long)) {
      const char*bits = ivl_expr_bits(expr);
      char pad_bit = bits[nbits-1];
      for (unsigned idx = 8*sizeof(long); idx < nbits; idx++) {
         if (bits[idx] != pad_bit) return false;
      }
   }

   return true;
}

/*
 * Return the given number (expression) as a signed long value.
 *
 * Make sure to call number_is_long() first to verify that the number
 * can be represented accurately in a long value.
 */
static long get_number_as_long(ivl_expr_t expr)
{
   long imm = 0;
   switch (ivl_expr_type(expr)) {
   case IVL_EX_ULONG:
      imm = ivl_expr_uvalue(expr);
      break;

   case IVL_EX_NUMBER: {
      const char*bits = ivl_expr_bits(expr);
      unsigned nbits = ivl_expr_width(expr);
      if (nbits > 8*sizeof(long)) nbits = 8*sizeof(long);
      for (unsigned idx = 0; idx < nbits; idx++) {
         switch (bits[idx]) {
         case '0':
            break;
         case '1':
            imm |= 1L << idx;
            break;
         default:
            assert(0);
         }

         if (ivl_expr_signed(expr) && bits[nbits-1] == '1' &&
             nbits < 8*sizeof(long)) imm |= -1UL << nbits;
      }
      break;
   }

   default:
      assert(0);
   }
   return imm;
}

/*
 * Build a check against a constant 'x'. This is for an out of range
 * or undefined select.
 */
static void check_against_x(vhdl_binop_expr *all, vhdl_var_ref *test,
                            ivl_expr_t expr, unsigned width, unsigned base,
                            bool is_casez)
{
   if (is_casez) {
      // For a casez we need to check against 'x'.
      for (unsigned i = 0; i < ivl_expr_width(expr); i++) {
         vhdl_binop_expr *sub_expr =
            new vhdl_binop_expr(VHDL_BINOP_OR, vhdl_type::boolean());
         vhdl_type *type;
         vhdl_var_ref *ref;

         // Check if the test bit is 'z'.
         type = vhdl_type::nunsigned(width);
         ref = new vhdl_var_ref(test->get_name().c_str(), type);
         ref->set_slice(new vhdl_const_int(i+base));
         vhdl_binop_expr *cmp =
            new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
         cmp->add_expr(ref);
         cmp->add_expr(new vhdl_const_bit('z'));
         sub_expr->add_expr(cmp);

         // Compare the test bit against a constant 'x'.
         type = vhdl_type::nunsigned(width);
         ref = new vhdl_var_ref(test->get_name().c_str(), type);
         ref->set_slice(new vhdl_const_int(i+base));
         cmp = new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
         cmp->add_expr(ref);
         cmp->add_expr(new vhdl_const_bit('x'));
         sub_expr->add_expr(cmp);

         all->add_expr(sub_expr);
      }
   } else {
      // For a casex 'x' is a don't care, so just put 'true'.
      all->add_expr(new vhdl_const_bool(true));
   }
}

/*
 * Build the test signal to constant bits check.
 */
static void process_number(vhdl_binop_expr *all, vhdl_var_ref *test,
                           ivl_expr_t expr, unsigned width, unsigned base,
                           bool is_casez)
{
   const char *bits = ivl_expr_bits(expr);

   bool just_dont_care = true;
   for (unsigned i = 0; i < ivl_expr_width(expr); i++) {
      switch (bits[i]) {
      case 'x':
         if (is_casez) break;
         // fallthrough
      case '?':
      case 'z':
         continue;  // Ignore these.
      }

      vhdl_binop_expr *sub_expr =
         new vhdl_binop_expr(VHDL_BINOP_OR, vhdl_type::boolean());
      vhdl_type *type;
      vhdl_var_ref *ref;

      // Check if the test bit is 'z'.
      type = vhdl_type::nunsigned(width);
      ref = new vhdl_var_ref(test->get_name().c_str(), type);
      ref->set_slice(new vhdl_const_int(i+base));
      vhdl_binop_expr *cmp =
         new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
      cmp->add_expr(ref);
      cmp->add_expr(new vhdl_const_bit('z'));
      sub_expr->add_expr(cmp);

      // If this is a casex statement check if the test bit is 'x'.
      if (!is_casez) {
         type = vhdl_type::nunsigned(width);
         ref = new vhdl_var_ref(test->get_name().c_str(), type);
         ref->set_slice(new vhdl_const_int(i+base));
         cmp = new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
         cmp->add_expr(ref);
         cmp->add_expr(new vhdl_const_bit('x'));
         sub_expr->add_expr(cmp);
      }

      // Compare the bit against the constant value.
      type = vhdl_type::nunsigned(width);
      ref = new vhdl_var_ref(test->get_name().c_str(), type);
      ref->set_slice(new vhdl_const_int(i+base));
      cmp = new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
      cmp->add_expr(ref);
      cmp->add_expr(new vhdl_const_bit(bits[i]));
      sub_expr->add_expr(cmp);

      all->add_expr(sub_expr);
      just_dont_care = false;
   }

   // If there are no bits comparisons then just put a True
   if (just_dont_care) {
      all->add_expr(new vhdl_const_bool(true));
   }
}

/*
 * Build the test signal to label signal check.
 */
static bool process_signal(vhdl_binop_expr *all, vhdl_var_ref *test,
                           ivl_expr_t expr, unsigned width, unsigned base,
                           bool is_casez, unsigned swid, long sbase)
{
   // If the word or dimensions are not zero then we have an array.
   if (ivl_expr_oper1(expr) != 0 ||
       ivl_signal_dimensions(ivl_expr_signal(expr)) != 0) {
      error("Sorry, array selects are not currently allowed in this "
            "context.");
      return true;
   }

   unsigned ewid = ivl_expr_width(expr);
   if (sizeof(unsigned) >= sizeof(long)) {
      // Since we will be casting i (constrained by swid) to a long make sure
      // it will fit into a long. This is actually off by one, but this is the
      // best we can do since on 32 bit machines an unsigned and long are the
      // same size.
      assert(swid <= static_cast<unsigned>(numeric_limits<long>::max()));
      // We are also going to cast ewid to long so check it as well.
      assert(ewid <= static_cast<unsigned>(numeric_limits<long>::max()));
   }
   for (unsigned i = 0; i < swid; i++) {
      // Generate a comparison for this bit position
      vhdl_binop_expr *cmp;
      vhdl_type *type;
      vhdl_var_ref *ref;

      // Check if this is an out of bounds access. If this is a casez
      // then check against a constant 'x' for the out of bound bits
      // otherwise skip the check (casex).
      if (static_cast<long>(i) + sbase >= static_cast<long>(ewid) ||
          static_cast<long>(i) + sbase < 0) {
         if (is_casez) {
            // Get the current test bit.
            type = vhdl_type::nunsigned(width);
            ref = new vhdl_var_ref(test->get_name().c_str(), type);
            ref->set_slice(new vhdl_const_int(i+base));

            // Compare the bit against 'x'.
            cmp = new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
            cmp->add_expr(ref);
            cmp->add_expr(new vhdl_const_bit('x'));
            all->add_expr(cmp);
            continue;
         } else {
            // The compiler replaces a completely out of range select
            // with a constant so we know there will be at least one
            // valid bit here. We don't need a just_dont_care test.
            continue;
         }
      }

      vhdl_binop_expr *sub_expr =
         new vhdl_binop_expr(VHDL_BINOP_OR, vhdl_type::boolean());
      vhdl_var_ref *bit;

      // Get the current expression bit.
      // Why can we reuse the expression bit, but not the condition bit?
      type = vhdl_type::nunsigned(ivl_expr_width(expr));
      bit = new vhdl_var_ref(ivl_expr_name(expr), type);
      bit->set_slice(new vhdl_const_int(i+sbase));

      // Check if the expression bit is 'z'.
      cmp = new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
      cmp->add_expr(bit);
      cmp->add_expr(new vhdl_const_bit('z'));
      sub_expr->add_expr(cmp);

      // If this is a casex statement check if the expression bit is 'x'.
      if (!is_casez) {
         cmp = new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
         cmp->add_expr(bit);
         cmp->add_expr(new vhdl_const_bit('x'));
         sub_expr->add_expr(cmp);
      }

      // Check if the test bit is 'z'.
      type = vhdl_type::nunsigned(width);
      ref = new vhdl_var_ref(test->get_name().c_str(), type);
      ref->set_slice(new vhdl_const_int(i+base));
      cmp = new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
      cmp->add_expr(ref);
      cmp->add_expr(new vhdl_const_bit('z'));
      sub_expr->add_expr(cmp);

      // If this is a casex statement check if the test bit is 'x'.
      if (!is_casez) {
        type = vhdl_type::nunsigned(width);
         ref = new vhdl_var_ref(test->get_name().c_str(), type);
         ref->set_slice(new vhdl_const_int(i+base));
         cmp = new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
         cmp->add_expr(ref);
         cmp->add_expr(new vhdl_const_bit('x'));
         sub_expr->add_expr(cmp);
      }

      // Next check if the test and expression bits are equal.
      type = vhdl_type::nunsigned(width);
      ref = new vhdl_var_ref(test->get_name().c_str(), type);
      ref->set_slice(new vhdl_const_int(i+base));
      cmp = new vhdl_binop_expr(VHDL_BINOP_EQ, vhdl_type::boolean());
      cmp->add_expr(ref);
      cmp->add_expr(bit);
      sub_expr->add_expr(cmp);

      all->add_expr(sub_expr);
   }

   return false;
}

/*
 * These are the constructs that we allow in a casex/z label
 * expression. Returns true on failure.
 */
static bool process_expr_bits(vhdl_binop_expr *all, vhdl_var_ref *test,
                             ivl_expr_t expr, unsigned width, unsigned base,
                             bool is_casez)
{
   assert(ivl_expr_width(expr)+base <= width);

   switch (ivl_expr_type(expr)) {
   case IVL_EX_CONCAT:
      // Loop repeat number of times processing each sub element.
      for (unsigned repeat = 0; repeat < ivl_expr_repeat(expr); repeat++) {
         unsigned nparms = ivl_expr_parms(expr) - 1;
         for (unsigned parm = 0; parm <= nparms; parm++) {
            ivl_expr_t pexpr = ivl_expr_parm(expr, nparms-parm);
            if (process_expr_bits(all, test, pexpr, width, base, is_casez))
               return true;
            base += ivl_expr_width(pexpr);
         }
      }
      break;

   case IVL_EX_NUMBER:
      process_number(all, test, expr, width, base, is_casez);
      break;

   case IVL_EX_SIGNAL:
      if (process_signal(all, test, expr, width, base, is_casez,
                         ivl_expr_width(expr), 0)) return true;
      break;

   case IVL_EX_SELECT: {
      ivl_expr_t bexpr = ivl_expr_oper2(expr);
      if (ivl_expr_type(bexpr) != IVL_EX_NUMBER &&
          ivl_expr_type(bexpr) != IVL_EX_ULONG) {
         error("Sorry, only constant bit/part selects are currently allowed "
               "in this context.");
         return true;
      }
      // If the number is out of bounds or an 'x' then check against 'x'.
      if (!number_is_long(bexpr)) {
         check_against_x(all, test, expr, width, base, is_casez);
      } else if (process_signal(all, test, ivl_expr_oper1(expr), width, base,
                                is_casez, ivl_expr_width(expr),
                                get_number_as_long(bexpr))) return true;
      break;
      }

   default:
      error("Sorry, expression type %d is not currently supported.",
            ivl_expr_type(expr));
      return true;
      break;
   }

   return false;
}


/*
 * A casex/z statement cannot be directly translated to a VHDL case
 * statement as VHDL does not treat the don't-care bit as special.
 * The solution here is to generate an if statement from the casex/z
 * which compares only the non-don't-care bit positions.
 */
int draw_casezx(vhdl_procedural *proc, stmt_container *container,
                ivl_statement_t stmt, bool is_last)
{
   vhdl_var_ref *test = draw_case_test(proc, container, stmt);
   if (NULL == test)
      return 1;

   vhdl_if_stmt *result = NULL;

   int nbranches = ivl_stmt_case_count(stmt);
   bool is_casez = ivl_statement_type(stmt) == IVL_ST_CASEZ;
   for (int i = 0; i < nbranches; i++) {
      stmt_container *where = NULL;

      ivl_expr_t net = ivl_stmt_case_expr(stmt, i);
      if (net) {
         vhdl_binop_expr *all =
            new vhdl_binop_expr(VHDL_BINOP_AND, vhdl_type::boolean());
         // The net must be something we can generate a comparison for.
         if (process_expr_bits(all, test, net, ivl_expr_width(net), 0,
                               is_casez)) {
            error("%s:%d: Sorry, only case%s statements with simple "
                  "expression labels can be translated to VHDL",
                  ivl_stmt_file(stmt), ivl_stmt_lineno(stmt),
                  (is_casez ? "z" : "x"));
            delete all;
            return 1;
         }

         if (result)
            where = result->add_elsif(all);
         else {
            result = new vhdl_if_stmt(all);
            where = result->get_then_container();
         }
      }
      else {
         // This the default case and therefore the `else' branch
         assert(result);
         where = result->get_else_container();
      }

      // `where' now points to a branch of an if statement which
      // corresponds to this casex/z branch
      assert(where);
      draw_stmt(proc, where, ivl_stmt_case_stmt(stmt, i), is_last);
   }

   // Add a comment to say that this corresponds to a casex/z statement
   // as this may not be obvious
   ostringstream ss;
   ss << "Generated from case"
      << (is_casez ? 'z' : 'x')
      << " statement at " << ivl_stmt_file(stmt) << ":" << ivl_stmt_lineno(stmt);
   result->set_comment(ss.str());

   container->add_stmt(result);

   // We don't actually use the generated `test' expression
   delete test;

   return 0;
}

int draw_while(vhdl_procedural *proc, stmt_container *container,
               ivl_statement_t stmt, ivl_statement_t step=0)
{
   // Generate the body inside a temporary container before
   // generating the test
   // The reason for this is that some of the signals in the
   // test might be renamed while expanding the body (e.g. if
   // we need to generate an assignment to a constant signal)
   stmt_container tmp_container;
   int rc = draw_stmt(proc, &tmp_container, ivl_stmt_sub_stmt(stmt));
   if (rc != 0)
      return 1;
   // When we are emitting a for as a while we need to add the step
   if (step) {
      rc = draw_assign(proc, &tmp_container, step);
      if (rc != 0)
         return rc;
   }

   vhdl_expr *test = translate_expr(ivl_stmt_cond_expr(stmt));
   if (NULL == test)
      return 1;

   // The test must be a Boolean (and std_logic and (un)signed types
   // must be explicitly cast unlike in Verilog)
   vhdl_type boolean(VHDL_TYPE_BOOLEAN);
   test = test->cast(&boolean);

   emit_wait_for_0(proc, container, stmt, test);

   vhdl_while_stmt *loop = new vhdl_while_stmt(test);
   draw_stmt(proc, loop->get_container(), ivl_stmt_sub_stmt(stmt));

   // When we are emitting a for as a while we need to add the step
   if (step) {
      rc = draw_assign(proc, loop->get_container(), step);
      if (rc != 0)
         return rc;
   }

   emit_wait_for_0(proc, loop->get_container(), stmt, test);

   container->add_stmt(loop);
   return 0;
}

int draw_for_loop(vhdl_procedural *proc, stmt_container *container,
                  ivl_statement_t stmt)
{
   int rc = draw_assign(proc, container, ivl_stmt_init_stmt(stmt));
   if (rc != 0)
      return rc;

   return draw_while(proc, container, stmt, ivl_stmt_step_stmt(stmt));
}

int draw_forever(vhdl_procedural *proc, stmt_container *container,
                 ivl_statement_t stmt)
{
   vhdl_loop_stmt *loop = new vhdl_loop_stmt;
   container->add_stmt(loop);

   draw_stmt(proc, loop->get_container(), ivl_stmt_sub_stmt(stmt));

   return 0;
}

int draw_repeat(vhdl_procedural *proc, stmt_container *container,
                ivl_statement_t stmt)
{
   vhdl_expr *times = translate_expr(ivl_stmt_cond_expr(stmt));
   if (NULL == times)
      return 1;

   vhdl_type integer(VHDL_TYPE_INTEGER);
   times = times->cast(&integer);

   const char *it_name = "Verilog_Repeat";
   vhdl_for_stmt *loop =
      new vhdl_for_stmt(it_name, new vhdl_const_int(1), times);
   container->add_stmt(loop);

   draw_stmt(proc, loop->get_container(), ivl_stmt_sub_stmt(stmt));

   return 0;
}

/*
 * Tasks are difficult to translate to VHDL since they allow things
 * not allowed by VHDL's corresponding procedures (e.g. updating
 * global variables. The solution here is to expand tasks in-line.
 */
int draw_utask(vhdl_procedural *proc, stmt_container *container,
               ivl_statement_t stmt)
{
   ivl_scope_t tscope = ivl_stmt_call(stmt);

   // TODO: adding some comments to the output would be helpful

   // TODO: this completely ignores parameters!
   draw_stmt(proc, container, ivl_scope_def(tscope), false);

   return 0;
}

/*
 * Generate VHDL statements for the given Verilog statement and
 * add them to the given VHDL process. The container is the
 * location to add statements: e.g. the process body, a branch
 * of an if statement, etc.
 *
 * The flag is_last should be set if this is the final statement
 * in a block or process. It avoids generating useless `wait for 0ns'
 * statements if the next statement would be a wait anyway.
 */
int draw_stmt(vhdl_procedural *proc, stmt_container *container,
              ivl_statement_t stmt, bool is_last)
{
   assert(stmt);

   switch (ivl_statement_type(stmt)) {
   case IVL_ST_STASK:
      return draw_stask(proc, container, stmt);
   case IVL_ST_BLOCK:
      return draw_block(proc, container, stmt, is_last);
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
      return draw_if(proc, container, stmt, is_last);
   case IVL_ST_CASE:
      return draw_case(proc, container, stmt, is_last);
   case IVL_ST_WHILE:
      return draw_while(proc, container, stmt);
   case IVL_ST_FORLOOP:
      return draw_for_loop(proc, container, stmt);
   case IVL_ST_FOREVER:
      return draw_forever(proc, container, stmt);
   case IVL_ST_REPEAT:
      return draw_repeat(proc, container, stmt);
   case IVL_ST_UTASK:
      return draw_utask(proc, container, stmt);
   case IVL_ST_FORCE:
   case IVL_ST_RELEASE:
      error("force/release statements cannot be translated to VHDL");
      return 1;
   case IVL_ST_DISABLE:
      error("disable statement cannot be translated to VHDL");
      return 1;
   case IVL_ST_CASEX:
   case IVL_ST_CASEZ:
      return draw_casezx(proc, container, stmt, is_last);
   case IVL_ST_FORK:
      error("fork statement cannot be translated to VHDL");
      return 1;
   case IVL_ST_CASSIGN:
   case IVL_ST_DEASSIGN:
      error("continuous procedural assignment cannot be translated to VHDL");
      return 1;
   default:
      error("No VHDL translation for statement at %s:%d (type = %d)",
            ivl_stmt_file(stmt), ivl_stmt_lineno(stmt),
            ivl_statement_type(stmt));
      return 1;
   }
}
