#ifndef __vpi_priv_H
#define __vpi_priv_H
/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vpi_priv.h,v 1.11 2000/02/23 02:56:56 steve Exp $"
#endif

/*
 * This header file describes the "back side" of the VPI
 * interface. The product that offers the VPI interface uses types and
 * methods declared here to manage the VPI structures and provide the
 * needed behaviors.
 */
# include  <vpi_user.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __vpirt;
enum vpip_bit_t { V0 = 0, V1, Vx, Vz };

extern void vpip_bits_get_value(enum vpip_bit_t*bits, unsigned nbits,
				s_vpi_value*vp);

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
 * This is a private handle type that doesn't seem to be well defined
 * by the VPI standard.
 */
struct __vpiCallback {
      struct __vpiHandle base;
      struct t_cb_data cb_data;

      struct vpip_event*ev;
      struct __vpiCallback*next;
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

/*
 * Memory is an array of bits that is accessible in N-bit chunks, with
 * N being the width of a word. The memory word handle just points
 * back to the memory and uses an index to identify its position in
 * the memory.
 */
struct __vpiMemory {
      struct __vpiHandle base;
	/* The signal has a name (this points to static memory.) */
      const char*name;
      enum vpip_bit_t*bits;
      struct __vpiMemoryWord*words;
      vpiHandle*args;
      unsigned width;
      unsigned size;
};

struct __vpiMemoryWord {
      struct __vpiHandle base;
      struct __vpiMemory*mem;
      int index;
};

/*
 * This type is occasionally useful. Really! And while we're at it,
 * create a single instance of the null object. (This is all we need.)
 */
struct __vpiNull {
      struct __vpiHandle base;
};
extern struct __vpiNull vpip_null;

/*
 * This type represents the handle to a Verilog scope. These include
 * module instantiations and name begin-end blocks. The attach
 * function is used to attach handles to the scope by the runtime
 * initializaiton. 
 */
struct __vpiScope {
      struct __vpiHandle base;
	/* The scope has a name. (this points to static memory.) */
      const char*name;
	/* Keep an array of internal scope items. */
      struct __vpiHandle**intern;
      unsigned nintern;
};
extern void vpip_attach_to_scope(struct __vpiScope*scope, vpiHandle obj);


/*
 * This structure represents nets and registers. You can tell which by
 * the type_code in the base. The bits member points to the actual
 * array of bits that the environment provides. The bits must persist
 * as long as this object persists.
 */
struct __vpiSignal {
      struct __vpiHandle base;
	/* The signal has a name (this points to static memory.) */
      const char*name;
	/* The signal has a value and dimension. */
      enum vpip_bit_t*bits;
      unsigned nbits;
	/* monitors are added here. */
      struct __vpiCallback*monitor;
};


extern const struct __vpirt vpip_systask_rt;
struct __vpiSysTaskCall {
      struct __vpiHandle base;

      s_vpi_systf_data*info;
      vpiHandle*args;
      unsigned nargs;

      const char*file;
      unsigned lineno;
      int subtype;
};

/*
 * Represent a TimeVar variable. The actual time is stored in the
 * "time" member for fast manipulation by various bits of the
 * simulation engine. The time_obj member is used as persistent
 * storage of the time value when get_value is used (on the opaque
 * handle) to the get time.
 */
struct __vpiTimeVar {
      struct __vpiHandle base;

      const char*name;
      unsigned long time;
      struct t_vpi_time time_obj;
};


struct __vpiStringConst {
      struct __vpiHandle base;

      const char*value;
};

struct __vpiNumberConst {
      struct __vpiHandle base;

      enum vpip_bit_t*bits;
      unsigned nbits;
};

/*
 * These are methods to initialize specific handle types. Except for
 * vpip_make_iterator, all the vpi_make_* functions expect the caller
 * to allocate the memory for the handle. The result is the vpiHandle
 * of the constructed object.
 */
extern vpiHandle vpip_make_iterator(unsigned nargs, vpiHandle*args);
extern vpiHandle vpip_make_net(struct __vpiSignal*ref, const char*name);
extern vpiHandle vpip_make_scope(struct __vpiScope*ref,
				 int type_code,
				 const char*name);
extern vpiHandle vpip_make_string_const(struct __vpiStringConst*ref,
					const char*val);
extern vpiHandle vpip_make_number_const(struct __vpiNumberConst*ref,
					const enum vpip_bit_t*bits,
					unsigned nbits);
extern vpiHandle vpip_make_memory(struct __vpiMemory*ref, const char*name,
				  unsigned width, unsigned size);
extern vpiHandle vpip_make_reg(struct __vpiSignal*ref, const char*name);
extern vpiHandle vpip_make_time_var(struct __vpiTimeVar*ref,
				    const char*val);

/* Use this function to call a registered task. */
extern void vpip_calltask(const char*name, unsigned nparms, vpiHandle*parms);

extern void vpip_run_value_changes(struct __vpiSignal*sig);

/*
 * The simulation object holds the current state of the
 * simulation. There is a single global variable that is the
 * simulation.
 */

struct vpip_simulation_cycle;
struct vpip_event;
struct vpip_simulation {
	/* Current simulation time. */
      struct __vpiTimeVar sim_time;

	/* List of cbReadOnlySynch callbacks. */
      struct __vpiCallback*read_sync_list;

	/* List of simulation cycles, starting with the next time. */
      struct vpip_simulation_cycle*sim;
      int going_flag;
};

extern struct vpip_simulation vpip_simulation_obj;

extern void vpip_init_simulation();
extern void vpip_simulation_run();

/*
 * Schedule an event to be run sometime in the future. The d parmater
 * is the delay in simulation units before the event is processed. If
 * the non-block flag is set, the event is scheduled to happen at the
 * end of the time step.
 *
 * The return value from the insert method is a cookie that can be
 * used to manipulate the event before it is executed.
 */
extern struct vpip_event* vpip_sim_insert_event(unsigned long d,
						void*user_data,
						void (*sim_fun)(void*),
						int nonblock_flag);
extern void vpip_sim_cancel_event(struct vpip_event*cookie);

/*
 * This function returns a handle to the vpiTimeVar that is th main
 * simulation time clock.
 */
extern vpiHandle vpip_sim_time();

/*
 * Return true if the going_flag is false.
 */
extern int vpip_finished();

#ifdef __cplusplus
}
#endif

/*
 * $Log: vpi_priv.h,v $
 * Revision 1.11  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.10  2000/02/13 19:18:28  steve
 *  Accept memory words as parameter to $display.
 *
 * Revision 1.9  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 * Revision 1.8  1999/11/28 00:56:08  steve
 *  Build up the lists in the scope of a module,
 *  and get $dumpvars to scan the scope for items.
 *
 * Revision 1.7  1999/11/27 19:07:58  steve
 *  Support the creation of scopes.
 *
 * Revision 1.6  1999/11/10 02:52:24  steve
 *  Create the vpiMemory handle type.
 *
 * Revision 1.5  1999/11/06 16:52:16  steve
 *  complete value retrieval for number constants.
 *
 * Revision 1.4  1999/11/06 16:00:18  steve
 *  Put number constants into a static table.
 *
 * Revision 1.3  1999/10/29 03:37:22  steve
 *  Support vpiValueChance callbacks.
 *
 * Revision 1.2  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 * Revision 1.1  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 */
#endif
