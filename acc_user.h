#ifndef __acc_user_H
#define __acc_user_H
/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: acc_user.h,v 1.11 2003/04/24 18:57:05 steve Exp $"
#endif

/*
 * This header file contains the definitions and declarations needed
 * by an Icarus Verilog user using acc_ routines.
 *
 * NOTE: Icarus Verilog does not support acc_ routines. This is just a
 * stub. The functions that are implemented here are actually
 * implemented using VPI routines.
 */


#ifdef __cplusplus
# define EXTERN_C_START extern "C" {
# define EXTERN_C_END }
#else
# define EXTERN_C_START
# define EXTERN_C_END
# define bool int
#endif

EXTERN_C_START

# include  "_pli_types.h"

/*
 * This is a declaration of the "handle" type that is compatible with
 * the vpiHandle from vpi_user.h. The libveriuser library implements
 * the acc handle type as a vpiHandle, to prevent useless indirection.
 */
typedef struct __vpiHandle *handle;

/* OBJECT TYPES */
#define accModule    20
#define accScope     21
#define accNet       25
#define accReg       30
#define accStringParam 204
#define accParameter   220
#define accIntegerVar  281
#define accIntVar      281
#define accScalar    300
#define accVector    302
#define accConstant  600

/* type VALUES FOR t_setval_delay STRUCTURE */
#define accNoDelay  0
#define accInertialDelay 1
#define accTransportDelay 2
#define accPureTransportDelay 3
#define accForceFlag 4
#define accReleaseFlag 5

/* type VALUES FOR t_setval_value STRUCTURE */
#define accBinStrVal 1
#define accOctStrVal 2
#define accDecStrVal 3
#define accHexStrVal 4
#define accScalarVal 5
#define accIntVal 6
#define accRealVal 7
#define accStringVal 8
#define accVectorVal 9

/* type VALUES FOR t_acc_time STRUCTURE */
#define accTime 1
#define accSimTime 2
#define accRealTime 3

/* Constants used by acc_vcl_add */
#define vcl_verilog_logic 2
#define VCL_VERILOG_LOGIC vcl_verilog_logic
#define vcl_verilog_strength 3
#define VCL_VERILOG_STRENGTH vcl_verilog_strength

typedef struct t_acc_time {
      int type;
      int low, high;
      double real;
} s_acc_time, *p_acc_time;

typedef struct t_setval_delay {
      s_acc_time time;
      int model;
} s_setval_delay, *p_setval_delay;

typedef struct t_acc_vecval {
      int aval;
      int bval;
} s_acc_vecval, *p_acc_vecval;

typedef struct t_setval_value {
      int format;
      union {
	    char*str;
	    int scalar;
	    int integer;
	    double real;
	    p_acc_vecval vector;
      } value;
} s_setval_value, *p_setval_value, s_acc_value, *p_acc_value;

typedef struct t_strengths {
      PLI_UBYTE8 logic_value;
      PLI_UBYTE8 strength1;
      PLI_UBYTE8 strength2;
} s_strengths, *p_strengths;

typedef struct t_vc_record {
      PLI_INT32 vc_reason;
      PLI_INT32 vc_hightime;
      PLI_INT32 vc_lowtime;
      void*     user_data;
      union {
	    PLI_UBYTE8 logic_value;
	    double real_value;
	    handle vector_handle;
	    s_strengths strengths_s;
      } out_value;
} s_vc_record, *p_vc_record;

typedef struct t_location {
      PLI_INT32 line_no;
      const char*filename;
} s_location, *p_location;

extern int acc_error_flag;

extern int acc_initialize(void);

extern void acc_close(void);

/*
 * This is the acc_configure command, and the config_param
 * codes that are accepted.
 */
extern int   acc_configure(PLI_INT32 config_param, const char*value);
#define accDevelopmentVersion 11

extern int   acc_fetch_argc(void);
extern char**acc_fetch_argv(void);

extern char* acc_fetch_fullname(handle obj);

extern int   acc_fetch_location(p_location loc, handle obj);

extern char* acc_fetch_name(handle obj);

extern double acc_fetch_paramval(handle obj);

extern int   acc_fetch_tfarg_int(int n);
extern char* acc_fetch_tfarg_str(int n);

typedef struct t_timescale_info {
      PLI_INT16 unit;
      PLI_INT16 precision;
} s_timescale_info, *p_timescale_info;
extern void acc_fetch_timescale_info(handle obj, p_timescale_info info);

extern PLI_INT32 acc_fetch_size(handle obj);

extern PLI_INT32 acc_fetch_type(handle obj);
extern PLI_INT32 acc_fetch_fulltype(handle obj);
extern char*     acc_fetch_type_str(PLI_INT32 type);

extern char* acc_fetch_value(handle obj, const char*fmt, s_acc_value*value);

extern handle acc_handle_object(const char*name);
extern handle acc_handle_parent(handle obj);

extern handle acc_handle_tfarg(int n);
extern handle acc_handle_tfinst(void);

extern handle acc_next_topmod(handle prev_topmod);

extern int acc_object_in_typelist(handle object, PLI_INT32*typelist);
extern int acc_object_of_type(handle object, PLI_INT32 type);

extern char*acc_product_version(void);

extern int acc_set_value(handle obj, p_setval_value value,
			 p_setval_delay delay);

extern void acc_vcl_add(handle obj, PLI_INT32(*consumer)(p_vc_record),
			void*data, PLI_INT32 vcl_flag);
extern  void acc_vcl_delete(handle obj, PLI_INT32(*consumer)(p_vc_record),
			    void*data, PLI_INT32 vcl_flag);

extern char* acc_version(void);

EXTERN_C_END

/*
 * $Log: acc_user.h,v $
 * Revision 1.11  2003/04/24 18:57:05  steve
 *  Add acc_fetch_fulltype function.
 *
 * Revision 1.10  2003/04/20 02:48:39  steve
 *  Support value change callbacks.
 *
 * Revision 1.9  2003/04/12 18:57:13  steve
 *  More acc_ function stubs.
 *
 * Revision 1.8  2003/03/13 04:35:09  steve
 *  Add a bunch of new acc_ and tf_ functions.
 *
 * Revision 1.7  2003/02/17 06:39:47  steve
 *  Add at least minimal implementations for several
 *  acc_ functions. Add support for standard ACC
 *  string handling.
 *
 *  Add the _pli_types.h header file to carry the
 *  IEEE1364-2001 standard PLI type declarations.
 *
 * Revision 1.6  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2002/06/11 15:19:12  steve
 *  Add acc_fetch_argc/argv/version (mruff)
 *
 * Revision 1.4  2002/06/07 02:58:58  steve
 *  Add a bunch of acc/tf functions. (mruff)
 *
 * Revision 1.3  2002/06/02 19:03:29  steve
 *  Add acc_handle_tfarg and acc_next_topmode
 *
 * Revision 1.2  2002/05/30 02:06:05  steve
 *  Implement acc_product_version.
 *
 * Revision 1.1  2002/05/23 03:46:42  steve
 *  Add the acc_user.h header file.
 *
 */

#endif
