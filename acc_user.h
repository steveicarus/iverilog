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
#ident "$Id: acc_user.h,v 1.6 2002/08/12 01:34:58 steve Exp $"
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

/*
 * This is a declaration of the "handle" type that is compatible with
 * the vpiHandle from vpi_user.h.
 */
typedef struct __vpiHandle *handle;

/* OBJECT TYPES */
#define accScope   21
#define accScalar  300
#define accVector  302

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

extern int acc_error_flag;

extern int acc_initialize(void);

extern void acc_close(void);

extern int   acc_fetch_argc(void);
extern char**acc_fetch_argv(void);

extern char* acc_fetch_fullname(handle obj);

extern int   acc_fetch_tfarg_int(int n);
extern char* acc_fetch_tfarg_str(int n);

extern handle acc_handle_tfarg(int n);

extern handle acc_next_topmod(handle prev_topmod);

extern int acc_object_of_type(handle object, int type);

extern char*acc_product_version(void);

extern int acc_set_value(handle obj, p_setval_value value,
			 p_setval_delay delay);

extern char* acc_version(void);

EXTERN_C_END

/*
 * $Log: acc_user.h,v $
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
