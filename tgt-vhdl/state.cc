/*
 *  Managing global state for the VHDL code generator.
 *
 *  Copyright (C) 2009  Nick Gasson (nick@nickg.me.uk)
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

#include "state.hh"
#include "vhdl_syntax.hh"
#include "vhdl_target.h"

#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

/*
 * This file stores all the global state required during VHDL code
 * generation. At present we store the following:
 *
 *  - A mapping from Verilog signals to the VHDL scope (entity, etc.)
 *    where it is found, and the name of the corresponding VHDL signal.
 *    This allows us to support renaming invalid Verilog signal names
 *    to valid VHDL ones.
 *
 *  - The set of all VHDL entities generated.
 *
 *  - The currently active entity. "Active" here means that we are
 *    currently generating code for a process inside the corresponding
 *    scope. This is useful, for example, if a statement or expression
 *    in a process needs to add are referencing something in the containing
 *    architecture object.
 */

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

// All entities to emit.
// These are stored in a list rather than a set so the first
// entity added will correspond to the first (top) Verilog module
// encountered and hence it will appear first in the output file.
static entity_list_t g_entities;

// Store the mapping of ivl scope names to entity names
typedef map<ivl_scope_t, string> scope_name_map_t;
static scope_name_map_t g_scope_names;

typedef std::map<ivl_signal_t, signal_defn_t> signal_defn_map_t;
static signal_defn_map_t g_known_signals;

static vhdl_entity *g_active_entity = NULL;

// Set of scopes that are treated as the default examples of
// that type. Any other scopes of the same type are ignored.
typedef std::vector<ivl_scope_t> default_scopes_t;
static default_scopes_t g_default_scopes;

// True if signal `sig' has already been encountered by the code
// generator. This means we have already assigned it to a VHDL code
// object and possibly renamed it.
bool seen_signal_before(ivl_signal_t sig)
{
   return g_known_signals.find(sig) != g_known_signals.end();
}

// Remember the association of signal to a VHDL code object (typically
// an entity).
void remember_signal(ivl_signal_t sig, vhdl_scope *scope)
{
   assert(!seen_signal_before(sig));

   signal_defn_t defn = { ivl_signal_basename(sig), scope };
   g_known_signals[sig] = defn;
}

// Change the VHDL name of a Verilog signal.
void rename_signal(ivl_signal_t sig, const std::string &renamed)
{
   assert(seen_signal_before(sig));

   g_known_signals[sig].renamed = renamed;
}

// Given a Verilog signal, return the VHDL code object where it should
// be defined. Note that this can return a NULL pointer if `sig' hasn't
// be encountered yet.
vhdl_scope *find_scope_for_signal(ivl_signal_t sig)
{
   if (seen_signal_before(sig))
      return g_known_signals[sig].scope;
   else
      return NULL;
}

// Get the name of the VHDL signal corresponding to Verilog signal `sig'.
const std::string &get_renamed_signal(ivl_signal_t sig)
{
   assert(seen_signal_before(sig));

   return g_known_signals[sig].renamed;
}

// TODO: Can we dispose of this???
// -> This is only used in logic.cc to get the type of a signal connected
//    to a logic device -> we should be able to get this from the nexus
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
   return NULL;
}

// Compare the name of an entity against a string
struct cmp_ent_name {
   explicit cmp_ent_name(const string& n) : name_(n) {}

   bool operator()(const vhdl_entity* ent) const
   {
      return ent->get_name() == name_;
   }

   const string& name_;
};

// Find an entity given its name.
vhdl_entity* find_entity(const string& name)
{
   entity_list_t::const_iterator it
      = find_if(g_entities.begin(), g_entities.end(),
                cmp_ent_name(name));

   if (it != g_entities.end())
      return *it;
   else
      return NULL;
}

// Find a VHDL entity given a Verilog module scope. The VHDL entity
// name should be the same as the Verilog module type name.
// Note that this will return NULL if no entity has been recorded
// for this scope type.
vhdl_entity* find_entity(ivl_scope_t scope)
{
   // Skip over generate scopes
   while (ivl_scope_type(scope) == IVL_SCT_GENERATE)
      scope = ivl_scope_parent(scope);

   assert(ivl_scope_type(scope) == IVL_SCT_MODULE);

   if (is_default_scope_instance(scope)) {
      scope_name_map_t::iterator it = g_scope_names.find(scope);
      if (it != g_scope_names.end())
         return find_entity((*it).second);
      else
         return NULL;
   }
   else {
      const char *tname = ivl_scope_tname(scope);

      for (scope_name_map_t::iterator it = g_scope_names.begin();
           it != g_scope_names.end(); ++it) {
         if (strcmp(tname, ivl_scope_tname((*it).first)) == 0)
            return find_entity((*it).second);
      }

      return NULL;
   }
}

// Add an entity/architecture pair to the list of entities to emit.
void remember_entity(vhdl_entity* ent, ivl_scope_t scope)
{
   g_entities.push_back(ent);
   g_scope_names[scope] = ent->get_name();
}

// Print all VHDL entities, in order, to the specified output stream.
void emit_all_entities(std::ostream& os, int max_depth)
{
   for (entity_list_t::iterator it = g_entities.begin();
        it != g_entities.end();
        ++it) {
      if ((max_depth == 0 || (*it)->depth < max_depth))
         (*it)->emit(os);
   }
}

// Release all memory for the VHDL objects. No vhdl_element pointers
// will be valid after this call.
void free_all_vhdl_objects()
{
   int freed = vhdl_element::free_all_objects();
   debug_msg("Deallocated %d VHDL syntax objects", freed);

   size_t total = vhdl_element::total_allocated();
   debug_msg("%d total bytes used for VHDL syntax objects", total);

   g_entities.clear();
}

// Return the currently active entity
vhdl_entity *get_active_entity()
{
   return g_active_entity;
}

// Change the currently active entity
void set_active_entity(vhdl_entity *ent)
{
   g_active_entity = ent;
}

/*
 * True if two scopes have the same type name.
 */
static bool same_scope_type_name(ivl_scope_t a, ivl_scope_t b)
{
   if (strcmp(ivl_scope_tname(a), ivl_scope_tname(b)) != 0)
      return false;

   unsigned nparams_a = ivl_scope_params(a);
   unsigned nparams_b = ivl_scope_params(b);

   if (nparams_a != nparams_b)
      return false;

   for (unsigned i = 0; i < nparams_a; i++) {
      ivl_parameter_t param_a = ivl_scope_param(a, i);
      ivl_parameter_t param_b = ivl_scope_param(b, i);

      if (strcmp(ivl_parameter_basename(param_a),
                 ivl_parameter_basename(param_b)) != 0)
         return false;

      if (ivl_parameter_local(param_a) && ivl_parameter_local(param_b))
	    continue;

      // If this is a type parameter consider the scopes not equal since we do
      // not have support for comparing the actual types yet.
      if (ivl_parameter_is_type(param_a) || ivl_parameter_is_type(param_b))
	    return false;

      ivl_expr_t value_a = ivl_parameter_expr(param_a);
      ivl_expr_t value_b = ivl_parameter_expr(param_b);

      if (ivl_expr_type(value_a) != ivl_expr_type(value_b))
         return false;

      switch (ivl_expr_type(value_a)) {
         case IVL_EX_STRING:
            if (strcmp(ivl_expr_string(value_a), ivl_expr_string(value_b)) != 0)
               return false;
            break;

         case IVL_EX_NUMBER:
            if (ivl_expr_uvalue(value_a) != ivl_expr_uvalue(value_b))
               return false;
            break;

      default:
         assert(false);
      }
   }

   return true;
}

/*
 * True if we have already seen a scope with this type before.
 * If the result is `false' then s is stored in the set of seen
 * scopes.
 */
bool seen_this_scope_type(ivl_scope_t s)
{
   for (auto cur = g_default_scopes.begin() ; cur != g_default_scopes.end() ; cur++) {
     if (same_scope_type_name(s, *cur))
       return true;
   }

   g_default_scopes.push_back(s);
   return false;
}

/*
 * True if this scope is the default example of this scope type.
 * All other instances of this scope type are ignored.
 */
bool is_default_scope_instance(ivl_scope_t s)
{
   return find(g_default_scopes.begin(), g_default_scopes.end(), s)
      != g_default_scopes.end();
}
