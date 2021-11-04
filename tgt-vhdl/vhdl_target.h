// -*- mode: c++ -*-
#ifndef INC_VHDL_TARGET_H
#define INC_VHDL_TARGET_H

#include "vhdl_config.h"
#include "ivl_target.h"

#include "support.hh"
#include "vhdl_syntax.hh"

#include <string>

void error(const char *fmt, ...);
void debug_msg(const char *fmt, ...);

int draw_scope(ivl_scope_t scope, void *_parent);
extern "C" int draw_process(ivl_process_t net, void *cd);
int draw_stmt(vhdl_procedural *proc, stmt_container *container,
              ivl_statement_t stmt, bool is_last = false);
int draw_lpm(vhdl_arch *arch, ivl_lpm_t lpm);
void draw_logic(vhdl_arch *arch, ivl_net_logic_t log);

vhdl_expr *translate_expr(ivl_expr_t e);
vhdl_expr *translate_time_expr(ivl_expr_t e);

ivl_design_t get_vhdl_design();
vhdl_var_ref *nexus_to_var_ref(vhdl_scope *arch_scope, ivl_nexus_t nexus);
vhdl_var_ref* readable_ref(vhdl_scope* scope, ivl_nexus_t nex);
std::string make_safe_name(ivl_signal_t sig);
void require_support_function(support_function_t f);

#endif /* #ifndef INC_VHDL_TARGET_H */
