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
#include <map>
#include <set>

/*
 * Maps a signal to the scope it is defined within. Also
 * provides a mechanism for renaming signals -- i.e. when
 * an output has the same name as register: valid in Verilog
 * but not in VHDL, so two separate signals need to be
 * defined. 
 */
struct signal_defn_t {
   std::string renamed;     // The name of the VHDL signal
   vhdl_scope *scope;       // The scope where it is defined
};

typedef std::map<ivl_signal_t, signal_defn_t> signal_defn_map_t;


static int g_errors = 0;  // Total number of errors encountered
static entity_list_t g_entities;  // All entities to emit
static signal_defn_map_t g_known_signals;
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
 * Find an entity given a scope name.
 */
vhdl_entity *find_entity(const std::string &sname)
{
   entity_list_t::const_iterator it;
   for (it = g_entities.begin(); it != g_entities.end(); ++it) {
      if ((*it)->get_derived_from() == sname)
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
   assert(find_entity(ent->get_derived_from()) == NULL);
   g_entities.push_back(ent);
}

bool seen_signal_before(ivl_signal_t sig)
{
   return g_known_signals.find(sig) != g_known_signals.end();
}

/*
 * Remeber the association of signal to entity.
 */
void remember_signal(ivl_signal_t sig, vhdl_scope *scope)
{
   assert(!seen_signal_before(sig));

   signal_defn_t defn = { ivl_signal_basename(sig), scope };
   g_known_signals[sig] = defn;
}

/*
 * Change the VHDL name of a Verilog signal.
 */
void rename_signal(ivl_signal_t sig, const std::string &renamed)
{
   assert(seen_signal_before(sig));

   g_known_signals[sig].renamed = renamed;
}

vhdl_scope *find_scope_for_signal(ivl_signal_t sig)
{
   assert(seen_signal_before(sig));

   return g_known_signals[sig].scope;
}

const std::string &get_renamed_signal(ivl_signal_t sig)
{
   assert(seen_signal_before(sig));

   return g_known_signals[sig].renamed;
}

ivl_signal_t find_signal_named(const std::string &name, const vhdl_scope *scope)
{
   signal_defn_map_t::const_iterator it;
   for (it = g_known_signals.begin(); it != g_known_signals.end(); ++it) {
      if (((*it).second.scope == scope
           || (*it).second.scope == scope->get_parent())
          && (*it).second.renamed == name)
         return (*it).first;
   }
   assert(false);
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

      // Make sure we only emit one example of each type of entity
      set<string> seen_entities;
      
      for (entity_list_t::iterator it = g_entities.begin();
           it != g_entities.end();
           ++it) {
         if (seen_entities.find((*it)->get_name()) == seen_entities.end()) {
            (*it)->emit(outfile);
            seen_entities.insert((*it)->get_name());
         }
      }
      
      outfile.close();
   }
   
   // Clean up
   for (entity_list_t::iterator it = g_entities.begin();
        it != g_entities.end();
        ++it)
      delete (*it);
   g_entities.clear();
   
   return g_errors;
}
