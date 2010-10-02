/*
 *  VHDL implementation of $display.
 *
 *  Copyright (C) 2008-2009  Nick Gasson (nick@nickg.me.uk)
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
#include <cctype>
#include <sstream>

static const char *DISPLAY_LINE = "Verilog_Display_Line";

/*
 * Write a VHDL expression into the current display line.
 */
static void display_write(stmt_container *container, vhdl_expr *expr)
{
   vhdl_pcall_stmt *write = new vhdl_pcall_stmt("Write");
   vhdl_var_ref *ref =
      new vhdl_var_ref(DISPLAY_LINE, vhdl_type::line());
   write->add_expr(ref);

   vhdl_type_name_t type = expr->get_type()->get_name();
   if (type == VHDL_TYPE_SIGNED || type == VHDL_TYPE_UNSIGNED) {
      vhdl_type integer(VHDL_TYPE_INTEGER);
      write->add_expr(expr->cast(&integer));
   }
   else if (type != VHDL_TYPE_STRING) {
      // Need to add a call to Type'Image for types not
      // supported by std.textio
      std::string name(expr->get_type()->get_string());
      name += "'Image";

      vhdl_fcall *cast
         = new vhdl_fcall(name.c_str(), vhdl_type::string());
      cast->add_expr(expr);

      write->add_expr(cast);
   }
   else
      write->add_expr(expr);

   container->add_stmt(write);
}

/*
 * Write the value of DISPLAY_LINE to the output.
 */
static void display_line(stmt_container *container)
{
   vhdl_pcall_stmt *write_line = new vhdl_pcall_stmt("WriteLine");
   vhdl_var_ref *output_ref =
      new vhdl_var_ref("std.textio.Output", new vhdl_type(VHDL_TYPE_FILE));
   write_line->add_expr(output_ref);
   vhdl_var_ref *ref =
      new vhdl_var_ref(DISPLAY_LINE, vhdl_type::line());
   write_line->add_expr(ref);
   container->add_stmt(write_line);
}

/*
 * Parse an octal escape sequence.
 */
static char parse_octal(const char *p)
{
   assert(*p && *(p+1) && *(p+2));
   assert(isdigit(*p) && isdigit(*(p+1)) && isdigit(*(p+1)));

   return (*p - '0') * 64
      + (*(p+1) - '0') * 8
      + (*(p+2) - '0') * 1;
}

static void flush_string(std::ostringstream &ss, stmt_container *container)
{
   display_write(container, new vhdl_const_string(ss.str().c_str()));

   // Clear the stream
   ss.str("");
}

// This should display the hierarchical module name, but we don't support
// this in VHDL. So just emit a warning.
static void display_m(stmt_container* container)
{
   cerr << "Warning: no VHDL translation for %m format code" << endl;
}

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
int draw_stask_display(vhdl_procedural *proc, stmt_container *container,
                       ivl_statement_t stmt, bool newline)
{
   if (!proc->get_scope()->have_declared(DISPLAY_LINE)) {
      vhdl_var_decl *line_var =
         new vhdl_var_decl(DISPLAY_LINE, vhdl_type::line());
      line_var->set_comment("For generating $display output");
      proc->get_scope()->add_decl(line_var);
   }

   // Write the data into the line
   int count = ivl_stmt_parm_count(stmt), i = 0;
   while (i < count) {
      // $display may have an empty parameter, in which case
      // the expression will be null
      // The behaviour here seems to be to output a space
      ivl_expr_t net = ivl_stmt_parm(stmt, i++);
      if (net == NULL) {
         display_write(container, new vhdl_const_string(" "));
         continue;
      }

      if (ivl_expr_type(net) == IVL_EX_STRING) {
         ostringstream ss;
         for (const char *p = ivl_expr_string(net); *p; p++) {
            if (*p == '\\') {
               // Octal escape
               char ch = parse_octal(p+1);
               if (ch == '\n') {
                  flush_string(ss, container);
                  display_line(container);
               }
               else
                  ss << ch;
               p += 3;
            }
            else if (*p == '%' && *(++p) != '%') {
               flush_string(ss, container);

               // Skip over width for now
               while (isdigit(*p)) ++p;

               switch (*p) {
               case 'm':
                  display_m(container);
                  break;
               default:
                  {
                     assert(i < count);
                     ivl_expr_t netp = ivl_stmt_parm(stmt, i++);
                     assert(netp);

                     vhdl_expr *base = translate_expr(netp);
                     if (NULL == base)
                        return 1;

                     display_write(container, base);
                  }
               }
            }
            else
               ss << *p;
         }

         // Call Write on any non-empty string data left in the buffer
         if (!ss.str().empty())
            display_write(container, new vhdl_const_string(ss.str().c_str()));
      }
      else {
         vhdl_expr *base = translate_expr(net);
         if (NULL == base)
            return 1;

         display_write(container, base);
      }
   }

   if (newline)
      display_line(container);

   return 0;
}
