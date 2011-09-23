#ifndef __vpi_priv_H
#define __vpi_priv_H
/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
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

# include  "vpi_user.h"
# include  "pointers.h"
# include  "vvp_net.h"
# include  "config.h"

/*
 * Added to use some "vvp_fun_modpath_src"
 * and "vvp_fun_modpath" classes definitions
 */
#include  "delay.h"


/*
 * This header file contains the internal definitions that the vvp
 * program uses to implement the public interface in the vpi_user.h
 * header file elsewhere.
 */

/*
 * Private VPI properties that are only used in the cleanup code.
 */
#ifdef CHECK_WITH_VALGRIND
#define _vpiFromThr 0x1000001
#   define _vpiNoThr  0
#   define _vpiVThr   1
#   define _vpiWord   2
#   define _vpi_at_PV 3
#   define _vpi_at_A  4
#endif


/*
 * The vpi_mode_flag contains the major mode for VPI use. This is used
 * to generate error messages when vpi functions are called
 * incorrectly.
 */
enum vpi_mode_t {
      VPI_MODE_NONE =0,
	/* The compiler is calling a register function. */
      VPI_MODE_REGISTER,
	/* The compiler is calling a compiletf function. */
      VPI_MODE_COMPILETF,
	/* The compiler is calling a calltf function. */
      VPI_MODE_CALLTF,
	/* We are in the midst of a RWSync callback. */
      VPI_MODE_RWSYNC,
	/* We are in a ROSync callback. */
      VPI_MODE_ROSYNC
};
extern vpi_mode_t vpi_mode_flag;

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
      vpiHandle (*vpi_put_value_)(vpiHandle, p_vpi_value, int flags);

	/* These methods follow references. */
      vpiHandle (*handle_)(int, vpiHandle);
      vpiHandle (*iterate_)(int, vpiHandle);
      vpiHandle (*index_)(vpiHandle, int);

	/* This implements the vpi_free_object method. */
      int (*vpi_free_object_)(vpiHandle);

       /*
	 These two methods are used to read/write delay
	 values from/into modpath records
       */
      void  (*vpi_get_delays_)(vpiHandle, p_vpi_delay);
      void  (*vpi_put_delays_)(vpiHandle, p_vpi_delay);
};

/*
 * In general a vpi object is a structure that contains the member
 * "base" that is a __vpiHandle object. This template can convert any
 * of those structures into a vpiHandle object.
 */
template <class T> vpiHandle vpi_handle(T obj)
{ return &obj->base; }

/*
 * The vpiHandle for an iterator has this structure. The definition of
 * the methods lives in vpi_iter.c
 *
 * The args and nargs members point to the array of vpiHandle objects
 * that are to be iterated over. The next member is the index of the
 * next item to be returned by a vpi_scan.
 *
 * The free_args_flag member is true if when this iterator object is
 * released it must also free the args array.
 */
struct __vpiIterator {
      struct __vpiHandle base;
      vpiHandle *args;
      unsigned  nargs;
      unsigned  next;
      bool free_args_flag;
};

extern vpiHandle vpip_make_iterator(unsigned nargs, vpiHandle*args,
				    bool free_args_flag);

/*
 * This represents callback handles. There are some private types that
 * are defined and used in vpi_callback.cc.
 */
struct __vpiCallback {
      struct __vpiHandle base;

	// user supplied callback data
      struct t_cb_data cb_data;
      struct t_vpi_time cb_time;
      struct t_vpi_value cb_value;

	// scheduled event
      struct sync_cb* cb_sync;

	// The callback holder may use this for various purposes.
      long extra_data;

	// Used for listing callbacks.
      struct __vpiCallback*next;
};

extern struct __vpiCallback* new_vpi_callback();
extern void delete_vpi_callback(struct __vpiCallback* ref);
extern void callback_execute(struct __vpiCallback*cur);

struct __vpiSystemTime {
      struct __vpiHandle base;
      struct __vpiScope *scope;
};

/*
 * Scopes are created by .scope statements in the source. These
 * objects hold the items and properties that are knowingly bound to a
 * scope.
 */
struct __vpiScope {
      struct __vpiHandle base;
      struct __vpiScope *scope;
	/* The scope has a name. */
      const char*name;
      const char*tname;
      unsigned file_idx;
      unsigned lineno;
      unsigned def_file_idx;
      unsigned def_lineno;
      bool is_automatic;
	/* The scope has a system time of its own. */
      struct __vpiSystemTime scoped_time;
      struct __vpiSystemTime scoped_stime;
      struct __vpiSystemTime scoped_realtime;
	/* Keep an array of internal scope items. */
      struct __vpiHandle**intern;
      unsigned nintern;
        /* Keep an array of items to be automatically allocated */
      struct automatic_hooks_s**item;
      unsigned nitem;
        /* Keep a list of live contexts. */
      vvp_context_t live_contexts;
        /* Keep a list of freed contexts. */
      vvp_context_t free_contexts;
	/* Keep a list of threads in the scope. */
      vthread_t threads;
      signed int time_units :8;
      signed int time_precision :8;
};

extern struct __vpiScope* vpip_peek_current_scope(void);
extern void vpip_attach_to_current_scope(vpiHandle obj);
extern struct __vpiScope* vpip_peek_context_scope(void);
extern unsigned vpip_add_item_to_context(automatic_hooks_s*item,
                                         struct __vpiScope*scope);
extern vpiHandle vpip_make_root_iterator(void);
extern void vpip_make_root_iterator(struct __vpiHandle**&table,
				    unsigned&ntable);

/*
 * Signals include the variable types (reg, integer, time) and are
 * distinguished by the vpiType code. They also have a parent scope,
 * a declared name and declaration indices.
 */
struct __vpiSignal {
      struct __vpiHandle base;
#ifdef CHECK_WITH_VALGRIND
      struct __vpiSignal *pool;
#endif
      union { // The scope or parent array that contains me.
	    vpiHandle parent;
	    struct __vpiScope* scope;
      } within;
      union { // The name of this reg/net, or the index for array words.
            const char*name;
            vpiHandle index;
      } id;
	/* The indices that define the width and access offset. */
      int msb, lsb;
	/* Flags */
      unsigned signed_flag  : 1;
      unsigned isint_       : 1; // original type was integer
      unsigned is_netarray  : 1; // This is word of a net array
	/* The represented value is here. */
      vvp_net_t*node;
};
extern unsigned vpip_size(__vpiSignal *sig);
extern struct __vpiScope* vpip_scope(__vpiSignal*sig);

extern vpiHandle vpip_make_int(const char*name, int msb, int lsb,
			       vvp_net_t*vec);
extern vpiHandle vpip_make_reg(const char*name, int msb, int lsb,
			       bool signed_flag, vvp_net_t*net);
extern vpiHandle vpip_make_net(const char*name, int msb, int lsb,
			       bool signed_flag, vvp_net_t*node);

/*
 * This is used by system calls to represent a bit/part select of
 * a simple variable or constant array word.
 */
struct __vpiPV {
      struct __vpiHandle base;
      vpiHandle parent;
      vvp_net_t*net;
      vpiHandle sbase;
      int tbase;
      unsigned twid, width;
};
extern vpiHandle vpip_make_PV(char*name, int base, int width);
extern vpiHandle vpip_make_PV(char*name, char*symbol, int width);
extern vpiHandle vpip_make_PV(char*name, vpiHandle handle, int width);
extern vpiHandle vpip_make_PV(char*name, int tbase, int twid, int width);

extern struct __vpiPV* vpip_PV_from_handle(vpiHandle obj);
extern void vpip_part_select_value_change(struct __vpiCallback*cbh, vpiHandle obj);


/*
 * This function safely converts a vpiHandle back to a
 * __vpiSignal. Return a nil if the type is not appropriate.
 */
extern __vpiSignal* vpip_signal_from_handle(vpiHandle obj);


struct __vpiModPathTerm {
      struct __vpiHandle base;
      vpiHandle expr;
	/* The value returned by vpi_get(vpiEdge, ...); */
      int edge;
};

struct __vpiModPathSrc {
      struct __vpiHandle   base;
      struct __vpiModPath *dest;
      int   type;

	/* This is the input expression for this modpath. */
      struct __vpiModPathTerm path_term_in;

	/* This is the input net for the modpath. signals on this net
	   are used to determine the modpath. They are *not* propagated
	   anywhere. */
      vvp_net_t *net;
} ;


/*
 *
 * The vpiMoaPath vpiHandle will define
 * a vpiModPath of record .modpath as defined
 * in the IEEE 1364
 *
 */

struct __vpiModPath {
      struct __vpiScope   *scope ;

      class vvp_fun_modpath*modpath;

      struct __vpiModPathTerm path_term_out;
      vvp_net_t *input_net  ;
};

extern struct __vpiModPathTerm* vpip_modpath_term_from_handle(vpiHandle ref);
extern struct __vpiModPathSrc* vpip_modpath_src_from_handle(vpiHandle ref);


/*
 * The Function is used to create the vpiHandle
 * for vpiModPath && vpiModPathIn objects
 */

extern struct __vpiModPathSrc* vpip_make_modpath_src  (struct __vpiModPath*path_dest,
					 vvp_time64_t use_delay[12] ,
					 vvp_net_t *net ) ;

extern struct __vpiModPath* vpip_make_modpath(vvp_net_t *net) ;


/*
 * These methods support the vpi creation of events. The name string
 * passed in will be saved, so the caller must allocate it (or not
 * free it) after it is handed to this function.
 */
struct __vpiNamedEvent {
      struct __vpiHandle base;
	/* base name of the event object */
      const char*name;
	/* Parent scope of this object. */
      struct __vpiScope*scope;
	/* The functor, used for %set operations. */
      vvp_net_t*funct;
	/* List of callbacks interested in this event. */
      struct __vpiCallback*callbacks;
};

extern vpiHandle vpip_make_named_event(const char*name, vvp_net_t*f);
extern void vpip_run_named_event_callbacks(vpiHandle ref);
extern void vpip_real_value_change(struct __vpiCallback*cbh,
				   vpiHandle ref);

/*
 * Memory is an array of bits that is accessible in N-bit chunks, with
 * N being the width of a word. The memory word handle just points
 * back to the memory and uses an index to identify its position in
 * the memory.
 */

extern bool is_net_array(vpiHandle obj);

/*
 * These are the various variable types.
 */
struct __vpiRealVar {
      struct __vpiHandle base;
      union { // The scope or parent array that contains me.
	    vpiHandle parent;
	    struct __vpiScope* scope;
      } within;
	/* The name of this variable, or the index for array words. */
      union {
            const char*name;
            vpiHandle index;
      } id;
      unsigned is_netarray  : 1; // This is word of a net array
      vvp_net_t*net;
};

extern struct __vpiScope* vpip_scope(__vpiRealVar*sig);
extern vpiHandle vpip_make_real_var(const char*name, vvp_net_t*net);
extern struct __vpiRealVar* vpip_realvar_from_handle(vpiHandle obj);

/*
 * When a loaded VPI module announces a system task/function, one
 * __vpiUserSystf object is created to hold the definition of that
 * task/function. The distinction between task and function is stored
 * in the vpi_systf_data structure data that was supplied by the
 * external module.
 *
 * When the compiler encounters a %vpi_call statement, it creates a
 * __vpiSysTaskCall to represent that particular call. The call refers
 * to the definition handle so that when the %vpi_call instruction is
 * encountered at run-time, the definition can be located and used.
 *
 * The vpiSysTaskCall handles both functions and tasks, as the two are
 * extremely similar. The different VPI type is reflected in a
 * different vpi_type pointer in the base structure. The only
 * additional part is the vbit/vwid that is used by the put of the
 * system function call to place the values in the vthread bit space.
 */
struct __vpiUserSystf {
      struct __vpiHandle base;
      s_vpi_systf_data info;
};

extern struct __vpiUserSystf* vpip_find_systf(const char*name);


struct __vpiSysTaskCall {
      struct __vpiHandle base;
      struct __vpiScope* scope;
      struct __vpiUserSystf*defn;
      unsigned nargs;
      vpiHandle*args;
	/* Support for vpi_get_userdata. */
      void*userdata;
	/* These represent where in the vthread to put the return value. */
      unsigned vbit;
      signed   vwid;
      struct   vvp_net_t*fnet;
      unsigned file_idx;
      unsigned lineno;
      bool put_value;
};

extern struct __vpiSysTaskCall*vpip_cur_task;

/*
 * These are implemented in vpi_const.cc. These are vpiHandles for
 * constants.
 *
 * The persistent flag to vpip_make_string_const causes the created
 * handle to be persistent. This is necessary for cases where the
 * string handle may be reused, which is the normal case.
 */
struct __vpiStringConst {
      struct __vpiHandle base;
      char*value;
      size_t value_len;
};

vpiHandle vpip_make_string_const(char*text, bool persistent =true);
vpiHandle vpip_make_string_param(char*name, char*value,
                                 long file_idx, long lineno);

struct __vpiBinaryConst {
      struct __vpiHandle base;
      vvp_vector4_t bits;
	/* TRUE if this constant is signed. */
      int signed_flag :1;
	/* TRUE if this constant has an explicit size (i.e. 19'h0 vs. 'h0) */
      int sized_flag   :1;
};

vpiHandle vpip_make_binary_const(unsigned wid, const char*bits);
vpiHandle vpip_make_binary_param(char*name, const vvp_vector4_t&bits,
				 bool signed_flag,
				 long file_idx, long lineno);

struct __vpiDecConst {
      struct __vpiHandle base;
      int value;
};

vpiHandle vpip_make_dec_const(int value);
vpiHandle vpip_make_dec_const(struct __vpiDecConst*obj, int value);

struct __vpiRealConst {
      struct __vpiHandle base;
      double value;
};

vpiHandle vpip_make_real_const(double value);
vpiHandle vpip_make_real_param(char*name, double value,
                               long file_idx, long lineno);

/*
 *  This one looks like a constant, but really is a vector in the current
 *  thread.
 */

vpiHandle vpip_make_vthr_vector(unsigned base, unsigned wid, bool signed_flag);

vpiHandle vpip_make_vthr_word(unsigned base, const char*type);

vpiHandle vpip_make_vthr_A(char*label, unsigned index);
vpiHandle vpip_make_vthr_A(char*label, char*symbol);
vpiHandle vpip_make_vthr_A(char*label, unsigned tbase, unsigned twid);
vpiHandle vpip_make_vthr_A(char*label, vpiHandle handle);

/*
 * This function is called before any compilation to load VPI
 * modules. This gives the modules a chance to announce their
 * contained functions before compilation commences. It is called only
 * once per module.
 */
extern void vpip_load_module(const char*name);

# define VPIP_MODULE_PATH_MAX 64
extern const char* vpip_module_path[64];
extern unsigned vpip_module_path_cnt;

/*
 * The vpip_build_vpi_call function creates a __vpiSysTaskCall object
 * and returns the handle. The compiler uses this function when it
 * encounters a %vpi_call or %vpi_func statement.
 *
 * The %vpi_call instruction has as its only parameter the handle that
 * is returned by the vpip_build_vpi_call. This includes all the
 * information needed by vpip_execute_vpi_call to actually execute the
 * call. However, the vpiSysTaskCall that is the returned handle,
 * holds a parameter argument list that is passed in here.
 *
 * The vbit and vwid fields are used if this turns out to be a system
 * function. In that case, the vbit and vwid are used to address the
 * vector in thread bit space where the result is supposed to go.
 *
 * Note that the argv array is saved in the handle, and should should
 * not be released by the caller.
 */
extern vpiHandle vpip_build_vpi_call(const char*name,
				     unsigned vbit, int vwid,
				     struct vvp_net_t*fnet,
				     unsigned argc,
				     vpiHandle*argv,
				     long file_idx,
				     long lineno);

extern vthread_t vpip_current_vthread;

extern void vpip_execute_vpi_call(vthread_t thr, vpiHandle obj);


/*
 * These are functions used by the compiler to prepare for compilation
 * and to finish compilation in preparation for execution.
 */

vpiHandle vpip_sim_time(struct __vpiScope*scope, bool is_stime);
vpiHandle vpip_sim_realtime(struct __vpiScope*scope);

extern int vpip_get_time_precision(void);
extern void vpip_set_time_precision(int pres);

extern int vpip_time_units_from_handle(vpiHandle obj);
extern int vpip_time_precision_from_handle(vpiHandle obj);

extern void vpip_time_to_timestruct(struct t_vpi_time*ts, vvp_time64_t ti);
extern vvp_time64_t vpip_timestruct_to_time(const struct t_vpi_time*ts);

extern double vpip_time_to_scaled_real(vvp_time64_t ti, struct __vpiScope*sc);
extern vvp_time64_t vpip_scaled_real_to_time64(double val, struct __vpiScope*sc);

/*
 * These functions are used mostly as compile time to strings into
 * permallocated memory. The vpip_string function is the most general,
 * it allocates a fresh string no matter what. The vpip_name_string
 * allocates a string and keeps a pointer in the hash, and tries to
 * reuse it if it can. This us useful for handle names, which may be
 * reused in different scopes.
 */
extern const char* vpip_string(const char*str);
extern const char* vpip_name_string(const char*str);


/*
 * This function is used to make decimal string versions of various
 * vectors. The input format is a vvp_vector4_t, and the result is
 * written into buf, without overflowing nbuf.
 */
extern unsigned vpip_vec4_to_dec_str(const vvp_vector4_t&vec4,
				     char *buf, unsigned int nbuf,
				     int signed_flag);


extern void vpip_vec4_to_hex_str(const vvp_vector4_t&bits, char*buf,
				 unsigned nbuf, bool signed_flag);

extern void vpip_vec4_to_oct_str(const vvp_vector4_t&bits, char*buf,
				 unsigned nbuf, bool signed_flag);

extern void vpip_bin_str_to_vec4(vvp_vector4_t&val, const char*buf);
extern void vpip_oct_str_to_vec4(vvp_vector4_t&val, const char*str);
extern void vpip_dec_str_to_vec4(vvp_vector4_t&val, const char*str);
extern void vpip_hex_str_to_vec4(vvp_vector4_t&val, const char*str);

extern vvp_vector4_t vec4_from_vpi_value(s_vpi_value*vp, unsigned wid);
extern double real_from_vpi_value(s_vpi_value*vp);

extern void vpip_vec4_get_value(const vvp_vector4_t&word_val, unsigned width,
				bool signed_flag, s_vpi_value*vp);
extern void vpip_real_get_value(double real, s_vpi_value*vp);

/*
 * Function defined in vpi_signal.cc to manage vpi_get_* persistent
 * storage.
 */
enum vpi_rbuf_t {
      RBUF_VAL =0,
	/* Storage for *_get_value() */
      RBUF_STR,
	/* Storage for *_get_str() */
      RBUF_DEL
	/* Delete the storage for both buffers. */
};
extern char *need_result_buf(unsigned cnt, vpi_rbuf_t type);
/* following two routines use need_result_buf(, RBUF_STR) */
extern char *simple_set_rbuf_str(const char *s1);
extern char *generic_get_str(int code, vpiHandle ref, const char *name, const char *index);

/* A routine to find the enclosing module. */
extern vpiHandle vpip_module(struct __vpiScope*scope);

#endif
