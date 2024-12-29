#ifndef IVL_pform_H
#define IVL_pform_H
/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "netlist.h"
# include  "HName.h"
# include  "named.h"
# include  "Module.h"
# include  "Statement.h"
# include  "AStatement.h"
# include  "PGate.h"
# include  "PExpr.h"
# include  "PTask.h"
# include  "PUdp.h"
# include  "PWire.h"
# include  "PTimingCheck.h"
# include  "verinum.h"
# include  "discipline.h"
# include  <iostream>
# include  <string>
# include  <list>
# include  <memory>
# include  <cstdio>

/*
 * These classes implement the parsed form (P-form for short) of the
 * original Verilog source. the parser generates the pform for the
 * convenience of later processing steps.
 */


/*
 * Wire objects represent the named wires (of various flavor) declared
 * in the source.
 *
 * Gate objects are the functional modules that are connected together
 * by wires.
 *
 * Wires and gates, connected by joints, represent a netlist. The
 * netlist is therefore a representation of the desired circuit.
 */
class PGate;
class PExpr;
class PPackage;
class PSpecPath;
class PClass;
class PPackage;
struct vlltype;
struct lgate;

/*
 * The min:typ:max expression s selected at parse time using the
 * enumeration. When the compiler makes a choice, it also prints a
 * warning if min_typ_max_warn > 0.
 */
extern enum MIN_TYP_MAX { MIN, TYP, MAX } min_typ_max_flag;
extern unsigned min_typ_max_warn;
PExpr* pform_select_mtm_expr(PExpr*min, PExpr*typ, PExpr*max);

/*
 * This flag is true if the lexor thinks we are in a library source
 * file.
 */
extern bool pform_library_flag;

/*
 * These type are lexical types -- that is, types that are used as
 * lexical values to decorate the parse tree during parsing. They are
 * not in any way preserved once parsing is done.
 */

/* This is information about port name information for named port
   connections. */


struct parmvalue_t {
      std::list<PExpr*>*by_order;
      std::list<named_pexpr_t>*by_name;
};

struct str_pair_t { ivl_drive_t str0, str1; };

  /* Use this function to transform the parted form of the attribute
     list to the attribute map that is used later. */
extern void pform_bind_attributes(std::map<perm_string,PExpr*>&attributes,
				  std::list<named_pexpr_t>*attr,
				  bool keep_attr =false);

  /* The lexor calls this function to change the default nettype. */
extern void pform_set_default_nettype(NetNet::Type net,
				     const char*file,
				     unsigned lineno);

  /* Return true if currently processing a program block. This can be
     used to reject statements that cannot exist in program blocks. */
extern bool pform_in_program_block(void);

  /* Return true if currently processing an interface. This can be
     used to reject statements that cannot exist in interfaces. */
extern bool pform_in_interface(void);

/*
 * Look for the given wire in the current lexical scope. If the wire
 * (including variables of any type) cannot be found in the current
 * scope, then return 0.
 */
extern PWire* pform_get_wire_in_scope(perm_string name);

extern PWire* pform_get_make_wire_in_scope(const struct vlltype&li,
                                           perm_string name,
                                           NetNet::Type net_type,
                                           ivl_variable_type_t vt_type);

/*
 * The parser uses startmodule and endmodule together to build up a
 * module as it parses it. The startmodule tells the pform code that a
 * module has been noticed in the source file and the following events
 * are to apply to the scope of that module. The endmodule causes the
 * pform to close up and finish the named module.
 *
 * The program_block flag indicates that the module is actually a program
 * block. The is_interface flag indicates that the module is actually
 * an interface. These flags have implications during parse and during
 * elaboration/code generation.
 */
extern void pform_startmodule(const struct vlltype&loc, const char*name,
			      bool program_block, bool is_interface,
			      LexicalScope::lifetime_t lifetime,
			      std::list<named_pexpr_t>*attr);
extern void pform_module_set_ports(std::vector<Module::port_t*>*);
extern void pform_set_scope_timescale(const struct vlltype&loc);

/* These functions are used when we have a complete port definition, either
   in an ansi style or non-ansi style declaration. In this case, we have
   everything needed to define the port, all in one place. */
extern void pform_module_define_port(const struct vlltype&li,
				     const pform_ident_t&name,
				     NetNet::PortType,
				     NetNet::Type type,
				     data_type_t*vtype,
				     std::list<pform_range_t>*urange,
				     std::list<named_pexpr_t>*attr,
				     bool keep_attr =false);
extern void pform_module_define_port(const struct vlltype&li,
				     std::list<pform_port_t>*ports,
				     NetNet::PortType,
				     NetNet::Type type,
				     data_type_t*vtype,
				     std::list<named_pexpr_t>*attr);

extern Module::port_t* pform_module_port_reference(const struct vlltype&loc,
						   perm_string name);
extern void pform_endmodule(const char*, bool inside_celldefine,
                            Module::UCDriveType uc_drive_def);

extern void pform_start_class_declaration(const struct vlltype&loc,
					  class_type_t*type,
					  data_type_t*base_type,
					  std::list<named_pexpr_t> *base_args,
					  bool virtual_class);
extern void pform_class_property(const struct vlltype&loc,
				 property_qualifier_t pq,
				 data_type_t*data_type,
				 std::list<decl_assignment_t*>*decls);
extern void pform_set_this_class(const struct vlltype&loc, PTaskFunc*net);
extern void pform_set_constructor_return(PFunction*net);

extern void pform_end_class_declaration(const struct vlltype&loc);
extern bool pform_in_class();

extern void pform_make_udp(const struct vlltype&loc, perm_string name,
			   std::list<pform_ident_t>*parms,
			   std::vector<PWire*>*decl, std::list<std::string>*table,
			   Statement*init);

extern void pform_make_udp(const struct vlltype&loc, perm_string name,
			   bool sync_flag, const pform_ident_t&out_name,
			   PExpr*sync_init,
			   std::list<pform_ident_t>*parms,
			   std::list<std::string>*table);
/*
 * Package related functions.
 */
extern void pform_start_package_declaration(const struct vlltype&loc,
					    const char*type,
					    LexicalScope::lifetime_t lifetime);
extern void pform_end_package_declaration(const struct vlltype&loc);
extern void pform_package_import(const struct vlltype&loc,
				 PPackage*pkg, const char*ident);
extern void pform_package_export(const struct vlltype &loc, PPackage *pkg,
			         const char *ident);
PPackage *pform_package_importable(PPackage *pkg, perm_string name);
PPackage *pform_find_potential_import(const struct vlltype&loc, LexicalScope*scope,
				      perm_string name, bool tf_call, bool make_explicit);


extern PExpr* pform_package_ident(const struct vlltype&loc,
				  PPackage*pkg, pform_name_t*ident);

/*
 * Interface related functions.
 */
extern void pform_start_modport_item(const struct vlltype&loc, const char*name);
extern void pform_end_modport_item(const struct vlltype&loc);
extern void pform_add_modport_port(const struct vlltype&loc,
	                           NetNet::PortType port_type,
	                           perm_string name, PExpr*expr);

/*
 * This creates an identifier aware of names that may have been
 * imported from other packages.
 */
extern PEIdent* pform_new_ident(const struct vlltype&loc, const pform_name_t&name);

extern PTrigger* pform_new_trigger(const struct vlltype&loc, PPackage*pkg,
				   const pform_name_t&name, unsigned lexical_pos);
extern PNBTrigger* pform_new_nb_trigger(const struct vlltype&loc,
				        const std::list<PExpr*>*dly,
				        const pform_name_t&name,
				        unsigned lexical_pos);

/*
 * Enter/exit name scopes. The push_scope function pushes the scope
 * name string onto the scope hierarchy. The pop pulls it off and
 * deletes it. Thus, the string pushed must be allocated.
 */
extern void pform_pop_scope();

/*
 * Peek at the current (most recently active) scope.
 */
extern LexicalScope* pform_peek_scope();

extern PClass* pform_push_class_scope(const struct vlltype&loc, perm_string name);

extern PFunction*pform_push_constructor_scope(const struct vlltype&loc);

extern PPackage* pform_push_package_scope(const struct vlltype&loc, perm_string name,
					  LexicalScope::lifetime_t lifetime);

extern PTask*pform_push_task_scope(const struct vlltype&loc, char*name,
				   LexicalScope::lifetime_t lifetime);

extern PFunction*pform_push_function_scope(const struct vlltype&loc, const char*name,
					   LexicalScope::lifetime_t lifetime);

extern PBlock*pform_push_block_scope(const struct vlltype&loc, char*name,
				     PBlock::BL_TYPE tt);

extern void pform_put_behavior_in_scope(AProcess*proc);

extern verinum* pform_verinum_with_size(verinum*s, verinum*val,
					const char*file, unsigned lineno);

/*
 * This function takes the list of names as new genvars to declare in
 * the current module or generate scope.
 */
extern void pform_genvars(const struct vlltype&li, std::list<pform_ident_t>*names);

/*
 * This flag is set by the parser to indicate the current generate block
 * was not enclosed in a begin-end pair. This is a pre-requisite for
 * directly nesting generate constructs.
 */
extern bool pform_generate_single_item;

extern void pform_start_generate_for(const struct vlltype&li,
				     bool local_index,
				     char*ident1,
				     PExpr*init,
				     PExpr*test,
				     char*ident2,
				     PExpr*next);
extern void pform_start_generate_if(const struct vlltype&li, PExpr*test);
extern void pform_start_generate_else(const struct vlltype&li);
extern void pform_start_generate_case(const struct vlltype&lp, PExpr*test);
extern void pform_start_generate_nblock(const struct vlltype&lp, char*name);
extern void pform_generate_case_item(const struct vlltype&lp, std::list<PExpr*>*test);
extern void pform_generate_block_name(char*name);
extern void pform_endgenerate(bool end_conditional);

/*
 * This function returns the lexically containing generate scheme, if
 * there is one. The parser may use this to check if we are within a
 * generate scheme.
 */
extern PGenerate* pform_parent_generate(void);

bool pform_error_in_generate(const vlltype&loc, const char *type);

extern void pform_make_elab_task(const struct vlltype&li,
                                 perm_string name,
                                 const std::list<named_pexpr_t> &params);

extern void pform_set_typedef(const struct vlltype&loc, perm_string name,
			      data_type_t*data_type,
			      std::list<pform_range_t>*unp_ranges = nullptr);
extern void pform_forward_typedef(const struct vlltype&loc, perm_string name,
			      enum typedef_t::basic_type basic_type);

extern void pform_set_type_referenced(const struct vlltype&loc, const char*name);

/*
 * This function makes a PECallFunction of the named function.
 */
extern PECallFunction* pform_make_call_function(const struct vlltype&loc,
						const pform_name_t&name,
						const std::list<named_pexpr_t> &parms);
extern PCallTask* pform_make_call_task(const struct vlltype&loc,
				       const pform_name_t&name,
				       const std::list<named_pexpr_t> &parms);

extern void pform_make_foreach_declarations(const struct vlltype&loc,
					    std::list<perm_string>*loop_vars);
extern PForeach* pform_make_foreach(const struct vlltype&loc,
				    char*ident,
				    std::list<perm_string>*loop_vars,
				    Statement*stmt);

/*
 * The makewire functions announce to the pform code new wires. These
 * go into a module that is currently opened.
 */
extern PWire *pform_makewire(const struct vlltype&li,
                             const pform_ident_t&name,
			     NetNet::Type type,
			     std::list<pform_range_t> *indices);

/* This form handles assignment declarations. */

extern void pform_makewire(const struct vlltype&li,
			   std::list<PExpr*>*delay,
			   str_pair_t str,
			   std::list<decl_assignment_t*>*assign_list,
			   NetNet::Type type,
			   data_type_t*data_type,
			   std::list<named_pexpr_t>*attr = 0,
			   bool is_const = false);

extern void pform_make_var(const struct vlltype&loc,
			   std::list<decl_assignment_t*>*assign_list,
			   data_type_t*data_type,
			   std::list<named_pexpr_t>*attr = 0,
			   bool is_const = false);

extern void pform_make_var_init(const struct vlltype&li,
				const pform_ident_t&name,
				PExpr*expr);

/* This function is used when we have an incomplete port definition in
   a non-ansi style declaration. Look up the names of the wires, and set
   the port type, i.e. input, output or inout, and, if specified, the
   range and signedness. If the wire does not exist, create it. */
extern void pform_set_port_type(const struct vlltype&li,
				std::list<pform_port_t>*ports,
				NetNet::PortType,
				data_type_t*dt,
				std::list<named_pexpr_t>*attr);

extern void pform_set_data_type(const struct vlltype&li,
				data_type_t *data_type,
				std::vector<PWire*> *wires,
				NetNet::Type net_type,
				std::list<named_pexpr_t>*attr,
				bool is_const = false);

extern void pform_set_string_type(const struct vlltype&li, const string_type_t*string_type, std::list<perm_string>*names, NetNet::Type net_type, std::list<named_pexpr_t>*attr);

extern void pform_set_class_type(const struct vlltype&li, class_type_t*class_type, std::list<perm_string>*names, NetNet::Type net_type, std::list<named_pexpr_t>*addr);


  /* pform_set_attrib and pform_set_type_attrib exist to support the
     $attribute syntax, which can only set string values to
     attributes. The functions keep the value strings that are
     passed in. */
extern void pform_set_attrib(perm_string name, perm_string key,
			     char*value);
extern void pform_set_type_attrib(perm_string name, const std::string&key,
				  char*value);

extern LexicalScope::range_t* pform_parameter_value_range(bool exclude_flag,
						    bool low_open, PExpr*low_expr,
						    bool hig_open, PExpr*hig_expr);

extern void pform_set_parameter(const struct vlltype&loc,
				perm_string name,
				bool is_local, bool is_type,
				data_type_t*data_type, std::list<pform_range_t>*udims,
				PExpr*expr, LexicalScope::range_t*value_range);
extern void pform_set_specparam(const struct vlltype&loc,
				 perm_string name,
				 std::list<pform_range_t>*range,
				 PExpr*expr);
extern void pform_set_defparam(const pform_name_t&name, PExpr*expr);

extern void pform_make_let(const struct vlltype&loc,
                           perm_string name,
                           std::list<PLet::let_port_t*>*ports,
                           PExpr*expr);

extern PLet::let_port_t* pform_make_let_port(data_type_t*data_type,
                                             perm_string name,
                                             std::list<pform_range_t>*range,
                                             PExpr*def);

/*
 * Functions related to specify blocks.
 */
extern PSpecPath*pform_make_specify_path(const struct vlltype&li,
					 std::list<perm_string>*src, char pol,
					 bool full_flag, std::list<perm_string>*dst);
extern PSpecPath*pform_make_specify_edge_path(const struct vlltype&li,
					 int edge_flag, /*posedge==true */
					 std::list<perm_string>*src, char pol,
					 bool full_flag, std::list<perm_string>*dst,
					 PExpr*data_source_expression);
extern PSpecPath*pform_assign_path_delay(PSpecPath*obj, std::list<PExpr*>*delays);

extern void pform_module_specify_path(PSpecPath*obj);

/*
 * Functions related to timing checks.
 */
extern PRecRem* pform_make_recrem(const struct vlltype&li,
			    PTimingCheck::event_t*reference_event,
			    PTimingCheck::event_t*data_event,
			    PExpr*setup_limit,
			    PExpr*hold_limit,
			    PTimingCheck::optional_args_t* args
			    );
extern PSetupHold* pform_make_setuphold(const struct vlltype&li,
			    PTimingCheck::event_t*reference_event,
			    PTimingCheck::event_t*data_event,
			    PExpr*setup_limit,
			    PExpr*hold_limit,
			    PTimingCheck::optional_args_t* args
			    );
extern void pform_module_timing_check(PTimingCheck*obj);

/*
 * pform_make_behavior creates processes that are declared with always
 * or initial items.
 */
extern PProcess*  pform_make_behavior(ivl_process_type_t, Statement*,
				      std::list<named_pexpr_t>*attr);
extern void pform_mc_translate_on(bool flag);

extern std::vector<PWire*>* pform_make_udp_input_ports(std::list<pform_ident_t>*);

extern void pform_make_events(const struct vlltype&loc,
			      const std::list<pform_ident_t>*names);
/*
 * The makegate function creates a new gate (which need not have a
 * name) and connects it to the specified wires.
 */
extern void pform_makegates(const struct vlltype&loc,
			    PGBuiltin::Type type,
			    struct str_pair_t str,
			    std::list<PExpr*>*delay,
			    std::vector<lgate>*gates,
			    std::list<named_pexpr_t>*attr);

extern void pform_make_modgates(const struct vlltype&loc,
				perm_string type,
				struct parmvalue_t*overrides,
				std::vector<lgate>*gates,
				std::list<named_pexpr_t>*attr);

/* Make a continuous assignment node, with optional bit- or part- select. */
extern void pform_make_pgassign_list(const struct vlltype&loc,
				     std::list<PExpr*>*alist,
				     std::list<PExpr*>*del,
				     struct str_pair_t str);

extern std::vector<pform_tf_port_t>*pform_make_task_ports(const struct vlltype&loc,
					     NetNet::PortType pt,
					     data_type_t*vtype,
					     std::list<pform_port_t>*ports,
					     bool allow_implicit = false);

/*
 * The parser uses this function to convert a unary
 * increment/decrement expression to the equivalent compressed
 * assignment statement.
 */
extern PAssign* pform_compressed_assign_from_inc_dec(const struct vlltype&loc,
						     PExpr*exp);

/*
 * The parser uses this function to convert a genvar increment/decrement
 * expression to the equivalent binary add/subtract expression.
 */
extern PExpr* pform_genvar_inc_dec(const struct vlltype&loc, const char*name,
                                   bool inc_flag);

extern PExpr* pform_genvar_compressed(const struct vlltype &loc,
				      const char *name, char op, PExpr *rval);

/*
 * These are functions that the outside-the-parser code uses the do
 * interesting things to the Verilog. The parse function reads and
 * parses the source file and places all the modules it finds into the
 * mod list. The dump function dumps a module to the output stream.
 */
extern void pform_dump(std::ostream&out, Module*mod);

/* ** pform_discipline.cc
 * Functions for handling the parse of natures and disciplines. These
 * functions are in pform_disciplines.cc
 */

extern void pform_start_nature(const char*name);
extern void pform_end_nature(const struct vlltype&loc);

extern void pform_nature_access(const struct vlltype&loc, const char*name);

extern void pform_start_discipline(const char*name);
extern void pform_end_discipline(const struct vlltype&loc);

extern void pform_discipline_domain(const struct vlltype&loc, ivl_dis_domain_t use_domain);
extern void pform_discipline_potential(const struct vlltype&loc, const char*name);
extern void pform_discipline_flow(const struct vlltype&loc, const char*name);

extern void pform_attach_discipline(const struct vlltype&loc,
				    ivl_discipline_t discipline, std::list<pform_ident_t>*names);

extern void pform_dump(std::ostream&out, const ivl_nature_s*);
extern void pform_dump(std::ostream&out, const ivl_discipline_s*);

/* ** pform_analog.cc
*/
extern void pform_make_analog_behavior(const struct vlltype&loc,
				       ivl_process_type_t type, Statement*st);

extern AContrib*pform_contribution_statement(const struct vlltype&loc,
					     PExpr*lval, PExpr*rval);

extern PExpr* pform_make_branch_probe_expression(const struct vlltype&loc,
						 char*name, char*n1, char*n2);

extern PExpr* pform_make_branch_probe_expression(const struct vlltype&loc,
						 char*name, char*branch);

/*
 * Parse configuration file with format <key>=<value>, where key
 * is the hierarchical name of a valid parameter name and value
 * is the value user wants to assign to. The value should be constant.
 */
extern void parm_to_defparam_list(const std::string&param);

/*
 * Tasks to set the timeunit or timeprecision for SystemVerilog.
 */
extern bool get_time_unit(const char*cp, int &unit);
extern int  pform_get_timeunit();
extern int  pform_get_timeprec();
extern void pform_set_timeunit(const char*txt, bool initial_decl);
extern void pform_set_timeprec(const char*txt, bool initial_decl);
/*
 * Flags to determine whether this is an initial declaration.
 */
extern bool allow_timeunit_decl;
extern bool allow_timeprec_decl;

void pform_put_enum_type_in_scope(enum_type_t*enum_set);

bool pform_requires_sv(const struct vlltype&loc, const char *feature);
void pform_block_decls_requires_sv(void);

void pform_start_parameter_port_list();
void pform_end_parameter_port_list();

void pform_check_net_data_type(const struct vlltype&loc, NetNet::Type net_type,
			       const data_type_t *data_type);

#endif /* IVL_pform_H */
