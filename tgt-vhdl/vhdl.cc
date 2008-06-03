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
#include <cassert>
#include <cstring>
#include <list>

static int g_errors = 0;  // Total number of errors encountered

static entity_list_t g_entities;  // All entities to emit

typedef std::string package_name_t;
typedef std::list<package_name_t> require_list_t;

static require_list_t g_requires;  // External packages required


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
 * Find an entity given a type name.
 */
vhdl_entity *find_entity(const std::string &tname)
{
   entity_list_t::const_iterator it;
   for (it = g_entities.begin(); it != g_entities.end(); ++it) {
      if ((*it)->get_name() == tname)
         return *it;
   }
   return NULL;
}

/*
 * Add an entity/architecture pair to the list of entities
 * to emit.
 */
void remember_entity(vhdl_entity* ent)
{
   assert(find_entity(ent->get_name()) == NULL);
   g_entities.push_back(ent);
}

/*
 * Add a package to the list of packages that should be
 * use-ed at the start of the program.
 */
void require_package(const char *name)
{
   package_name_t pname(name);
   require_list_t::iterator it;
   for (it = g_requires.begin(); it != g_requires.end(); ++it) {
      if (*it == pname)
         return;
   }
   g_requires.push_back(pname);
}

extern "C" int target_design(ivl_design_t des)
{
   ivl_scope_t *roots;
   unsigned int nroots;
   ivl_design_roots(des, &roots, &nroots);

   for (unsigned int i = 0; i < nroots; i++)
      draw_scope(roots[i], NULL);

   ivl_design_process(des, draw_process, NULL);

   // Write the generated elements to the output file
   const char *ofname = ivl_design_flag(des, "-o");
   std::ofstream outfile(ofname);

   // Write all the required packages (and libraries?)
   //outfile << "library ieee;" << std::endl;
   for (require_list_t::iterator it = g_requires.begin();
        it != g_requires.end();
        ++it)
      outfile << "use " << *it << ".all;" << std::endl;
   outfile << std::endl;
   
   for (entity_list_t::iterator it = g_entities.begin();
        it != g_entities.end();
        ++it)
      (*it)->emit(outfile);
   
   outfile.close();

   // Clean up
   for (entity_list_t::iterator it = g_entities.begin();
        it != g_entities.end();
        ++it)
      delete (*it);
   g_entities.clear();
   
   return g_errors;
}
