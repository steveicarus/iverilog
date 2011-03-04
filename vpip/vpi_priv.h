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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_priv.h,v 1.4 2004/10/04 01:10:58 steve Exp $"
#endif

/*
 * This header file describes the "back side" of the VPI
 * interface. The product that offers the VPI interface uses types and
 * methods declared here to manage the VPI structures and provide the
 * needed behaviors.
 */
# include  "vpi_user.h"

#ifdef __cplusplus
extern "C" {
#endif

struct __vpirt;

/*
 * The simulation engine internally carries the strengths along with
 * the bit values, and that includes ambiguous strengths. The complete
 * bit value (including ambiguity) is encoded in 8 bits like so:
 *
 *     VSSSvsss
 *
 * The V and v bits encode the bit logic values, and the SSS and sss
 * bits encode the strength range. The logic values are like so:
 *
 *     0SSS0sss  - Logic 0
 *     1SSS1sss  - Logic 1
 *     1xxx0xxx  - Logic X
 *     00001000  - Logic Z
 *
 *     00000000  - Invalid/No signal
 *
 * So as you can see, logic values can be quickly compared by masking
 * the strength bits.
 *
 * If the value is unambiguous, then the SSS and sss bits have the
 * same value, and encode the strength of the driven value. If the
 * value is logic X, then "unambiguous" in this context means the
 * strength is well known, even though the logic value is
 * not. However, it is treated as ambiguous by the resolver.
 *
 * If the strength is ambiguous, then the high 4 bits are always
 * arithmetically larger then the low 4 bits. For logic 0 and logic 1
 * values, this means that the SSS value is >= the sss value. For
 * logic X values, the 'V' bit is set and SSS is the strength toward 1,
 * and the 'v' bit is 0 and sss is the strength toward 0.
 */
typedef unsigned char vpip_bit_t;

# define Su1 0xff          //supply1
# define St1 0xee          //strong1
# define Pu1 0xdd          //pull1
# define La1 0xcc          //large1
# define We1 0xbb          //weak1
# define Me1 0xaa          //medium1
# define Sm1 0x99          //samll1

# define Su0 0x77          //supply0
# define St0 0x66          //strong0
# define Pu0 0x55          //pull0
# define La0 0x44          //large0
# define We0 0x33          //weak0
# define Me0 0x22          //medium0
# define Sm0 0x11          //small0

# define SuX 0xf7          //supplyx
# define StX 0xe6          //strongx
# define PuX 0xd5          //pullx
# define LaX 0xc4          //largex
# define WeX 0xb3          //weakx
# define MeX 0xa2          //mediumx
# define SmX 0x91          //smallx

# define HiZ  0x08         //highz
# define HiZ0 0x08         //highz
# define HiZ1 0x08         //highz

# define StH 0xe8          //strong 1 highz
# define StL 0x06          //highz  strong0


	/* Compare the logic values of two vpip_bit_t variables. This
	   is like the === operator of Verilog, it ignored strengths. */
# define B_EQ(l,r) (((l)&0x88) == ((r)&0x88))

	/* Test and return true if the value has ambiguous
	   strength. The logic value may yet be knowable. */
# define B_ISAMBIG(v) (((v)&0x0f) != (((v)>>4)&0x0f))

	/* Test whether the value is of the specified logic value. It
	   is possible for even ambiguous signals to have a known
	   logic value. */
# define B_IS0(v)  (((v)&0x88) == 0x00)
# define B_IS1(v)  (((v)&0x88) == 0x88)
# define B_ISX(v)  (((v)&0x88) == 0x80)
# define B_ISZ(v)  ((v) == HiZ)
# define B_ISXZ(v) (1 & (((v)>>7) ^ ((v)>>3)))


      /* Take as input an array of bits, and return the resolved
	 value. The result accounts for the strengths involved. */
extern vpip_bit_t vpip_pair_resolve(vpip_bit_t a, vpip_bit_t b);
extern vpip_bit_t vpip_bits_resolve(const vpip_bit_t*bits, unsigned nbits);


extern void vpip_bits_get_value(const vpip_bit_t*bits, unsigned nbits,
				s_vpi_value*vp, int signed_flag);
extern void vpip_bits_set_value(vpip_bit_t*bits, unsigned nbits,
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
      const char* (*vpi_get_str_)(int, vpiHandle);
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

	/* Set this value if I'm pending in the event queue. */
      struct vpip_event*ev;
	/* Set this value if I'm waiting for a value change on a signal*/
      struct __vpiSignal*sig;

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
      vpip_bit_t*bits;
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

extern struct __vpiNull *vpip_get_null(void);

/*
 * This type represents the handle to a Verilog scope. These include
 * module instantiations and name begin-end blocks. The attach
 * function is used to attach handles to the scope by the runtime
 * initialization.
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
      vpip_bit_t*bits;
      unsigned nbits;

      unsigned signed_flag : 1;

	/* monitors are added here. */
      struct __vpiCallback*mfirst;
      struct __vpiCallback*mlast;
};


extern const struct __vpirt *vpip_get_systask_rt(void);
extern const struct __vpirt *vpip_get_sysfunc_rt(void);

struct __vpiSysTaskCall {
      struct __vpiHandle base;

      struct __vpiScope*scope;

      s_vpi_systf_data*info;
      vpiHandle*args;
      unsigned nargs;

      vpip_bit_t*res;
      unsigned nres;

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

      const vpip_bit_t*bits;
      unsigned nbits;
};

/*
 * These are methods to initialize specific handle types. Except for
 * vpip_make_iterator, all the vpi_make_* functions expect the caller
 * to allocate the memory for the handle. The result is the vpiHandle
 * of the constructed object.
 */
extern vpiHandle vpip_make_iterator(unsigned nargs, vpiHandle*args);
extern vpiHandle vpip_make_net(struct __vpiSignal*ref, const char*name,
			       vpip_bit_t*bits, unsigned nbits,
			       int signed_flag);
extern vpiHandle vpip_make_scope(struct __vpiScope*ref,
				 int type_code,
				 const char*name);
extern vpiHandle vpip_make_string_const(struct __vpiStringConst*ref,
					const char*val);
extern vpiHandle vpip_make_number_const(struct __vpiNumberConst*ref,
					const vpip_bit_t*bits,
					unsigned nbits);
extern vpiHandle vpip_make_memory(struct __vpiMemory*ref, const char*name,
				  unsigned width, unsigned size);
extern vpiHandle vpip_make_reg(struct __vpiSignal*ref, const char*name,
			       vpip_bit_t*bits, unsigned nbits,
			       int signed_flag);
extern vpiHandle vpip_make_time_var(struct __vpiTimeVar*ref,
				    const char*val);

/* Use this function to call a registered task. */
extern void vpip_calltask(struct __vpiScope*scope, const char*name,
			  unsigned nparms, vpiHandle*parms);

/*
 * This calls a system function with a given name. The return value is
 * taken by the res[] array.
 */
extern void vpip_callfunc(const char*name, unsigned nres, vpip_bit_t*res,
			  unsigned nparms, vpiHandle*parms);

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

	/* This is the precision of the simulation clock. It may be
	   used by the run time to scale time values. */
      short time_precision;
};

extern struct vpip_simulation *vpip_get_simulation_obj(void);

extern void vpip_set_vlog_info(int argc, char**argv);
extern void vpip_init_simulation();
extern void vpip_time_scale(int precision);
extern void vpip_simulation_run();
extern void vpi_mcd_init(void);

/*
 * Schedule an event to be run sometime in the future. The d parameter
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
 * Revision 1.4  2004/10/04 01:10:58  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.3  2002/08/12 01:35:06  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/10/26 02:29:10  steve
 *  const/non-const warnings. (Stephan Boettcher)
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.31  2001/01/06 22:22:17  steve
 *  Support signed decimal display of variables.
 *
 * Revision 1.30  2000/11/11 01:52:09  steve
 *  change set for support of nmos, pmos, rnmos, rpmos, notif0, and notif1
 *  change set to correct behavior of bufif0 and bufif1
 *  (Tim Leight)
 *
 *  Also includes fix for PR#27
 *
 * Revision 1.29  2000/10/28 00:51:42  steve
 *  Add scope to threads in vvm, pass that scope
 *  to vpi sysTaskFunc objects, and add vpi calls
 *  to access that information.
 *
 *  $display displays scope in %m (PR#1)
 *
 * Revision 1.28  2000/10/06 23:11:39  steve
 *  Replace data references with function calls. (Venkat)
 *
 * Revision 1.27  2000/10/04 02:37:44  steve
 *  Use .def file instead of _dllexport.
 *
 * Revision 1.26  2000/10/03 16:15:35  steve
 *  Cleanup build of VPI modules under Cygwin. (Venkat)
 *
 * Revision 1.25  2000/09/30 03:20:48  steve
 *  Cygwin port changes from Venkat
 *
 * Revision 1.24  2000/09/08 17:08:10  steve
 *  initialize vlog info.
 *
 * Revision 1.23  2000/08/20 17:49:05  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.22  2000/07/26 03:53:12  steve
 *  Make simulation precision available to VPI.
 *
 * Revision 1.21  2000/05/18 03:27:32  steve
 *  Support writing scalars and vectors to signals.
 *
 * Revision 1.20  2000/05/11 01:37:33  steve
 *  Calculate the X output value from drive0 and drive1
 *
 * Revision 1.19  2000/05/09 21:16:35  steve
 *  Give strengths to logic and bufz devices.
 *
 * Revision 1.18  2000/05/07 18:20:08  steve
 *  Import MCD support from Stephen Tell, and add
 *  system function parameter support to the IVL core.
 *
 * Revision 1.17  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.16  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.15  2000/04/22 04:20:20  steve
 *  Add support for force assignment.
 *
 * Revision 1.14  2000/03/31 07:08:39  steve
 *  allow cancelling of cbValueChange events.
 *
 * Revision 1.13  2000/03/25 05:02:24  steve
 *  signal bits are referenced at run time by the vpiSignal struct.
 *
 * Revision 1.12  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
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
