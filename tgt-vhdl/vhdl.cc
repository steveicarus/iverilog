/*
 *  VHDL code generator for Icarus Verilog.
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
#include <fstream>
#include <cstdarg>
#include <cstdio>

static int g_errors = 0;  // Total number of errors encountered

/*
 * Called when an unrecoverable problem is encountered.
 */
void error(const char *fmt, ...)
{
   std::va_list args;

   va_start(args, fmt);
   std::printf("VHDL conversion error: ");  // Source/line number?
   std::vprintf(fmt, args);
   std::putchar('\n');
   va_end(args);

   g_errors++;
}

extern "C" int target_design(ivl_design_t des)
{
   ivl_scope_t *roots;
   unsigned int nroots;
   ivl_design_roots(des, &roots, &nroots);

   const char *ofname = ivl_design_flag(des, "-o");
   std::ofstream outfile(ofname);

   for (unsigned int i = 0; i < nroots; i++) {
      ivl_scope_t scope = roots[i];
      const char *scope_name = ivl_scope_basename(scope);
      
      // Dummy output to test regression script
      vhdl_entity test_ent(scope_name);
      vhdl_arch test_arch(scope_name);
      vhdl_process test_proc;
      test_arch.set_comment("I am a comment");
      test_arch.add_stmt(&test_proc);
      test_proc.set_comment("I am a process");
      test_ent.emit(outfile);
      test_arch.emit(outfile);
   }
   
   outfile.close();
      
   return g_errors;
}
