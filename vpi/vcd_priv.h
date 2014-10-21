#ifndef IVL_vcd_priv_H
#define IVL_vcd_priv_H
/*
 * Copyright (c) 2003-2014 Stephen Williams (steve@icarus.com)
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

#include "vpi_user.h"

#ifdef __cplusplus
# define EXTERN extern "C"
#else
# define EXTERN extern
#endif

EXTERN int is_escaped_id(const char *name);

struct vcd_names_s;
EXTERN struct stringheap_s name_heap;

struct vcd_names_list_s {
      struct vcd_names_s *vcd_names_list;
      const char **vcd_names_sorted;
      int listed_names, sorted_names;
};

EXTERN void vcd_names_add(struct vcd_names_list_s*tab, const char *name);

EXTERN const char *vcd_names_search(struct vcd_names_list_s*tab,
				    const char *key);

EXTERN void vcd_names_sort(struct vcd_names_list_s*tab);

EXTERN void vcd_names_delete(struct vcd_names_list_s*tab);

/*
 * Keep a map of nexus ident's to help with alias detection.
 */
EXTERN const char*find_nexus_ident(int nex);
EXTERN void       set_nexus_ident(int nex, const char *id);

EXTERN void nexus_ident_delete(void);

/*
 * Keep a set of scope names to help with duplicate detection.
 */
EXTERN void vcd_scope_names_add(const char*name);
EXTERN int  vcd_scope_names_test(const char*name);
EXTERN void vcd_scope_names_delete(void);

/*
 * Implement a work queue that can be used to send commands to a
 * dumper thread.
 */

typedef enum vcd_work_item_type_e {
      WT_NONE,
      WT_EMIT_BITS,
      WT_EMIT_DOUBLE,
      WT_DUMPON,
      WT_DUMPOFF,
      WT_FLUSH,
      WT_TERMINATE
} vcd_work_item_type_t;

struct lxt2_wr_symbol;

struct vcd_work_item_s {
      vcd_work_item_type_t type;
      uint64_t time;
      union {
	    struct lxt2_wr_symbol*lxt2;
      } sym_;

      union {
	    double val_double;
	    char*val_char;
      } op_;
};

/*
 * The thread_peek and thread_pop functions work as pairs. The work
 * thread processing work items uses vcd_work_thread_peek to look at
 * the first item in the work queue. The work thread can be assured
 * that the work item it stable. When it is done with the work item,
 * it calls vcd_work_thread_pop to cause it to be popped from the work
 * queue.
 */
EXTERN struct vcd_work_item_s* vcd_work_thread_peek(void);
EXTERN void vcd_work_thread_pop(void);

/*
 * Create work threads with the vcd_work_start function, and terminate
 * the work thread (gracefully) with the vcd_work_terminate
 * function. Synchronize with the work thread with the vcd_work_sync
 * function. This blocks until the work thread is done all the work it
 * has so far.
 */
EXTERN void vcd_work_start( void* (*fun) (void*arg), void*arg);
EXTERN void vcd_work_terminate(void);

EXTERN void vcd_work_sync(void);

/*
 * The remaining vcd_work_* functions send messages to the work thread
 * causing it to perform various VCD-related tasks.
 */
EXTERN void vcd_work_flush(void); /* Drain output caches. */
EXTERN void vcd_work_set_time(uint64_t val);
EXTERN void vcd_work_dumpon(void);
EXTERN void vcd_work_dumpoff(void);
EXTERN void vcd_work_emit_double(struct lxt2_wr_symbol*sym, double val);
EXTERN void vcd_work_emit_bits(struct lxt2_wr_symbol*sym, const char*bits);

/* The compiletf routines are common for the VCD, LXT and LXT2 dumpers. */
EXTERN PLI_INT32 sys_dumpvars_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name);

#undef EXTERN

#endif /* IVL_vcd_priv_H */
