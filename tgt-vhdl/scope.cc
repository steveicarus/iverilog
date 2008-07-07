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

/*
 * Given a nexus find a constant value in it that can be used
 * as an initial signal value.
 */
static vhdl_expr *nexus_to_const(ivl_nexus_t nexus)
{
   int nptrs = ivl_nexus_ptrs(nexus);
   for (int i = 0; i < nptrs; i++) {
      ivl_nexus_ptr_t nexus_ptr = ivl_nexus_ptr(nexus, i);

      ivl_net_const_t con;
      if ((con = ivl_nexus_ptr_con(nexus_ptr))) {
         if (ivl_const_width(con) == 1)
            return new vhdl_const_bit(ivl_const_bits(con)[0]);
         else
            return new vhdl_const_bits
               (ivl_const_bits(con), ivl_const_width(con),
                ivl_const_signed(con) != 0);
      }
      else {
         // Ignore other types of nexus pointer
      }
   }
   
   return NULL;
}

/*
 * Given a nexus and an architecture scope, find the first signal
 * that is connected to the nexus, if there is one. Never return
 * a reference to 'ignore' if it is found in the nexus.
 */
static vhdl_expr *nexus_to_expr(vhdl_scope *arch_scope, ivl_nexus_t nexus,
                                ivl_signal_t ignore = NULL)
{
   std::cout << "nexus_to_expr " << ivl_nexus_name(nexus) << std::endl;
   
   int nptrs = ivl_nexus_ptrs(nexus);
   for (int i = 0; i < nptrs; i++) {
      ivl_nexus_ptr_t nexus_ptr = ivl_nexus_ptr(nexus, i);

      ivl_signal_t sig;
      ivl_net_logic_t log;
      ivl_lpm_t lpm;
      if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
         if (!seen_signal_before(sig) || sig == ignore)
            continue;
         
         const char *signame = get_renamed_signal(sig).c_str();
         
         vhdl_decl *decl = arch_scope->get_decl(signame);
         if (NULL == decl)
            continue;  // Not in this scope

         vhdl_type *type = new vhdl_type(*(decl->get_type()));
         return new vhdl_var_ref(signame, type);
      }
      else if ((log = ivl_nexus_ptr_log(nexus_ptr))) {
         return translate_logic(arch_scope, log);
      }
      else if ((lpm = ivl_nexus_ptr_lpm(nexus_ptr))) {
         std::cout << "LPM to expr" << std::endl;
         vhdl_expr *e = lpm_to_expr(arch_scope, lpm);
         if (e)
            return e;
      }
      else {
         // Ignore other types of nexus pointer
      }
   }
   
   assert(false);
}

vhdl_var_ref *nexus_to_var_ref(vhdl_scope *arch_scope, ivl_nexus_t nexus)
{
   int nptrs = ivl_nexus_ptrs(nexus);
   for (int i = 0; i < nptrs; i++) {
      ivl_nexus_ptr_t nexus_ptr = ivl_nexus_ptr(nexus, i);

      ivl_signal_t sig;
      if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
         if (!seen_signal_before(sig))
            continue;
         
         const char *signame = get_renamed_signal(sig).c_str();
         
         vhdl_decl *decl = arch_scope->get_decl(signame);
         if (NULL == decl)
            continue;  // Not in this scope

         vhdl_type *type = new vhdl_type(*(decl->get_type()));
         return new vhdl_var_ref(signame, type);
      }
      else {
         // Ignore other types of nexus pointer
      }
   }
   
   return NULL;
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
      gate->add_expr(nexus_to_expr(scope, input));
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

   vhdl_expr *operand = nexus_to_expr(scope, input);
   return new vhdl_unaryop_expr(op, operand, vhdl_type::std_logic()); 
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
      return nexus_to_expr(scope, ivl_logic_pin(log, 1));
   default:
      error("Don't know how to translate logic type = %d",
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
      
      // The output is always pin zero
      ivl_nexus_t output = ivl_logic_pin(log, 0);
      vhdl_var_ref *lhs =
         dynamic_cast<vhdl_var_ref*>(nexus_to_expr(arch->get_scope(), output));
      if (NULL == lhs)
         continue;  // Not suitable for continuous assignment
      
      vhdl_expr *rhs = translate_logic(arch->get_scope(), log);
      
      arch->add_stmt(new vhdl_cassign_stmt(lhs, rhs));
   }
}

/*
 * Make sure a signal name conforms to VHDL naming rules.
 */
static std::string make_safe_name(ivl_signal_t sig)
{
   std::string name(ivl_signal_basename(sig));      
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
 * Create a VHDL type for a Verilog signal.
 */
static vhdl_type *get_signal_type(ivl_signal_t sig)
{
   int width = ivl_signal_width(sig);
   if (width == 1)
      return vhdl_type::std_logic();
   else if (ivl_signal_signed(sig))
      return vhdl_type::nsigned(width);
   else
      return vhdl_type::nunsigned(width);
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

      vhdl_type *sig_type = get_signal_type(sig);
      
      std::string name = make_safe_name(sig);
      rename_signal(sig, name);
      
      ivl_signal_port_t mode = ivl_signal_port(sig);
      switch (mode) {
      case IVL_SIP_NONE:
         {
            vhdl_decl *decl = new vhdl_signal_decl(name.c_str(), sig_type);

            // A local signal can have a constant initializer in VHDL
            // This may be found in the signal's nexus
            // TODO: Make this work for multiple words
            vhdl_expr *init = nexus_to_const(ivl_signal_nex(sig, 0));
            if (init != NULL)
               decl->set_initial(init);
            
            ent->get_arch()->get_scope()->add_decl(decl);
         }
         break;
      case IVL_SIP_INPUT:
         ent->get_scope()->add_decl
            (new vhdl_port_decl(name.c_str(), sig_type, VHDL_PORT_IN));
         break;
      case IVL_SIP_OUTPUT:
         ent->get_scope()->add_decl
            (new vhdl_port_decl(name.c_str(), sig_type, VHDL_PORT_OUT));

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
            ent->get_arch()->get_scope()->add_decl(new vhdl_signal_decl(newname.c_str(), reg_type));
            
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
      if (draw_lpm(arch, ivl_scope_lpm(scope, i)) != 0)
         error("Failed to translate LPM");
   }
}

/*
 * Create a VHDL entity for scopes of type IVL_SCT_MODULE.
 */
static vhdl_entity *create_entity_for(ivl_scope_t scope)
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
   
   // Locate all the signals in this module and add them to
   // the architecture
   declare_signals(ent, scope);

   // Similarly, add all the primitive logic gates
   declare_logic(arch, scope);

   // ...and all the LPM devices
   declare_lpm(arch, scope);
   
   // Build a comment to add to the entity/architecture
   std::ostringstream ss;
   ss << "Generated from Verilog module " << ivl_scope_tname(scope);
   
   arch->set_comment(ss.str());
   ent->set_comment(ss.str());
   
   remember_entity(ent);
   return ent;
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

   vhdl_expr *to_e = nexus_to_expr(parent->get_arch()->get_scope(), nexus, to);
   assert(to_e);

   // The expressions in a VHDL port map must be 'globally static'
   // i.e. they can't be arbitrary expressions
   // To handle this, only vhdl_var_refs are mapped automatically
   // Otherwise a temporary variable is created to store the
   // result of the expression and that is mapped to the port
   // This is actually a bit stricter than necessary: but turns out
   // to be much easier to implement
   std::string name = make_safe_name(to);
   vhdl_var_ref *to_ref;
   if ((to_ref = dynamic_cast<vhdl_var_ref*>(to_e))) {
      inst->map_port(name.c_str(), to_ref);
   }
   else {
      // Not a static expression
      std::string tmpname(inst->get_inst_name().c_str());
      tmpname += "_";
      tmpname += name;
      tmpname += "_Expr";
      
      vhdl_type *tmptype = new vhdl_type(*to_e->get_type());
      parent->get_arch()->get_scope()->add_decl
         (new vhdl_signal_decl(tmpname.c_str(), tmptype));

      vhdl_var_ref *tmp_ref1 = new vhdl_var_ref(tmpname.c_str(), NULL);
      parent->get_arch()->add_stmt(new vhdl_cassign_stmt(tmp_ref1, to_e));

      vhdl_var_ref *tmp_ref2 = new vhdl_var_ref(*tmp_ref1);
      inst->map_port(name.c_str(), tmp_ref2);
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
 * Instantiate an entity in the hierarchy, and possibly create
 * that entity if it hasn't been encountered yet.
 */
static int draw_module(ivl_scope_t scope, ivl_scope_t parent)
{
   assert(ivl_scope_type(scope) == IVL_SCT_MODULE);

   // Maybe we need to create this entity first?
   vhdl_entity *ent = find_entity(ivl_scope_tname(scope)); 
   if (NULL == ent)
      ent = create_entity_for(scope);
   assert(ent);

   // Is this module instantiated inside another?
   if (parent != NULL) {
      vhdl_entity *parent_ent = find_entity(ivl_scope_tname(parent));
      assert(parent_ent != NULL);

      // Make sure we only collect instantiations from *one*
      // example of this module in the hieararchy
      if (parent_ent->get_derived_from() == ivl_scope_name(parent)) {

         vhdl_arch *parent_arch = parent_ent->get_arch();
         assert(parent_arch != NULL);
         
         // Create a forward declaration for it
         if (!parent_arch->get_scope()->have_declared(ent->get_name())) {
            vhdl_decl *comp_decl = vhdl_component_decl::component_decl_for(ent);
            parent_arch->get_scope()->add_decl(comp_decl);
         }
         
         // And an instantiation statement
         std::string inst_name(ivl_scope_basename(scope));
         if (inst_name == ent->get_name()) {
            // Cannot have instance name the same as type in VHDL
            inst_name += "_Inst";
         }
                     
         vhdl_comp_inst *inst =
            new vhdl_comp_inst(inst_name.c_str(), ent->get_name().c_str());
         port_map(scope, parent_ent, inst);         

         parent_arch->add_stmt(inst);
      }
      else {
         // Ignore this instantiation (already accounted for)
      }
   }
   
   return 0;
}

/*
 * Create a VHDL function from a Verilog function definition.
 */
int draw_function(ivl_scope_t scope, ivl_scope_t parent)
{
   assert(ivl_scope_type(scope) == IVL_SCT_FUNCTION);

   // Find the containing entity
   vhdl_entity *ent = find_entity(ivl_scope_tname(parent));
   assert(ent);

   const char *funcname = ivl_scope_tname(scope);

   // Has this function been declared already?
   // (draw_function will be invoked multiple times for
   // the same function if it appears multiple times in
   // the design hierarchy)
   if (ent->get_arch()->get_scope()->have_declared(funcname))
      return 0;

   // The return type is worked out from the output port
   vhdl_function *func = new vhdl_function(funcname, NULL);
   
   int nsigs = ivl_scope_sigs(scope);
   for (int i = 0; i < nsigs; i++) {
      ivl_signal_t sig = ivl_scope_sig(scope, i);            
      vhdl_type *sigtype = get_signal_type(sig);
      
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
   
   draw_stmt(func, func->get_container(), ivl_scope_def(scope));
   
   ent->get_arch()->get_scope()->add_decl(func);   
   return 0;
}

int draw_scope(ivl_scope_t scope, void *_parent)
{
   ivl_scope_t parent = static_cast<ivl_scope_t>(_parent);
   
   ivl_scope_type_t type = ivl_scope_type(scope);
   int rc = 0;
   switch (type) {
   case IVL_SCT_MODULE:
      rc = draw_module(scope, parent);
      break;
   case IVL_SCT_FUNCTION:
      rc = draw_function(scope, parent);
      break;
   default:
      error("No VHDL conversion for %s (at %s)",
            ivl_scope_tname(scope),
            ivl_scope_name(scope));
      break;
   }
   if (rc != 0)
      return rc;
   
   rc = ivl_scope_children(scope, draw_scope, scope);
   if (rc != 0)
      return rc;
   
   return 0;
}

