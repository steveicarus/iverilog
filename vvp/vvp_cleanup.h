#ifndef __vvp_cleanup_H
#define __vvp_cleanup_H
/*
 * Copyright (c) 2009 Cary R. (cygcary@yahoo.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include "pointers.h"
# include "vpi_priv.h"
# include "vvp_net.h"

/* Routines used to cleanup the runtime memory when it is all finished. */

extern void codespace_delete(void);
extern void dec_str_delete(void);
extern void def_table_delete(void);
extern void island_delete(void);
extern void vpi_mcd_delete(void);
extern void load_module_delete(void);
extern void modpath_delete(void);
extern void root_table_delete(void);
extern void signal_pool_delete(void);
extern void udp_defns_delete(void);
extern void vpi_handle_delete(void);
extern void vvp_net_pool_delete(void);
extern void ufunc_pool_delete(void);

extern void A_delete(struct __vpiHandle *item);
extern void PV_delete(struct __vpiHandle *item);
extern void constant_delete(struct __vpiHandle *item);
extern void contexts_delete(struct __vpiScope *scope);
extern void memory_delete(struct __vpiHandle *item);
extern void named_event_delete(struct __vpiHandle *item);
extern void parameter_delete(struct __vpiHandle *item);
extern void signal_delete(struct __vpiHandle *item);
extern void real_delete(struct __vpiHandle *item);
extern void thread_vthr_delete(struct __vpiHandle *item);
extern void thread_word_delete(struct __vpiHandle *item);
extern void vpi_call_delete(struct __vpiHandle *item);
extern void exec_ufunc_delete(vvp_code_t euf_code);
extern void vthreads_delete(vthread_t base);
extern void vvp_net_delete(vvp_net_t *item);

#endif
