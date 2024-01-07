/*
 * Copyright (c) 2003-2024 Stephen Williams (steve@icarus.com)
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

/* The sys_priv.h include must be before the lxt2_write.h include! */
# include "sys_priv.h"
# include "lxt2_write.h"
# include "vcd_priv.h"

/*
 * This file contains the implementations of the LXT2 related functions.
 */

# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <time.h>
# include  "stringheap.h"
# include  <assert.h>
# include  "ivl_alloc.h"


static struct lxt2_wr_trace *dump_file = NULL;

static void* lxt2_thread(void*arg);

static struct t_vpi_time zero_delay = { vpiSimTime, 0, 0, 0.0 };

/*
 * Manage a table of all the dump-enabled vcd item. The cells of this
 * table are allocated incrementally, but are released all at once, so
 * we can allocate the items in chunks.
 */
struct vcd_info {
      vpiHandle item;
      vpiHandle cb;
      struct lxt2_wr_symbol *sym;
      struct vcd_info *dmp_next;
};

struct vcd_info_chunk {
      struct vcd_info_chunk*chunk_next;
      uint16_t chunk_fill;
      struct vcd_info data[0x10000];
};

struct vcd_info_chunk*info_chunk_list = 0;

struct vcd_info*new_vcd_info(void)
{
      struct vcd_info_chunk*cur_chunk = info_chunk_list;
      if (cur_chunk == 0 || cur_chunk->chunk_fill == 0xffff) {
	    struct vcd_info_chunk*tmp = calloc(1, sizeof(struct vcd_info_chunk));
	    tmp->chunk_fill = 0;
	    tmp->chunk_next = cur_chunk;
	    info_chunk_list = cur_chunk = tmp;
	    return info_chunk_list->data + 0;
      }

      cur_chunk->chunk_fill += 1;
      struct vcd_info*ptr = cur_chunk->data + cur_chunk->chunk_fill;
      return ptr;
}

void functor_all_vcd_info( void (*fun) (struct vcd_info*info) )
{
      struct vcd_info_chunk*cur;
      for (cur = info_chunk_list ; cur ; cur = cur->chunk_next) {
	    int idx;
	    struct vcd_info*ptr;
	    for (idx=0, ptr=cur->data ; idx <= cur->chunk_fill ; idx += 1, ptr += 1)
		  fun (ptr);
      }
}

void delete_all_vcd_info(void)
{
      while (info_chunk_list) {
	    struct vcd_info_chunk*tmp = info_chunk_list->chunk_next;
	    free(info_chunk_list);
	    info_chunk_list = tmp;
      }
}

/*
 * Use this pointer to keep a list of vcd_info items that are
 * scheduled to be dumped. Use the VCD_INFO_ENDP value as a trailer
 * pointer instead of 0 so that individual vcd_info objects can tell
 * if they are in the list already, even if they are at the end of the
 * list.
 */
# define VCD_INFO_ENDP ((struct vcd_info*)1)
static struct vcd_info *vcd_dmp_list = VCD_INFO_ENDP;

static PLI_UINT64 vcd_cur_time = 0;
static int dump_is_off = 0;
static long dump_limit = 0;
static int dump_is_full = 0;
static int finish_status = 0;


static enum lxm_optimum_mode_e {
      LXM_NONE  = 0,
      LXM_SPACE = 1,
      LXM_SPEED = 2
} lxm_optimum_mode = LXM_SPEED;

static off_t lxt2_file_size_limit = 0x40000000UL;

/*
 * The lxt_scope head and current pointers are used to keep a scope
 * stack that can be accessed from the bottom. The lxt_scope_head
 * points to the first (bottom) item in the stack and
 * lxt_scope_current points to the last (top) item in the stack. The
 * push_scope and pop_scope methods manipulate the stack.
 */
struct lxt_scope
{
      struct lxt_scope *next, *prev;
      char *name;
      int len;
};

static struct lxt_scope *lxt_scope_head=NULL,  *lxt_scope_current=NULL;

static void push_scope(const char *name)
{
      struct lxt_scope *t = (struct lxt_scope *)
	    calloc(1, sizeof(struct lxt_scope));

      t->name = strdup(name);
      t->len = strlen(name);

      if(!lxt_scope_head) {
	    lxt_scope_head = lxt_scope_current = t;
      } else {
	    lxt_scope_current->next = t;
	    t->prev = lxt_scope_current;
	    lxt_scope_current = t;
      }
}

static void pop_scope(void)
{
      struct lxt_scope *t;

      assert(lxt_scope_current);

      t=lxt_scope_current->prev;
      free(lxt_scope_current->name);
      free(lxt_scope_current);
      lxt_scope_current = t;
      if (lxt_scope_current) {
	    lxt_scope_current->next = 0;
      } else {
	    lxt_scope_head = 0;
      }
}

/*
 * This function uses the scope stack to generate a hierarchical
 * name. Scan the scope stack from the bottom up to construct the
 * name.
 */
static char *create_full_name(const char *name)
{
      char *n, *n2;
      int len = 0;
      int is_esc_id = is_escaped_id(name);
      struct lxt_scope *t = lxt_scope_head;

	/* Figure out how long the combined string will be. */
      while(t) {
	    len+=t->len+1;
	    t=t->next;
      }

      len += strlen(name) + 1;
      if (is_esc_id) len += 1;

	/* Allocate a string buffer. */
      n = n2 = malloc(len);

      t = lxt_scope_head;
      while(t) {
	    strcpy(n2, t->name);
	    n2 += t->len;
	    *n2 = '.';
	    n2++;
	    t=t->next;
      }

      if (is_esc_id) {
	    *n2 = '\\';
	    n2++;
      }
      strcpy(n2, name);
      n2 += strlen(n2);
      assert( (n2 - n + 1) == len );

      return n;
}

static void show_this_item(struct vcd_info*info)
{
      s_vpi_value value;

      if (vpi_get(vpiType, info->item) == vpiRealVar) {
	    value.format = vpiRealVal;
	    vpi_get_value(info->item, &value);
	    vcd_work_emit_double(info->sym, value.value.real);

      } else {
	    value.format = vpiBinStrVal;
	    vpi_get_value(info->item, &value);
	    vcd_work_emit_bits(info->sym, value.value.str);
      }
}


static void show_this_item_x(struct vcd_info*info)
{
      if (vpi_get(vpiType,info->item) == vpiRealVar) {
	      /* Should write a NaN here? */
      } else {
	    vcd_work_emit_bits(info->sym, "x");
      }
}


static PLI_UINT64 dumpvars_time;
__inline__ static int dump_header_pending(void)
{
      return dumpvars_status != 2;
}

/*
 * This function writes out all the traced variables, whether they
 * changed or not.
 */
static void vcd_checkpoint(void)
{
      functor_all_vcd_info( show_this_item );
}

static void vcd_checkpoint_x(void)
{
      functor_all_vcd_info( show_this_item_x );
}

static PLI_INT32 variable_cb_2(p_cb_data cause)
{
      assert(cause->time->type == vpiSimTime);
      PLI_UINT64 now = timerec_to_time64(cause->time);

      if (now != vcd_cur_time) {
	    vcd_work_set_time(now);
	    vcd_cur_time = now;
      }

      while (vcd_dmp_list != VCD_INFO_ENDP) {
	    struct vcd_info* info = vcd_dmp_list;
	    show_this_item(info);
	    vcd_dmp_list = info->dmp_next;
	    info->dmp_next = 0;
      }

      return 0;
}

static PLI_INT32 variable_cb_1(p_cb_data cause)
{
      struct t_cb_data cb;
      struct vcd_info*info = (struct vcd_info*)cause->user_data;

      if (dump_is_full) return 0;
      if (dump_is_off) return 0;
      if (dump_header_pending()) return 0;
      if (info->dmp_next) return 0;

      if ((dump_limit > 0) && (ftell(dump_file->handle) > dump_limit)) {
            dump_is_full = 1;
            vpi_printf("WARNING: Dump file limit (%ld bytes) "
                       "exceeded.\n", dump_limit);
            return 0;
      }

      if (vcd_dmp_list == VCD_INFO_ENDP) {
          cb = *cause;
	  cb.time = &zero_delay;
          cb.reason = cbReadOnlySynch;
          cb.cb_rtn = variable_cb_2;
          vpi_register_cb(&cb);
      }

      info->dmp_next  = vcd_dmp_list;
      vcd_dmp_list    = info;

      return 0;
}

static PLI_INT32 dumpvars_cb(p_cb_data cause)
{
      if (dumpvars_status != 1) return 0;

      dumpvars_status = 2;

      dumpvars_time = timerec_to_time64(cause->time);
      vcd_cur_time = dumpvars_time;

      if (!dump_is_off) {
	    vcd_work_set_time(dumpvars_time);
	    vcd_checkpoint();
      }

      return 0;
}

static PLI_INT32 finish_cb(p_cb_data cause)
{
      if (finish_status != 0) return 0;

      finish_status = 1;

      dumpvars_time = timerec_to_time64(cause->time);
      if (!dump_is_off && !dump_is_full && dumpvars_time != vcd_cur_time) {
	    vcd_work_set_time(dumpvars_time);
      }

      vcd_work_terminate();
      delete_all_vcd_info();

      vcd_scope_names_delete();
      nexus_ident_delete();
      vcd_free_dump_path();

      return 0;
}

__inline__ static int install_dumpvars_callback(void)
{
      struct t_cb_data cb;

      if (dumpvars_status == 1) return 0;

      if (dumpvars_status == 2) {
	    vpi_printf("LXT2 warning: $dumpvars ignored, previously"
	               " called at simtime %" PLI_UINT64_FMT "\n",
	               dumpvars_time);
	    return 1;
      }

      cb.time = &zero_delay;
      cb.reason = cbReadOnlySynch;
      cb.cb_rtn = dumpvars_cb;
      cb.user_data = 0x0;
      cb.obj = 0x0;

      vpi_register_cb(&cb);

      cb.reason = cbEndOfSimulation;
      cb.cb_rtn = finish_cb;

      vpi_register_cb(&cb);

      dumpvars_status = 1;
      return 0;
}

static PLI_INT32 sys_dumpoff_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_time now;
      PLI_UINT64 now64;

      (void)name; /* Parameter is not used. */

      if (dump_is_off) return 0;

      dump_is_off = 1;

      if (dump_file == 0) return 0;
      if (dump_header_pending()) return 0;

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      now64 = timerec_to_time64(&now);

      if (now64 > vcd_cur_time) {
	    vcd_work_set_time(now64);
	    vcd_cur_time = now64;
      }

      vcd_work_dumpoff();
      vcd_checkpoint_x();

      return 0;
}

static PLI_INT32 sys_dumpon_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_time now;
      PLI_UINT64 now64;

      (void)name; /* Parameter is not used. */

      if (!dump_is_off) return 0;

      dump_is_off = 0;

      if (dump_file == 0) return 0;
      if (dump_header_pending()) return 0;

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      now64 = timerec_to_time64(&now);

      if (now64 > vcd_cur_time) {
	    vcd_work_set_time(now64);
	    vcd_cur_time = now64;
      }

      vcd_work_dumpon();
      vcd_checkpoint();

      return 0;
}

static PLI_INT32 sys_dumpall_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_time now;
      PLI_UINT64 now64;

      (void)name; /* Parameter is not used. */

      if (dump_is_off) return 0;
      if (dump_file == 0) return 0;
      if (dump_header_pending()) return 0;

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      now64 = timerec_to_time64(&now);

      if (now64 > vcd_cur_time) {
	    vcd_work_set_time(now64);
	    vcd_cur_time = now64;
      }

      vcd_checkpoint();

      return 0;
}

static void *close_dumpfile(void)
{
      vcd_work_terminate();
      lxt2_wr_close(dump_file);
      dump_file = NULL;
      return NULL;
}

static void open_dumpfile(vpiHandle callh)
{
      off_t use_file_size_limit = lxt2_file_size_limit;
      char* use_dump_path = vcd_get_dump_path("lx2");

      dump_file = lxt2_wr_init(use_dump_path);

      if (getenv("LXT_FILE_SIZE_LIMIT")) {
	    const char*limit_string = getenv("LXT_FILE_SIZE_LIMIT");
	    char*ep;
	    use_file_size_limit = strtoul(limit_string,&ep,0);
	    if (use_file_size_limit == 0 || ep[0] != 0) {
		  vpi_printf("LXT2 Warning: %s:%d: LXT_FILE_SIZE_LIMIT is invalid: %s\n",
			     vpi_get_str(vpiFile, callh),
			     (int)vpi_get(vpiLineNo, callh),
			     limit_string);
		  use_file_size_limit = lxt2_file_size_limit;
	    }
      }

      if (dump_file == 0) {
	    vpi_printf("LXT2 Error: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Unable to open %s for output.\n", use_dump_path);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    vcd_free_dump_path();
	    return;
      } else {
	    int prec = vpi_get(vpiTimePrecision, 0);

	    vpi_printf("LXT2 info: dumpfile %s opened for output.\n",
	               use_dump_path);

	    assert(prec >= -15);
	    lxt2_wr_set_timescale(dump_file, prec);

	    lxt2_wr_set_initial_value(dump_file, 'x');
	    lxt2_wr_set_compression_depth(dump_file, 4);
	    lxt2_wr_set_partial_on(dump_file, 1);
	    lxt2_wr_set_break_size(dump_file, use_file_size_limit);

	    vcd_work_start(lxt2_thread, 0);
            atexit((void(*)(void))close_dumpfile);
      }
}

static PLI_INT32 sys_dumpfile_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name;
      return sys_dumpfile_common("LXT2", "lx2");
}

/*
 * The LXT2 format is a binary format, but a $dumpflush causes what is
 * in the binary file at the moment to be consistent with itself so
 * that the waveforms can be read by another process. LXT2 normally
 * writes checkpoints out, but this makes it happen at a specific
 * time.
 */
static PLI_INT32 sys_dumpflush_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */
      if (dump_file) vcd_work_flush();

      return 0;
}

static PLI_INT32 sys_dumplimit_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      s_vpi_value val;

      (void)name; /* Parameter is not used. */

      /* Get the value and set the dump limit. */
      assert(argv);
      val.format = vpiIntVal;
      vpi_get_value(vpi_scan(argv), &val);
      dump_limit = val.value.integer;

      vpi_free_object(argv);
      return 0;
}

static void scan_item(unsigned depth, vpiHandle item, int skip)
{
      static int dumpable_types[] = {
            /* Value */
            /* vpiNamedEvent, */
            vpiNet,
            /* vpiParameter, */
            vpiReg,
            vpiVariables,
            /* Scope */
            vpiFunction,
            vpiGenScope,
            vpiModule,
            vpiNamedBegin,
            vpiNamedFork,
            vpiTask,
            -1
      };

      struct t_cb_data cb;
      struct vcd_info* info;

      const char* name;
      const char* ident;
      int nexus_id;

      switch (vpi_get(vpiType, item)) {

	  case vpiMemoryWord:
	    if (vpi_get(vpiConstantSelect, item) == 0) {
		    /* Turn a non-constant array word select into a
		     * constant word select. */
		  vpiHandle array = vpi_handle(vpiParent, item);
		  PLI_INT32 idx = vpi_get(vpiIndex, item);
		  item = vpi_handle_by_index(array, idx);
	    }
	    // fallthrough
	  case vpiIntegerVar:
	  case vpiBitVar:
	  case vpiByteVar:
	  case vpiShortIntVar:
	  case vpiIntVar:
	  case vpiLongIntVar:
	  case vpiTimeVar:
	  case vpiReg:
	  case vpiNet:

	      /* An array word is implicitly escaped so look for an
	       * escaped identifier that this could conflict with. */
            if (vpi_get(vpiType, item) == vpiMemoryWord &&
                vpi_handle_by_name(vpi_get_str(vpiFullName, item), 0)) {
		  vpi_printf("LXT2 warning: dumping array word %s will "
		             "conflict with an escaped identifier.\n",
		             vpi_get_str(vpiFullName, item));
            }

            if (skip || vpi_get(vpiAutomatic, item)) break;

	    name = vpi_get_str(vpiName, item);
	    nexus_id = vpi_get(_vpiNexusId, item);
	    if (nexus_id) {
		  ident = find_nexus_ident(nexus_id);
	    } else {
		  ident = 0;
	    }

	    if (!ident) {
		  char*tmp = create_full_name(name);
		  ident = strdup_sh(&name_heap, tmp);
		  free(tmp);

		  if (nexus_id) set_nexus_ident(nexus_id, ident);

		  info = new_vcd_info();

		  info->item  = item;
		  info->sym   = lxt2_wr_symbol_add(dump_file, ident,
		                                   0 /* array rows */,
		                                   vpi_get(vpiLeftRange, item),
		                                   vpi_get(vpiRightRange, item),
		                                   LXT2_WR_SYM_F_BITS);
		  info->dmp_next = 0;

		  cb.time      = 0;
		  cb.user_data = (char*)info;
		  cb.value     = NULL;
		  cb.obj       = item;
		  cb.reason    = cbValueChange;
		  cb.cb_rtn    = variable_cb_1;

		  info->cb    = vpi_register_cb(&cb);

	    } else {
		  char *n = create_full_name(name);
		  lxt2_wr_symbol_alias(dump_file, ident, n,
				       vpi_get(vpiSize, item)-1, 0);
		  free(n);
            }

	    break;

	  case vpiRealVar:

            if (skip || vpi_get(vpiAutomatic, item)) break;

	    name = vpi_get_str(vpiName, item);
	    { char*tmp = create_full_name(name);
	      ident = strdup_sh(&name_heap, tmp);
	      free(tmp);
	    }
	    info = new_vcd_info();

	    info->item = item;
	    info->sym  = lxt2_wr_symbol_add(dump_file, ident,
	                                    0 /* array rows */,
	                                    vpi_get(vpiSize, item)-1,
	                                    0, LXT2_WR_SYM_F_DOUBLE);
	    info->dmp_next = 0;

	    cb.time      = 0;
	    cb.user_data = (char*)info;
	    cb.value     = NULL;
	    cb.obj       = item;
	    cb.reason    = cbValueChange;
	    cb.cb_rtn    = variable_cb_1;

	    info->cb    = vpi_register_cb(&cb);

	    break;

	  case vpiModule:
	  case vpiGenScope:
	  case vpiFunction:
	  case vpiTask:
	  case vpiNamedBegin:
	  case vpiNamedFork:

	    if (depth > 0) {
		  const char* fullname = vpi_get_str(vpiFullName, item);
		  int i;
		  int nskip = vcd_scope_names_test(fullname);

#if 0
		  vpi_printf("LXT2 info: scanning scope %s, %u levels\n",
		             fullname, depth);
#endif

		  if (nskip) {
			vpi_printf("LXT2 warning: ignoring signals in "
			           "previously scanned scope %s\n", fullname);
		  } else {
			vcd_scope_names_add(fullname);
		  }

		  name = vpi_get_str(vpiName, item);

                  push_scope(name);

		  for (i=0; dumpable_types[i]>0; i++) {
			vpiHandle hand;
			vpiHandle argv = vpi_iterate(dumpable_types[i], item);
			while (argv && (hand = vpi_scan(argv))) {
			      scan_item(depth-1, hand, nskip);
			}
		  }

                  pop_scope();
	    }
	    break;

	  case vpiPackage:
	      // Don't print a warning for empty packages.
	    if (vcd_instance_contains_dumpable_items(dumpable_types, item))
		  vpi_printf("LXT2 warning: $dumpvars: Package (%s) is not dumpable "
			     "with LXT2.\n", vpi_get_str(vpiFullName, item));
	    break;

	  default:
	    vpi_printf("LXT2 warning: $dumpvars: Unsupported parameter "
	               "type (%s).\n", vpi_get_str(vpiType, item));
      }

}

static int draw_scope(vpiHandle item)
{
      int depth;
      const char *name;

      vpiHandle scope = vpi_handle(vpiScope, item);
      if (!scope) return 0;

      depth = 1 + draw_scope(scope);
      name = vpi_get_str(vpiName, scope);

      push_scope(name);

      return depth;
}

static PLI_INT32 sys_dumpvars_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item;
      s_vpi_value value;
      unsigned depth = 0;

      (void)name; /* Parameter is not used. */

      if (dump_file == 0) {
	    open_dumpfile(callh);
	    if (dump_file == 0) {
		  if (argv) vpi_free_object(argv);
		  return 0;
	    }
      }

      if (install_dumpvars_callback()) {
	    if (argv) vpi_free_object(argv);
	    return 0;
      }

        /* Get the depth if it exists. */
      if (argv) {
	    value.format = vpiIntVal;
	    vpi_get_value(vpi_scan(argv), &value);
	    depth = value.value.integer;
      }
      if (!depth) depth = 10000;

        /* This dumps all the instances in the design if none are given. */
      if (!argv || !(item = vpi_scan(argv))) {
	    argv = vpi_iterate(vpiInstance, 0x0);
	    assert(argv);  /* There must be at least one top level instance. */
	    item = vpi_scan(argv);
      }

      for ( ; item; item = vpi_scan(argv)) {

	    int dep = draw_scope(item);

	    scan_item(depth, item, 0);

	    while (dep--) pop_scope();
      }

	/* Most effective compression. */
      if (lxm_optimum_mode == LXM_SPACE) {
	    lxt2_wr_set_compression_depth(dump_file, 9);
	    lxt2_wr_set_partial_off(dump_file);
      }

      return 0;
}

static void* lxt2_thread(void*arg)
{
	/* Keep track of the current time, and only call the set_time
	   function when the time changes. */
      uint64_t cur_time = 0;
      int run_flag = 1;

      (void)arg; /* Parameter is not used. */

      while (run_flag) {
	    struct vcd_work_item_s*cell = vcd_work_thread_peek();

	    if (cell->time != cur_time) {
		  cur_time = cell->time;
		  lxt2_wr_set_time64(dump_file, cur_time);
	    }

	    switch (cell->type) {
		case WT_NONE:
		  break;
		case WT_FLUSH:
		  lxt2_wr_flush(dump_file);
		  break;
		case WT_DUMPON:
		  lxt2_wr_set_dumpon(dump_file);
		  break;
		case WT_DUMPOFF:
		  lxt2_wr_set_dumpoff(dump_file);
		  break;
		case WT_EMIT_DOUBLE:
		  lxt2_wr_emit_value_double(dump_file, cell->sym_.lxt2,
					    0, cell->op_.val_double);
		  break;
		case WT_EMIT_BITS:
		  lxt2_wr_emit_value_bit_string(dump_file, cell->sym_.lxt2,
						0, cell->op_.val_char);
		  break;
		case WT_TERMINATE:
		  run_flag = 0;
		  break;
	    }

	    vcd_work_thread_pop();
      }

      return 0;
}

void sys_lxt2_register(void)
{
      int idx;
      struct t_vpi_vlog_info vlog_info;
      s_vpi_systf_data tf_data;
      vpiHandle res;

	/* Scan the extended arguments, looking for lxt2 optimization flags. */
      vpi_get_vlog_info(&vlog_info);

	/* The "speed" option is not used in this dumper. */
      for (idx = 0 ;  idx < vlog_info.argc ;  idx += 1) {
	    if (strcmp(vlog_info.argv[idx],"-lxt2-space") == 0) {
		  lxm_optimum_mode = LXM_SPACE;

	    } else if (strcmp(vlog_info.argv[idx],"-lxt2-speed") == 0) {
		  lxm_optimum_mode = LXM_SPEED;

	    } else if (strcmp(vlog_info.argv[idx],"-lx2-space") == 0) {
		  lxm_optimum_mode = LXM_SPACE;

	    } else if (strcmp(vlog_info.argv[idx],"-lx2-speed") == 0) {
		  lxm_optimum_mode = LXM_SPEED;

	    }
      }

      /* All the compiletf routines are located in vcd_priv.c. */

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpall";
      tf_data.calltf    = sys_dumpall_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpall";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpfile";
      tf_data.calltf    = sys_dumpfile_calltf;
      tf_data.compiletf = sys_one_string_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpfile";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpflush";
      tf_data.calltf    = sys_dumpflush_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpflush";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumplimit";
      tf_data.calltf    = sys_dumplimit_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumplimit";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpoff";
      tf_data.calltf    = sys_dumpoff_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpoff";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpon";
      tf_data.calltf    = sys_dumpon_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpon";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpvars";
      tf_data.calltf    = sys_dumpvars_calltf;
      tf_data.compiletf = sys_dumpvars_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpvars";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
