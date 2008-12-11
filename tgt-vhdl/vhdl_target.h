#ifndef INC_VHDL_TARGET_H
#define INC_VHDL_TARGET_H

#include "vhdl_config.h"
#include "ivl_target.h"

#include "vhdl_syntax.hh"
#include "vhdl_type.hh"

#include "support.hh"

#include <string>

using namespace std;

void error(const char *fmt, ...);
void debug_msg(const char *fmt, ...);

int draw_scope(ivl_scope_t scope, void *_parent);
int draw_process(ivl_process_t net, void *cd);
int draw_stmt(vhdl_procedural *proc, stmt_container *container,
              ivl_statement_t stmt, bool is_last = false);
int draw_lpm(vhdl_arch *arch, ivl_lpm_t lpm);
void draw_logic(vhdl_arch *arch, ivl_net_logic_t log);

vhdl_expr *translate_expr(ivl_expr_t e);
vhdl_expr *translate_time_expr(ivl_expr_t e);

void remember_entity(vhdl_entity *ent);
vhdl_entity *find_entity(const string &sname);

ivl_design_t get_vhdl_design();
vhdl_entity *get_active_entity();
void set_active_entity(vhdl_entity *ent);

vhdl_var_ref *nexus_to_var_ref(vhdl_scope *arch_scope, ivl_nexus_t nexus);

bool seen_signal_before(ivl_signal_t sig);
void remember_signal(ivl_signal_t sig, vhdl_scope *scope);
void rename_signal(ivl_signal_t sig, const string &renamed);
vhdl_scope *find_scope_for_signal(ivl_signal_t sig);
const string &get_renamed_signal(ivl_signal_t sig);
ivl_signal_t find_signal_named(const string &name, const vhdl_scope *scope);

int draw_stask_display(vhdl_procedural *proc, stmt_container *container,
                       ivl_statement_t stmt, bool newline = true);
void prune_wait_for_0(stmt_container *container);   
void require_support_function(support_function_t f);

#endif /* #ifndef INC_VHDL_TARGET_H */
 
