/*
 *  VHDL code generator for Icarus Verilog.
 *
 *  Copyright (C) 2008-2024  Nick Gasson (nick@nickg.me.uk)
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

#include "version_base.h"
#include "version_tag.h"
#include "vhdl_target.h"
#include "state.hh"

#include <iostream>
#include <fstream>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>

using namespace std;

static const char*version_string =
"Icarus Verilog VHDL Code Generator " VERSION " (" VERSION_TAG ")\n\n"
"Copyright (C) 2008-2024  Nick Gasson (nick@nickg.me.uk)\n\n"
"  This program is free software; you can redistribute it and/or modify\n"
"  it under the terms of the GNU General Public License as published by\n"
"  the Free Software Foundation; either version 2 of the License, or\n"
"  (at your option) any later version.\n"
"\n"
"  This program is distributed in the hope that it will be useful,\n"
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"  GNU General Public License for more details.\n"
"\n"
"  You should have received a copy of the GNU General Public License along\n"
"  with this program; if not, write to the Free Software Foundation, Inc.,\n"
"  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n"
;


static int g_errors = 0;  // Total number of errors encountered
static ivl_design_t g_design;


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

/*
 * Print a message only if -pdebug was specified.
 */
void debug_msg(const char *fmt, ...)
{
   std::va_list args;

   va_start(args, fmt);

   if (std::strcmp(ivl_design_flag(g_design, "debug"), "")) {
      std::fputs("[DEBUG] ", stdout);
      std::vprintf(fmt, args);
      std::putchar('\n');
   }

   va_end(args);
}

ivl_design_t get_vhdl_design()
{
   return g_design;
}

extern "C" int target_design(ivl_design_t des)
{
   ivl_scope_t *roots;
   unsigned int nroots;
   ivl_design_roots(des, &roots, &nroots);

   g_design = des;

   for (unsigned int i = 0; i < nroots; i++)
      draw_scope(roots[i], NULL);

   // Only generate processes if there were no errors generating entities
   // (otherwise the necessary information won't be present)
   if (0 == g_errors)
      ivl_design_process(des, draw_process, NULL);

   // Write the generated elements to the output file
   // only if there were no errors generating entities or processes
   if (0 == g_errors) {
      const char *ofname = ivl_design_flag(des, "-o");
      ofstream outfile(ofname);
      outfile << "-- This VHDL was converted from Verilog using the" << endl
              << "-- Icarus Verilog VHDL Code Generator " VERSION
                 " (" VERSION_TAG ")" << endl << endl;

      // If the user passed -pdepth=N then only emit entities with
      // depth < N
      // I.e. -pdepth=1 emits only the top-level entity
      // If max_depth is zero then all entities will be emitted
      // (This is handy since it means we can use atoi ;-)
      int max_depth = std::atoi(ivl_design_flag(des, "depth"));

      emit_all_entities(outfile, max_depth);
   }

   // Clean up
   free_all_vhdl_objects();

   return g_errors;
}

extern "C" const char* target_query(const char*key)
{
   if (strcmp(key, "version") == 0) return version_string;
   return 0;
}
