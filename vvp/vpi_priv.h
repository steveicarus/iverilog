#ifndef __vpi_priv_H
#define __vpi_priv_H
/*
 * Copyright (c) 2001-2012 Stephen Williams (steve@icarus.com)
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

# include  "sv_vpi_user.h"
# include  "vvp_net.h"
# include  "config.h"

# include  <set>

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
 * Routines/definitions used to build the file/line number tracing object.
 */
#define _vpiFileLine    0x1000003
#define _vpiDescription 0x1000004

extern bool show_file_line;
extern bool code_is_instrumented;

extern vpiHandle vpip_build_file_line(char*description,
                                      long file_idx, long lineno);

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
 * structure is derived from this class so that the library can
 * internally pass the derived types as pointers to one of these.
 */
class __vpiHandle {
    public:
      inline __vpiHandle() { }
	// The destructor is virtual so that dynamic types will work.
      virtual ~__vpiHandle();

      virtual int get_type_code(void) const =0;
      virtual int vpi_get(int code);
      virtual char* vpi_get_str(int code);

      virtual void vpi_get_value(p_vpi_value val);
      virtual vpiHandle vpi_put_value(p_vpi_value val, int flags);
      virtual vpiHandle vpi_handle(int code);
      virtual vpiHandle vpi_iterate(int code);
      virtual vpiHandle vpi_index(int idx);
      virtual void vpi_get_delays(p_vpi_delay del);
      virtual void vpi_put_delays(p_vpi_delay del);

	// Objects may have destroyer functions of their own. If so,
	// then this virtual method will return a POINTER to that
	// function. The pointer is used to "delete" the object, which
	// is why the function itself cannot be a method.
      typedef int (*free_object_fun_t)(vpiHandle);
      virtual free_object_fun_t free_object_fun(void);
};


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
struct __vpiIterator : public __vpiHandle {
      __vpiIterator();
      int get_type_code(void) const;
      free_object_fun_t free_object_fun(void);

      vpiHandle *args;
      unsigned  nargs;
      unsigned  next;
      bool free_args_flag;
};

extern vpiHandle vpip_make_iterator(unsigned nargs, vpiHandle*args,
				    bool free_args_flag);

/*
 * This represents callback handles. There are some private types that
 * are defined and used in vpi_callback.cc. The __vpiCallback are
 * always used in association with vvp_vpi_callback objects.
 */
struct __vpiCallback : public __vpiHandle {
      __vpiCallback();
      ~__vpiCallback();
      int get_type_code(void) const;

	// Used for listing callbacks.
      struct __vpiCallback*next;

	// user supplied callback data
      struct t_cb_data cb_data;

	// The callback holder may use this for various purposes.
      long extra_data;
};

extern void callback_execute(struct __vpiCallback*cur);

struct __vpiSystemTime : public __vpiHandle {
      __vpiSystemTime();
      int get_type_code(void) const;
      int vpi_get(int code);
      char*vpi_get_str(int code);
      void vpi_get_value(p_vpi_value val);
      vpiHandle vpi_handle(int code);

      struct __vpiScope*scope;
};

struct __vpiScopedTime : public __vpiSystemTime {
      __vpiScopedTime();
      char*vpi_get_str(int code);
      void vpi_get_value(p_vpi_value val);
};
struct __vpiScopedSTime : public __vpiSystemTime {
      __vpiScopedSTime();
      int vpi_get(int code);
      char*vpi_get_str(int code);
      void vpi_get_value(p_vpi_value val);
};
struct __vpiScopedRealtime : public __vpiSystemTime {
      __vpiScopedRealtime();
      int vpi_get(int code);
      char*vpi_get_str(int code);
      void vpi_get_value(p_vpi_value val);
};


/*
 * Scopes are created by .scope statements in the source. These
 * objects hold the items and properties that are knowingly bound to a
 * scope.
 */
struct __vpiScope : public __vpiHandle {
      int vpi_get(int code);
      char* vpi_get_str(int code);
      vpiHandle vpi_handle(int code);
      vpiHandle vpi_iterate(int code);

      struct __vpiScope *scope;
	/* The scope has a name. */
      const char*name;
      const char*tname;
      unsigned file_idx;
      unsigned lineno;
      unsigned def_file_idx;
      unsigned def_lineno;
      bool is_automatic;
      bool is_cell;
	/* The scope has a system time of its own. */
      struct __vpiScopedTime scoped_time;
      struct __vpiScopedSTime scoped_stime;
      struct __vpiScopedRealtime scoped_realtime;
	/* Keep an array of internal scope items. */
      class __vpiHandle**intern;
      unsigned nintern;
        /* Keep an array of items to be automatically allocated */
      struct automatic_hooks_s**item;
      unsigned nitem;
        /* Keep a list of live contexts. */
      vvp_context_t live_contexts;
        /* Keep a list of freed contexts. */
      vvp_context_t free_contexts;
	/* Keep a list of threads in the scope. */
      std::set<vthread_t> threads;
      signed int time_units :8;
      signed int time_precision :8;

    protected:
      inline __vpiScope() { }
};

extern struct __vpiScope* vpip_peek_current_scope(void);
extern void vpip_attach_to_scope(struct __vpiScope*scope, vpiHandle obj);
extern void vpip_attach_to_current_scope(vpiHandle obj);
extern struct __vpiScope* vpip_peek_context_scope(void);
extern unsigned vpip_add_item_to_context(automatic_hooks_s*item,
                                         struct __vpiScope*scope);
extern vpiHandle vpip_make_root_iterator(void);
extern void vpip_make_root_iterator(class __vpiHandle**&table,
				    unsigned&ntable);

/*
 * Signals include the variable types (reg, integer, time) and are
 * distinguished by the vpiType code. They also have a parent scope,
 * a declared name and declaration indices.
 */
struct __vpiSignal : public __vpiHandle {
      int vpi_get(int code);
      char* vpi_get_str(int code);
      void vpi_get_value(p_vpi_value val);
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
      vpiHandle vpi_handle(int code);
      vpiHandle vpi_iterate(int code);

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
      unsigned is_netarray  : 1; // This is word of a net array
	/* The represented value is here. */
      vvp_net_t*node;

    public:
      static void*operator new(std::size_t size);
      static void operator delete(void*); // not implemented
    protected:
      inline __vpiSignal() { }
    private: // Not implemented
      static void*operator new[] (std::size_t size);
      static void operator delete[](void*);
};
extern unsigned vpip_size(__vpiSignal *sig);
extern struct __vpiScope* vpip_scope(__vpiSignal*sig);

extern vpiHandle vpip_make_int2(const char*name, int msb, int lsb,
			       bool signed_flag, vvp_net_t*vec);
extern vpiHandle vpip_make_int4(const char*name, int msb, int lsb,
			       vvp_net_t*vec);
extern vpiHandle vpip_make_var4(const char*name, int msb, int lsb,
			       bool signed_flag, vvp_net_t*net);
extern vpiHandle vpip_make_net4(const char*name, int msb, int lsb,
				bool signed_flag, vvp_net_t*node);

/*
 * This is used by system calls to represent a bit/part select of
 * a simple variable or constant array word.
 */
struct __vpiPV : public __vpiHandle {
      __vpiPV();
      int get_type_code(void) const;
      int vpi_get(int code);
      char* vpi_get_str(int code);
      void vpi_get_value(p_vpi_value val);
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
      vpiHandle vpi_handle(int code);

      vpiHandle parent;
      vvp_net_t*net;
      vpiHandle sbase;
      int tbase;
      unsigned twid, width;
      bool is_signed;
};
extern vpiHandle vpip_make_PV(char*name, int base, int width);
extern vpiHandle vpip_make_PV(char*name, char*symbol, int width);
extern vpiHandle vpip_make_PV(char*name, vpiHandle handle, int width);
extern vpiHandle vpip_make_PV(char*name, int tbase, int twid, char*is_signed,
                              int width);

extern void vpip_part_select_value_change(struct __vpiCallback*cbh, vpiHandle obj);


struct __vpiModPathTerm : public __vpiHandle {
      __vpiModPathTerm();
      int get_type_code(void) const;
      int vpi_get(int code);
      vpiHandle vpi_handle(int code);

      vpiHandle expr;
	/* The value returned by vpi_get(vpiEdge, ...); */
      int edge;
};

struct __vpiModPathSrc : public __vpiHandle {
      __vpiModPathSrc();
      int get_type_code(void) const;
      int vpi_get(int code);
      void vpi_get_value(p_vpi_value val);
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
      vpiHandle vpi_handle(int code);
      vpiHandle vpi_iterate(int code);
      vpiHandle vpi_index(int idx);
      void vpi_get_delays(p_vpi_delay del);
      void vpi_put_delays(p_vpi_delay del);
      free_object_fun_t free_object_fun(void);

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


/*
 * The Function is used to create the vpiHandle
 * for vpiModPath && vpiModPathIn objects
 */

extern struct __vpiModPathSrc* vpip_make_modpath_src(struct __vpiModPath*path,
                                                     vvp_net_t *net) ;

extern struct __vpiModPath* vpip_make_modpath(vvp_net_t *net) ;


/*
 * These methods support the vpi creation of events. The name string
 * passed in will be saved, so the caller must allocate it (or not
 * free it) after it is handed to this function.
 */
class __vpiNamedEvent : public __vpiHandle {

    public:
      __vpiNamedEvent(__vpiScope*scope, const char*name);
      int get_type_code(void) const;
      int vpi_get(int code);
      char* vpi_get_str(int code);
      vpiHandle vpi_handle(int code);

      inline void add_vpi_callback(__vpiCallback*cb)
      { cb->next = callbacks_;
	callbacks_ = cb;
      }

      void run_vpi_callbacks(void);

	/* The functor, used for %set operations. */
      vvp_net_t*funct;

    private:
	/* base name of the event object */
      const char*name_;
	/* Parent scope of this object. */
      struct __vpiScope*scope_;
	/* List of callbacks interested in this event. */
      __vpiCallback*callbacks_;
};

extern vpiHandle vpip_make_named_event(const char*name, vvp_net_t*f);
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
struct __vpiRealVar : public __vpiHandle {
      __vpiRealVar();
      int get_type_code(void) const;
      int vpi_get(int code);
      char* vpi_get_str(int code);
      void vpi_get_value(p_vpi_value val);
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
      vpiHandle vpi_handle(int code);
      vpiHandle vpi_iterate(int code);

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
struct __vpiUserSystf : public __vpiHandle {
      __vpiUserSystf();
      int get_type_code(void) const;

      s_vpi_systf_data info;
      bool is_user_defn;
};

extern vpiHandle vpip_make_systf_iterator(void);

extern struct __vpiUserSystf* vpip_find_systf(const char*name);


struct __vpiSysTaskCall : public __vpiHandle {

      struct __vpiScope* scope;
      struct __vpiUserSystf*defn;
      unsigned nargs;
      vpiHandle*args;
	/* Support for vpi_get_userdata. */
      void*userdata;
	/* These represent where in the vthread to put the return value. */
      unsigned vbit;
      signed   vwid;
      class    vvp_net_t*fnet;
      unsigned file_idx;
      unsigned lineno;
      bool put_value;
    protected:
      inline __vpiSysTaskCall() { }
};

extern struct __vpiSysTaskCall*vpip_cur_task;

/*
 * The persistent flag to vpip_make_string_const causes the created
 * handle to be persistent. This is necessary for cases where the
 * string handle may be reused, which is the normal case.
 *
 * When constructing with a string, the class takes possession of the
 * text value string, and will delete it in the constructor.
 */

vpiHandle vpip_make_string_const(char*text, bool persistent =true);
vpiHandle vpip_make_string_param(char*name, char*value,
                                 long file_idx, long lineno);

struct __vpiBinaryConst : public __vpiHandle {
      __vpiBinaryConst();
      int get_type_code(void) const;
      int vpi_get(int code);
      void vpi_get_value(p_vpi_value val);

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

struct __vpiDecConst : public __vpiHandle {
      __vpiDecConst(int val =0);
      int get_type_code(void) const;
      int vpi_get(int code);
      void vpi_get_value(p_vpi_value val);

      int value;
};

class __vpiRealConst : public __vpiHandle {
    public:
      __vpiRealConst(double);
      int get_type_code(void) const;
      int vpi_get(int code);
      void vpi_get_value(p_vpi_value val);
    public:
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
vpiHandle vpip_make_vthr_A(char*label, unsigned tbase, unsigned twid,
                           char*is_signed);
vpiHandle vpip_make_vthr_A(char*label, vpiHandle handle);
vpiHandle vpip_make_vthr_APV(char*label, unsigned index, unsigned bit, unsigned wid);

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
				     class vvp_net_t*fnet,
				     bool func_as_task_err,
				     bool func_as_task_warn,
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
				 unsigned nbuf);

extern void vpip_vec4_to_oct_str(const vvp_vector4_t&bits, char*buf,
				 unsigned nbuf);

extern void vpip_bin_str_to_vec4(vvp_vector4_t&val, const char*buf);
extern void vpip_oct_str_to_vec4(vvp_vector4_t&val, const char*str);
extern void vpip_dec_str_to_vec4(vvp_vector4_t&val, const char*str);
extern void vpip_hex_str_to_vec4(vvp_vector4_t&val, const char*str);

extern vvp_vector4_t vec4_from_vpi_value(s_vpi_value*vp, unsigned wid);
extern double real_from_vpi_value(s_vpi_value*vp);

extern void vpip_vec4_get_value(const vvp_vector4_t&word_val, unsigned width,
				bool signed_flag, s_vpi_value*vp);
extern void vpip_vec2_get_value(const vvp_vector2_t&word_val, unsigned width,
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

extern int vpip_delay_selection;

#endif
