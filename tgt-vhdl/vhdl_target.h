#ifndef INC_VHDL_TARGET_H
#define INC_VHDL_TARGET_H

#include "vhdl_config.h"
#include "ivl_target.h"

#include "vhdl_element.hh"

#include <string>

void error(const char *fmt, ...);

int draw_scope(ivl_scope_t scope, void *_parent);
int draw_process(ivl_process_t net, void *cd);
int draw_stmt(vhdl_process *proc, ivl_statement_t stmt);

void remember_entity(vhdl_entity *ent);
vhdl_entity *find_entity(const std::string &tname);

#endif /* #ifndef INC_VHDL_TARGET_H */
 
