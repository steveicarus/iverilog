#ifndef __vpi_priv_H
#define __vpi_priv_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: vpi_priv.h,v 1.4 2001/03/20 06:16:24 steve Exp $"
#endif

# include  "vpi_user.h"
# include  "pointers.h"

/*
 * This header file contains the internal definitions that the vvp
 * program uses to implement the public interface in the vpi_user.h
 * header file elsewhere.
 */



/*
 * This structure is the very base of a vpiHandle. Every handle
 * structure starts with this structure, so that the library can
 * internally pass the derived types as pointers to one of these.
 */
struct __vpiHandle {
      const struct __vpirt *vpi_type;
};

/*
 * Objects with this structure are used to represent a type of
 * vpiHandle. A specific object becomes of this type by holding a
 * pointer to an instance of this structure.
 */
struct __vpirt {
      int type_code;

	/* These methods extract information from the handle. */
      int   (*vpi_get_)(int, vpiHandle);
      char* (*vpi_get_str_)(int, vpiHandle);
      void  (*vpi_get_value_)(vpiHandle, p_vpi_value);
      vpiHandle (*vpi_put_value_)(vpiHandle, p_vpi_value, p_vpi_time, int);

	/* These methods follow references. */
      vpiHandle (*handle_)(int, vpiHandle);
      vpiHandle (*iterate_)(int, vpiHandle);
      vpiHandle (*index_)(vpiHandle, int);
};

/*
 * The vpiHandle for an iterator has this structure. The definition of
 * the methods lives in vpi_iter.c
 */
struct __vpiIterator {
      struct __vpiHandle base;
      vpiHandle *args;
      unsigned  nargs;
      unsigned  next;
};

extern vpiHandle vpip_make_iterator(unsigned nargs, vpiHandle*args);

/*
 * Scopes are created by .scope statements in the source.
 */
struct __vpiScope {
      struct __vpiHandle base;
	/* The scope has a name. */
      char*name;
      /* Keep an array of internal scope items. */
      struct __vpiHandle**intern;
      unsigned nintern;
};
extern vpiHandle vpip_peek_current_scope(void);
extern void vpip_attach_to_current_scope(vpiHandle obj);

/*
 * Signals include the variable types (reg, integer, time) and are
 * distinguished by the vpiType code. They also have a parent scope,
 * a declared name and declaration indices.
 */
struct __vpiSignal {
      struct __vpiHandle base;
      vpiHandle scope;
	/* The name of this reg/net object */
      char*name;
	/* The indices that define the width and access offset. */
      int msb, lsb;
	/* The represented value is here. */
      vvp_ipoint_t bits;
};
extern vpiHandle vpip_make_reg(char*name, int msb, int lsb,
			       vvp_ipoint_t base);

/*
 * When a loaded VPI module announces a system task/function, one
 * __vpiUserSystf object is created to hold the definition of that
 * task/function.
 *
 * When the compiler encounters a %vpi_call statement, it creates a
 * __vpiSysTaskCall to represent that particular call. The call refers
 * to the definition handle so that when the %vpi_call instruction is
 * encountered at run-time, the definition can be located and used.
 */
struct __vpiUserSystf {
      struct __vpiHandle base;
      s_vpi_systf_data info;
};

struct __vpiSysTaskCall {
      struct __vpiHandle base;
      vpiHandle scope;
      struct __vpiUserSystf*defn;
      unsigned nargs;
      vpiHandle*args;
};

extern struct __vpiSysTaskCall*vpip_cur_task;

/*
 * These are implemented in vpi_const.cc. These are vpiHandles for
 * constants.
 */
struct __vpiStringConst {
      struct __vpiHandle base;
      const char*value;
};

vpiHandle vpip_make_string_const(char*text);


/*
 * This function is called before any compilation to load VPI
 * modules. This gives the modules a chance to announce their
 * contained functions before compilation commences. It is called only
 * once. 
 */
extern void vpip_load_modules(unsigned cnt, const char*tab[], const char*path);

/*
 * The vpip_build_vpi_call function creates a __vpiSysTaskCall object
 * and returns the handle. The compiler uses this function when it
 * encounters a %vpi_call statement.
 *
 * The %vpi_call instruction has as its only parameter the handle that
 * is returned by the vpip_build_vpi_call. This includes all the
 * information needed by vpip_execute_vpi_call to actually execute the
 * call. However, the vpiSysTaskCall that is the returned handle,
 * holds a parameter argument list that is passed in here.
 *
 * Note that the argv array is saved in the handle, should should not
 * be released by the caller.
 */
extern vpiHandle vpip_build_vpi_call(const char*name,
				     unsigned argc,
				     vpiHandle*argv);

extern void vpip_execute_vpi_call(vpiHandle obj);


/*
 * These are functions used by the compiler to prepare for compilation
 * and to finish compilation in preparation for execution.
 */
extern void scope_init(void);
extern void scope_cleanup(void);

/*
 * $Log: vpi_priv.h,v $
 * Revision 1.4  2001/03/20 06:16:24  steve
 *  Add support for variable vectors.
 *
 * Revision 1.3  2001/03/18 04:35:18  steve
 *  Add support for string constants to VPI.
 *
 * Revision 1.2  2001/03/18 00:37:55  steve
 *  Add support for vpi scopes.
 *
 * Revision 1.1  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 */
#endif
