/*
 *  VHDL code generation for scopes.
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
#include <sstream>
#include <cassert>

static vhdl_expr *translate_logic(vhdl_scope *scope, ivl_net_logic_t log);
static std::string make_safe_name(ivl_signal_t sig);

static vhdl_entity *g_active_entity = NULL;

vhdl_entity *get_active_entity()
{
   return g_active_entity;
}

void set_active_entity(vhdl_entity *ent)
{
   g_active_entity = ent;
}

/*
 * This represents the portion of a nexus that is visible within
 * a VHDL scope. If that nexus portion does not contain a signal,
 * then `tmpname' gives the name of the temporary that will be
 * used when this nexus is used in `scope' (e.g. for LPMs that
 * appear in instantiations).
 */
struct scope_nexus_t {
   vhdl_scope *scope;
   ivl_signal_t sig;    // A real signal
   string tmpname;      // A new temporary signal
};
   
/*
 * This structure is stored in the private part of each nexus.
 * It stores a scope_nexus_t for each VHDL scope which is
 * connected to that nexus. It's stored as a list so we can use
 * contained_within to allow several nested scopes to reference
 * the same signal.
 */
struct nexus_private_t {
   list<scope_nexus_t> signals;
   vhdl_expr *const_driver;
};

/*
 * Remember that sig is the representative of this nexus in scope. 
 */
static void link_scope_to_nexus_signal(nexus_private_t *priv, vhdl_scope *scope,
                                       ivl_signal_t sig)
{  
   scope_nexus_t sigmap = { scope, sig, "" };
   priv->signals.push_back(sigmap);
}

/*
 * Make a temporary the representative of this nexus in scope.
 */
static void link_scope_to_nexus_tmp(nexus_private_t *priv, vhdl_scope *scope,
                                    const string &name)
{
   scope_nexus_t sigmap = { scope, NULL, name };
   priv->signals.push_back(sigmap);
}

/*
 * Returns the scope_nexus_t of this nexus visible within scope.
 */
static scope_nexus_t *visible_nexus(nexus_private_t *priv, vhdl_scope *scope)
{
   list<scope_nexus_t>::iterator it;
   for (it = priv->signals.begin(); it != priv->signals.end(); ++it) {
      if (scope->contained_within((*it).scope))
         return &*it;
   }
   return NULL;
}

/*
 * Finds the name of the nexus signal within this scope.
 */
static string visible_nexus_signal_name(nexus_private_t *priv, vhdl_scope *scope)
{
   scope_nexus_t *sn = visible_nexus(priv, scope);
   assert(sn);

   return sn->sig ? get_renamed_signal(sn->sig) : sn->tmpname;
}

/*
 * Generates VHDL code to fully represent a nexus.
 */
void draw_nexus(ivl_nexus_t nexus)
{
   nexus_private_t *priv = new nexus_private_t;
   priv->const_driver = NULL;
   
   int nptrs = ivl_nexus_ptrs(nexus);

   // First pass through connect all the signals up
   for (int i = 0; i < nptrs; i++) {
      ivl_nexus_ptr_t nexus_ptr = ivl_nexus_ptr(nexus, i);
      
      ivl_signal_t sig;
      if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
         cout << "signal " << ivl_signal_basename(sig) << endl;
         
         vhdl_scope *scope = find_scope_for_signal(sig);

         if (visible_nexus(priv, scope)) {
            cout << "...should be linked to "
                 << visible_nexus_signal_name(priv, scope) << endl;
            assert(false);
         }
         else {
            cout << "...represents this nexus in scope " << hex << scope << endl;
            link_scope_to_nexus_signal(priv, scope, sig);
         }
      }
   }

   // Second pass through make sure logic/LPMs have signal
   // inputs and outputs
   for (int i = 0; i < nptrs; i++) {
      ivl_nexus_ptr_t nexus_ptr = ivl_nexus_ptr(nexus, i);
      
      ivl_net_logic_t log;
      ivl_lpm_t lpm;
      ivl_net_const_t con;
      if ((log = ivl_nexus_ptr_log(nexus_ptr))) {
         ivl_scope_t log_scope = ivl_logic_scope(log);
         vhdl_scope *vhdl_scope =
            find_entity(ivl_scope_name(log_scope))->get_arch()->get_scope();

         cout << "logic " << ivl_logic_basename(log) << endl;

         if (visible_nexus(priv, vhdl_scope)) {
            cout << "...linked to signal "
                 << visible_nexus_signal_name(priv, vhdl_scope) << endl;
         }
         else {
            cout << "...has no signal!" << endl;
            assert(false);
         }
      }
      else if ((lpm = ivl_nexus_ptr_lpm(nexus_ptr))) {
         ivl_scope_t lpm_scope = ivl_lpm_scope(lpm);
         vhdl_scope *vhdl_scope =
            find_entity(ivl_scope_name(lpm_scope))->get_arch()->get_scope();

         cout << "LPM " << ivl_lpm_basename(lpm) << endl;
         
         if (visible_nexus(priv, vhdl_scope)) {
            cout << "...linked to signal "
                 << visible_nexus_signal_name(priv, vhdl_scope) << endl;
         }
         else {
            // Create a temporary signal to connect the nexus
            // TODO: we could avoid this for IVL_LPM_PART_PV
            vhdl_type *type = vhdl_type::type_for(ivl_lpm_width(lpm),
                                                  ivl_lpm_signed(lpm) != 0);
            ostringstream ss;
            ss << "LPM" << ivl_lpm_basename(lpm);
            vhdl_scope->add_decl(new vhdl_signal_decl(ss.str().c_str(), type));
            
            link_scope_to_nexus_tmp(priv, vhdl_scope, ss.str());
         }
      }
      else if ((con = ivl_nexus_ptr_con(nexus_ptr))) {
         cout << "CONSTANT" << endl;

         if (ivl_const_width(con) == 1)
            priv->const_driver = new vhdl_const_bit(ivl_const_bits(con)[0]);
         else
            priv->const_driver =
               new vhdl_const_bits(ivl_const_bits(con), ivl_const_width(con),
                                   ivl_const_signed(con) != 0);
      }
   }
   
   // Save the private data in the nexus
   ivl_nexus_set_private(nexus, priv);
}

/*
 * Ensure that a nexus has been initialised. I.e. all the necessary
 * statements, declarations, etc. have been generated.
 */
static void seen_nexus(ivl_nexus_t nexus)
{
   if (ivl_nexus_get_private(nexus) == NULL) {
      cout << "first time we've seen nexus "
           << ivl_nexus_name(nexus) << endl;
      
      draw_nexus(nexus);
   }
}
 
/*
 * Translate a nexus to a variable reference. Given a nexus and a
 * scope, this function returns a reference to a signal that is
 * connected to the nexus and within the given scope. This signal
 * might not exist in the original Verilog source (even as a
 * compiler-generated temporary). If this nexus hasn't been
 * encountered before, the necessary code to connect up the nexus
 * will be generated.
 */
vhdl_var_ref *nexus_to_var_ref(vhdl_scope *scope, ivl_nexus_t nexus)
{
   cout << "nexus_to_var_ref " << ivl_nexus_name(nexus) << endl;
   
   seen_nexus(nexus);
   
   nexus_private_t *priv =
      static_cast<nexus_private_t*>(ivl_nexus_get_private(nexus));
   string renamed(visible_nexus_signal_name(priv, scope));
   
   cout << "--> signal " << renamed << endl;
   
   vhdl_decl *decl = scope->get_decl(renamed);
   assert(decl);
   
   vhdl_type *type = new vhdl_type(*(decl->get_type()));
   return new vhdl_var_ref(renamed.c_str(), type);    
}
 
/*
 * Convert the inputs of a logic gate to a binary expression.
 */
static vhdl_expr *inputs_to_expr(vhdl_scope *scope, vhdl_binop_t op,
                                 ivl_net_logic_t log)
{
   // Not always std_logic but this is probably OK since
   // the program has already been type checked
   vhdl_binop_expr *gate =
      new vhdl_binop_expr(op, vhdl_type::std_logic());
   
   int npins = ivl_logic_pins(log);
   for (int i = 1; i < npins; i++) {
      ivl_nexus_t input = ivl_logic_pin(log, i);
      gate->add_expr(nexus_to_var_ref(scope, input));
   }

   return gate;
}

/*
 * Convert a gate intput to an unary expression.
 */
static vhdl_expr *input_to_expr(vhdl_scope *scope, vhdl_unaryop_t op,
                                ivl_net_logic_t log)
{
   ivl_nexus_t input = ivl_logic_pin(log, 1);
   assert(input);

   vhdl_expr *operand = nexus_to_var_ref(scope, input);
   return new vhdl_unaryop_expr(op, operand, vhdl_type::std_logic()); 
}

static void bufif_logic(vhdl_arch *arch, ivl_net_logic_t log, bool if0)
{
   ivl_nexus_t output = ivl_logic_pin(log, 0);
   vhdl_var_ref *lhs = nexus_to_var_ref(arch->get_scope(), output);
   assert(lhs);
   
   vhdl_expr *val = nexus_to_var_ref(arch->get_scope(), ivl_logic_pin(log, 1));
   assert(val);

   vhdl_expr *sel = nexus_to_var_ref(arch->get_scope(), ivl_logic_pin(log, 2));
   assert(val);

   vhdl_expr *on = new vhdl_const_bit(if0 ? '0' : '1');
   vhdl_expr *cmp = new vhdl_binop_expr(sel, VHDL_BINOP_EQ, on, NULL);

   ivl_signal_t sig = find_signal_named(lhs->get_name(), arch->get_scope());
   char zbit;
   switch (ivl_signal_type(sig)) {
   case IVL_SIT_TRI0:
      zbit = '0';
      break;
   case IVL_SIT_TRI1:
      zbit = '1';
      break;
   case IVL_SIT_TRI:
   default:
      zbit = 'Z';
   }
   
   vhdl_const_bit *z = new vhdl_const_bit(zbit);
   vhdl_cassign_stmt *cass = new vhdl_cassign_stmt(lhs, z);
   cass->add_condition(val, cmp);

   arch->add_stmt(cass);
}

static vhdl_expr *translate_logic(vhdl_scope *scope, ivl_net_logic_t log)
{
   switch (ivl_logic_type(log)) {
   case IVL_LO_NOT:
      return input_to_expr(scope, VHDL_UNARYOP_NOT, log);
   case IVL_LO_AND:
      return inputs_to_expr(scope, VHDL_BINOP_AND, log);
   case IVL_LO_OR:
      return inputs_to_expr(scope, VHDL_BINOP_OR, log);
   case IVL_LO_XOR:
      return inputs_to_expr(scope, VHDL_BINOP_XOR, log);
   case IVL_LO_BUF:
   case IVL_LO_BUFZ:
      return nexus_to_var_ref(scope, ivl_logic_pin(log, 1));
   case IVL_LO_PULLUP:
      return new vhdl_const_bit('1');
   case IVL_LO_PULLDOWN:
      return new vhdl_const_bit('0');
   default:
      error("Don't know how to translate logic type = %d to expression",
            ivl_logic_type(log));
      return NULL;
   }
}

/*
 * Translate all the primitive logic gates into concurrent
 * signal assignments.
 */
static void declare_logic(vhdl_arch *arch, ivl_scope_t scope)
{
   int nlogs = ivl_scope_logs(scope);
   for (int i = 0; i < nlogs; i++) {
      ivl_net_logic_t log = ivl_scope_log(scope, i);
      
      switch (ivl_logic_type(log)) {
      case IVL_LO_BUFIF0:
         bufif_logic(arch, log, true);
         break;
      case IVL_LO_BUFIF1:
         bufif_logic(arch, log, false);
         break;
      default:
         {          
            // The output is always pin zero
            ivl_nexus_t output = ivl_logic_pin(log, 0);
            vhdl_var_ref *lhs = nexus_to_var_ref(arch->get_scope(), output);

            vhdl_expr *rhs = translate_logic(arch->get_scope(), log);
            vhdl_cassign_stmt *ass = new vhdl_cassign_stmt(lhs, rhs);
            
            ivl_expr_t delay = ivl_logic_delay(log, 1);
            if (delay)
               ass->set_after(translate_time_expr(delay));
            
            arch->add_stmt(ass);
         }
      }
   }
}

/*
 * Make sure a signal name conforms to VHDL naming rules.
 */
static string make_safe_name(ivl_signal_t sig)
{
   string name(ivl_signal_basename(sig));      
   if (name[0] == '_')
      name.insert(0, "VL");
   
   const char *vhdl_reserved[] = {
      "in", "out", "entity", "architecture", "inout", "array",
      "is", "not", "and", "or", "bus", "bit", "line", // Etc...
      NULL
   };
   for (const char **p = vhdl_reserved; *p != NULL; p++) {
      if (name == *p) {
         name.insert(0, "VL_");
         break;
      }
   }
   return name;
}

/*
 * Declare all signals and ports for a scope.
 */
static void declare_signals(vhdl_entity *ent, ivl_scope_t scope)
{
   int nsigs = ivl_scope_sigs(scope);
   for (int i = 0; i < nsigs; i++) {
      ivl_signal_t sig = ivl_scope_sig(scope, i);      
      remember_signal(sig, ent->get_arch()->get_scope());

      string name(make_safe_name(sig));
      rename_signal(sig, name);
      
      vhdl_type *sig_type;
      unsigned dimensions = ivl_signal_dimensions(sig);
      if (dimensions > 0) {
         // Arrays are implemented by generating a separate type
         // declaration for each array, and then declaring a
         // signal of that type

         if (dimensions > 1) {
            error("> 1 dimension arrays not implemented yet");
            return;
         }

         string type_name = name + "_Type";
         vhdl_type *base_type =
            vhdl_type::type_for(ivl_signal_width(sig), ivl_signal_signed(sig) != 0);

         int lsb = ivl_signal_array_base(sig);
         int msb = lsb + ivl_signal_array_count(sig) - 1;
         
         vhdl_type *array_type =
            vhdl_type::array_of(base_type, type_name, msb, lsb);
         vhdl_decl *array_decl = new vhdl_type_decl(type_name.c_str(), array_type);
         ent->get_arch()->get_scope()->add_decl(array_decl);
           
         sig_type = new vhdl_type(*array_type);
      }
      else
         sig_type =
            vhdl_type::type_for(ivl_signal_width(sig), ivl_signal_signed(sig) != 0);

      ivl_signal_port_t mode = ivl_signal_port(sig);
      switch (mode) {
      case IVL_SIP_NONE:
         {
            vhdl_decl *decl = new vhdl_signal_decl(name.c_str(), sig_type);
            ent->get_arch()->get_scope()->add_decl(decl);
         }
         break;
      case IVL_SIP_INPUT:
         ent->get_scope()->add_decl
            (new vhdl_port_decl(name.c_str(), sig_type, VHDL_PORT_IN));
         break;
      case IVL_SIP_OUTPUT:
         {
            vhdl_port_decl *decl =
               new vhdl_port_decl(name.c_str(), sig_type, VHDL_PORT_OUT);            
            ent->get_scope()->add_decl(decl);
         }

         if (ivl_signal_type(sig) == IVL_SIT_REG) {
            // A registered output
            // In Verilog the output and reg can have the
            // same name: this is not valid in VHDL
            // Instead a new signal foo_Reg is created
            // which represents the register
            std::string newname(name);
            newname += "_Reg";
            rename_signal(sig, newname.c_str());

            vhdl_type *reg_type = new vhdl_type(*sig_type);
            ent->get_arch()->get_scope()->add_decl
               (new vhdl_signal_decl(newname.c_str(), reg_type));
            
            // Create a concurrent assignment statement to
            // connect the register to the output
            ent->get_arch()->add_stmt
               (new vhdl_cassign_stmt
                (new vhdl_var_ref(name.c_str(), NULL),
                 new vhdl_var_ref(newname.c_str(), NULL)));
         }
         break;
      case IVL_SIP_INOUT:
         ent->get_scope()->add_decl
            (new vhdl_port_decl(name.c_str(), sig_type, VHDL_PORT_INOUT));
         break;
      default:
         assert(false);
      }
   }
}

/*
 * Generate VHDL for LPM instances in a module.
 */
static void declare_lpm(vhdl_arch *arch, ivl_scope_t scope)
{
   int nlpms = ivl_scope_lpms(scope);
   for (int i = 0; i < nlpms; i++) {
      ivl_lpm_t lpm = ivl_scope_lpm(scope, i);
      if (draw_lpm(arch, lpm) != 0)
         error("Failed to translate LPM %s", ivl_lpm_name(lpm));
   }
}

/*
 * Map two signals together in an instantiation.
 * The signals are joined by a nexus.
 */
static void map_signal(ivl_signal_t to, vhdl_entity *parent,
                       vhdl_comp_inst *inst)
{   
   // TODO: Work for multiple words
   ivl_nexus_t nexus = ivl_signal_nex(to, 0);
   seen_nexus(nexus);

   vhdl_scope *arch_scope = parent->get_arch()->get_scope();

   nexus_private_t *priv =
      static_cast<nexus_private_t*>(ivl_nexus_get_private(nexus));
   assert(priv);
   if (!visible_nexus(priv, arch_scope)) {
      // This nexus isn't attached to anything in the parent
      return;
   }

   vhdl_var_ref *ref = nexus_to_var_ref(parent->get_arch()->get_scope(), nexus);

   string name = make_safe_name(to);
  
   // If we're mapping an output of this entity to an output of
   // the child entity, then VHDL will not let us read the value
   // of the signal (i.e. it must pass straight through).
   // However, Verilog allows the signal to be read in the parent.
   // To get around this we create an internal signal name_Sig
   // that takes the value of the output and can be read.
   vhdl_decl *decl =
      parent->get_arch()->get_scope()->get_decl(ref->get_name());
   vhdl_port_decl *pdecl;
   if ((pdecl = dynamic_cast<vhdl_port_decl*>(decl))
       && pdecl->get_mode() == VHDL_PORT_OUT) {
      
      // We need to create a readable signal to shadow this output
      string shadow_name(ref->get_name());
      shadow_name += "_Sig";
      
      vhdl_signal_decl *shadow =
         new vhdl_signal_decl(shadow_name.c_str(),
                              new vhdl_type(*decl->get_type()));
      shadow->set_comment("Needed to make output readable");
      
      parent->get_arch()->get_scope()->add_decl(shadow);
      
      // Make a continuous assignment of the shadow to the output
      parent->get_arch()->add_stmt
         (new vhdl_cassign_stmt
          (ref, new vhdl_var_ref(shadow_name.c_str(), NULL)));

      // Make sure any future references to this signal read the
      // shadow not the output
      ivl_signal_t sig = find_signal_named(ref->get_name(),
                                           parent->get_arch()->get_scope());
      rename_signal(sig, shadow_name);
      
      // Finally map the child port to the shadow signal
      inst->map_port(name.c_str(),
                     new vhdl_var_ref(shadow_name.c_str(), NULL));
   }
   else {
      // Not an output port declaration therefore we can
      // definitely read it
      inst->map_port(name.c_str(), ref);
   }
}

/*
 * Find all the port mappings of a module instantiation.
 */
static void port_map(ivl_scope_t scope, vhdl_entity *parent,
                     vhdl_comp_inst *inst)
{
   // Find all the port mappings
   int nsigs = ivl_scope_sigs(scope);
   for (int i = 0; i < nsigs; i++) {
      ivl_signal_t sig = ivl_scope_sig(scope, i);
      
      ivl_signal_port_t mode = ivl_signal_port(sig);
      switch (mode) {
      case IVL_SIP_NONE:
         // Internal signals don't appear in the port map
         break;
      case IVL_SIP_INPUT:
      case IVL_SIP_OUTPUT:
      case IVL_SIP_INOUT:
         map_signal(sig, parent, inst);
         break;         
      default:
         assert(false);
      }      
   }
}

/*
 * Create a VHDL function from a Verilog function definition.
 */
static int draw_function(ivl_scope_t scope, ivl_scope_t parent)
{
   assert(ivl_scope_type(scope) == IVL_SCT_FUNCTION);

   // Find the containing entity
   vhdl_entity *ent = find_entity(ivl_scope_name(parent));
   assert(ent);

   const char *funcname = ivl_scope_tname(scope);

   assert(!ent->get_arch()->get_scope()->have_declared(funcname));

   // The return type is worked out from the output port
   vhdl_function *func = new vhdl_function(funcname, NULL);
   
   int nsigs = ivl_scope_sigs(scope);
   for (int i = 0; i < nsigs; i++) {
      ivl_signal_t sig = ivl_scope_sig(scope, i);            
      vhdl_type *sigtype =
         vhdl_type::type_for(ivl_signal_width(sig),
                             ivl_signal_signed(sig) != 0);
      
      std::string signame = make_safe_name(sig);

      switch (ivl_signal_port(sig)) {
      case IVL_SIP_INPUT:
         func->add_param(new vhdl_param_decl(signame.c_str(), sigtype));
         break;
      case IVL_SIP_OUTPUT:
         // The magic variable Verilog_Result holds the return value
         signame = "Verilog_Result";
         func->set_type(new vhdl_type(*sigtype));
      default:
         func->get_scope()->add_decl
            (new vhdl_var_decl(signame.c_str(), sigtype));
      }
      
      remember_signal(sig, func->get_scope());
      rename_signal(sig, signame);
   }
   
   // Non-blocking assignment not allowed in functions
   func->get_scope()->set_allow_signal_assignment(false);

   set_active_entity(ent);
   {
      draw_stmt(func, func->get_container(), ivl_scope_def(scope));
   }
   set_active_entity(NULL);
   
   ent->get_arch()->get_scope()->add_decl(func);   
   return 0;
}

/*
 * Create an empty VHDL entity for a Verilog module.
 */
static void create_skeleton_entity_for(ivl_scope_t scope)
{
   assert(ivl_scope_type(scope) == IVL_SCT_MODULE);

   // The type name will become the entity name
   const char *tname = ivl_scope_tname(scope);

   // Remember the scope name this entity was derived from so
   // the correct processes can be added later
   const char *derived_from = ivl_scope_name(scope);
   
   // Verilog does not have the entity/architecture distinction
   // so we always create a pair and associate the architecture
   // with the entity for convenience (this also means that we
   // retain a 1-to-1 mapping of scope to VHDL element)
   vhdl_arch *arch = new vhdl_arch(tname, "FromVerilog");
   vhdl_entity *ent = new vhdl_entity(tname, derived_from, arch);

   // Build a comment to add to the entity/architecture
   ostringstream ss;
   ss << "Generated from Verilog module " << ivl_scope_tname(scope);
   
   arch->set_comment(ss.str());
   ent->set_comment(ss.str());
   
   remember_entity(ent);
}

/*
 * A first pass through the hierarchy: create VHDL entities for
 * each unique Verilog module type.
 */
static int draw_skeleton_scope(ivl_scope_t scope, void *_parent)
{
   if (ivl_scope_type(scope) == IVL_SCT_MODULE) {
      create_skeleton_entity_for(scope);
      cout << "Created skeleton entity for " << ivl_scope_tname(scope)
           << " from instance " << ivl_scope_name(scope) << endl;
   }
   
   return ivl_scope_children(scope, draw_skeleton_scope, scope);
}

static int draw_all_signals(ivl_scope_t scope, void *_parent)
{
   if (ivl_scope_type(scope) == IVL_SCT_MODULE) {
      vhdl_entity *ent = find_entity(ivl_scope_name(scope));
      assert(ent);

      declare_signals(ent, scope);
   }

   return ivl_scope_children(scope, draw_all_signals, scope);
}

/*
 * Draw all tasks and functions in the hierarchy.
 */
static int draw_functions(ivl_scope_t scope, void *_parent)
{
   ivl_scope_t parent = static_cast<ivl_scope_t>(_parent);
   if (ivl_scope_type(scope) == IVL_SCT_FUNCTION) {
      vhdl_entity *ent = find_entity(ivl_scope_name(parent));
      assert(ent);
      
      if (draw_function(scope, parent) != 0)
         return 1;
   }

   return ivl_scope_children(scope, draw_functions, scope);
}

/*
 * Make concurrent assignments for constants in nets. This works
 * bottom-up so that the driver is in the lowest instance it can.
 * This also has the side effect of generating all the necessary
 * nexus code.
 */
static int draw_constant_drivers(ivl_scope_t scope, void *_parent)
{
   ivl_scope_children(scope, draw_constant_drivers, scope);
   
   if (ivl_scope_type(scope) == IVL_SCT_MODULE) {
      vhdl_entity *ent = find_entity(ivl_scope_name(scope));
      assert(ent);

      int nsigs = ivl_scope_sigs(scope);
      for (int i = 0; i < nsigs; i++) {
         ivl_signal_t sig = ivl_scope_sig(scope, i);
         
         for (unsigned i = ivl_signal_array_base(sig);
              i < ivl_signal_array_count(sig);
              i++) {
            // Make sure the nexus code is generated
            ivl_nexus_t nex = ivl_signal_nex(sig, i);
            seen_nexus(nex);
            
            assert(i == 0);   // TODO: Make work for more words
            nexus_private_t *priv =
               static_cast<nexus_private_t*>(ivl_nexus_get_private(nex));
            assert(priv);

            if (priv->const_driver) {
               cout << "NEEDS CONST DRIVER!" << endl;
               cout << "(in scope " << ivl_scope_name(scope) << endl;
               
               vhdl_var_ref *ref =
                  nexus_to_var_ref(ent->get_arch()->get_scope(), nex);
               
               ent->get_arch()->add_stmt
                  (new vhdl_cassign_stmt(ref, priv->const_driver));                  
               priv->const_driver = NULL;
            }
         }           
      }
   }

   return 0;
}

static int draw_all_logic_and_lpm(ivl_scope_t scope, void *_parent)
{
   if (ivl_scope_type(scope) == IVL_SCT_MODULE) {
      vhdl_entity *ent = find_entity(ivl_scope_name(scope));
      assert(ent);

      set_active_entity(ent);
      {
         declare_logic(ent->get_arch(), scope);
         declare_lpm(ent->get_arch(), scope);
      }
      set_active_entity(NULL);
   }   

   return ivl_scope_children(scope, draw_all_logic_and_lpm, scope);
}

static int draw_hierarchy(ivl_scope_t scope, void *_parent)
{
   if (ivl_scope_type(scope) == IVL_SCT_MODULE && _parent) {
      ivl_scope_t parent = static_cast<ivl_scope_t>(_parent);
   
      vhdl_entity *ent = find_entity(ivl_scope_name(scope));
      assert(ent);
      
      vhdl_entity *parent_ent = find_entity(ivl_scope_name(parent));
      assert(parent_ent);

      vhdl_arch *parent_arch = parent_ent->get_arch();
      assert(parent_arch != NULL);
         
      // Create a forward declaration for it
      if (!parent_arch->get_scope()->have_declared(ent->get_name())) {
         vhdl_decl *comp_decl = vhdl_component_decl::component_decl_for(ent);
         parent_arch->get_scope()->add_decl(comp_decl);
      }
         
      // And an instantiation statement
      string inst_name(ivl_scope_basename(scope));
      if (inst_name == ent->get_name()) {
         // Cannot have instance name the same as type in VHDL
         inst_name += "_Inst";
      }

      // Need to replace any [ and ] characters that result
      // from generate statements
      string::size_type loc = inst_name.find('[', 0);
      if (loc != string::npos)
         inst_name.erase(loc, 1);
      
      loc = inst_name.find(']', 0);
      if (loc != string::npos)
         inst_name.erase(loc, 1);         
      
      vhdl_comp_inst *inst =
         new vhdl_comp_inst(inst_name.c_str(), ent->get_name().c_str());
      port_map(scope, parent_ent, inst);         
      
      parent_arch->add_stmt(inst);
   }   

   return ivl_scope_children(scope, draw_hierarchy, scope);
}

int draw_scope(ivl_scope_t scope, void *_parent)
{
   int rc = draw_skeleton_scope(scope, _parent);
   if (rc != 0)
      return rc;

   rc = draw_all_signals(scope, _parent);
   if (rc != 0)
      return rc;

   rc = draw_all_logic_and_lpm(scope, _parent);
   if (rc != 0)
      return rc;      

   rc = draw_hierarchy(scope, _parent);
   if (rc != 0)
      return rc;

   rc = draw_functions(scope, _parent);
   if (rc != 0)
      return rc;

   rc = draw_constant_drivers(scope, _parent);
   if (rc != 0)
      return rc;
   
   return 0;
}

