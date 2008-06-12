#ifndef INC_VHDL_TARGET_H
#define INC_VHDL_TARGET_H

#include "vhdl_config.h"
#include "ivl_target.h"

#include "vhdl_syntax.hh"
#include "vhdl_type.hh"

#include <string>

void error(const char *fmt, ...);

int draw_scope(ivl_scope_t scope, void *_parent);
int draw_process(ivl_process_t net, void *cd);
int draw_stmt(vhdl_process *proc, stmt_container *container,
              ivl_statement_t stmt);

vhdl_expr *translate_expr(ivl_expr_t e);

void remember_entity(vhdl_entity *ent);
vhdl_entity *find_entity(const std::string &tname);


void remember_signal(ivl_signal_t sig, const vhdl_entity *ent);
void rename_signal(ivl_signal_t sig, const std::string &renamed);
const vhdl_entity *find_entity_for_signal(ivl_signal_t sig);
const std::string &get_renamed_signal(ivl_signal_t sig);


#endif /* #ifndef INC_VHDL_TARGET_H */
 
