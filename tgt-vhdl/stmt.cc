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

/*
 * Generate VHDL for the $display system task.
 * This is implemented using the functions in std.textio. Each
 * parameter is written to a line variable in the process and
 * then the line is written to the special variable `Output'
 * (which represents the console). Subsequent $displays will use
 * the same line variable.
 *
 * It's possible, although quite unlikely, that there will be
 * name collision with an existing variable called
 * `Verilog_Display_Line' - do something about this? It's also
 * possible for there to be a name collision with the special
 * variable `Output'.
 */
static int draw_stask_display(vhdl_process *proc, ivl_statement_t stmt)
{
   require_package("std.textio");
   
   return 0;
}

/*
 * Generate VHDL for system tasks (like $display). Not all of
 * these are supported.
 */
static int draw_stask(vhdl_process *proc, ivl_statement_t stmt)
{
   const char *name = ivl_stmt_name(stmt);
 
   std::cout << "IVL_ST_STASK " << name << std::endl;

   if (strcmp(name, "$display") == 0)
      return draw_stask_display(proc, stmt);
   else {
      error("No VHDL translation for system task %s", name);
      return 0;
   }
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
   default:
      error("No VHDL translation for statement at %s:%d (type = %d)",
            ivl_stmt_file(stmt), ivl_stmt_lineno(stmt),
            ivl_statement_type(stmt));
      return 1;            
   }
}
