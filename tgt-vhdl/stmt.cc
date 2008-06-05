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
static int draw_stask_display(vhdl_process *proc, ivl_statement_t stmt)
{
   // Add the package requirement to the containing entity
   proc->get_parent()->get_parent()->requires_package("std.textio");

   const char *display_line = "Verilog_Display_Line";
   
   if (!proc->have_declared_var(display_line)) {
      vhdl_type *line_type = new vhdl_scalar_type("Line");
      vhdl_var_decl *line_var =
         new vhdl_var_decl(display_line, line_type);
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
         // TODO: Need to add a call to Type'Image for types not
         // supported by std.textio
         e = translate_expr(net);
         if (NULL == e)
            return 1;
      }
      else
         e = new vhdl_const_string(" ");

      vhdl_pcall_stmt *write = new vhdl_pcall_stmt("Write");
      write->add_expr(new vhdl_var_ref(display_line));
      write->add_expr(e);

      proc->add_stmt(write);
   }

   // WriteLine(Output, Verilog_Display_Line)
   vhdl_pcall_stmt *write_line = new vhdl_pcall_stmt("WriteLine");
   write_line->add_expr(new vhdl_var_ref("std.textio.Output"));
   write_line->add_expr(new vhdl_var_ref(display_line));
   proc->add_stmt(write_line);
   
   return 0;
}

/*
 * Generate VHDL for system tasks (like $display). Not all of
 * these are supported.
 */
static int draw_stask(vhdl_process *proc, ivl_statement_t stmt)
{
   const char *name = ivl_stmt_name(stmt);

   if (strcmp(name, "$display") == 0)
      return draw_stask_display(proc, stmt);
   else {
      error("No VHDL translation for system task %s", name);
      return 0;
   }
}

/*
 * Generate VHDL for a block of Verilog statements. This doesn't
 * actually do anything, other than recursively translate the
 * block's statements and add them to the process. This is OK as
 * `begin' and `end process' function like a Verilog block.
 */
static int draw_block(vhdl_process *proc, ivl_statement_t stmt)
{
   int count = ivl_stmt_block_count(stmt);
   for (int i = 0; i < count; i++) {
      if (draw_stmt(proc, ivl_stmt_block_stmt(stmt, i)) != 0)
         return 1;
   }
   return 0;
}

/*
 * A no-op statement. This corresponds to a `null' statement in
 * VHDL.
 */
static int draw_noop(vhdl_process *proc, ivl_statement_t stmt)
{
   proc->add_stmt(new vhdl_null_stmt());
   return 0;
}

/*
 * Generate VHDL statements for the given Verilog statement and
 * add them to the given VHDL process.
 */
int draw_stmt(vhdl_process *proc, ivl_statement_t stmt)
{
   switch (ivl_statement_type(stmt)) {
   case IVL_ST_STASK:
      return draw_stask(proc, stmt);
   case IVL_ST_BLOCK:
      return draw_block(proc, stmt);
   case IVL_ST_NOOP:
      return draw_noop(proc, stmt);
   default:
      error("No VHDL translation for statement at %s:%d (type = %d)",
            ivl_stmt_file(stmt), ivl_stmt_lineno(stmt),
            ivl_statement_type(stmt));
      return 1;            
   }
}
