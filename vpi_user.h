#ifndef __vpi_user_H
#define __vpi_user_H
/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_user.h,v 1.36 2004/10/04 01:10:56 steve Exp $"
#endif


#if defined(__MINGW32__) || defined (__CYGWIN32__)
#  define DLLEXPORT __declspec(dllexport)
#else
#  define DLLEXPORT
#endif

#ifdef __cplusplus
# define EXTERN_C_START extern "C" {
# define EXTERN_C_END }
#else
# define EXTERN_C_START
# define EXTERN_C_END
#endif

#ifndef __GNUC__
# undef  __attribute__
# define __attribute__(x)
#endif

EXTERN_C_START

# include  <stdarg.h>
# include  <stdio.h>
# include  "_pli_types.h"

typedef struct __vpiHandle *vpiHandle;

/*
 * This structure is created by the VPI application to provide hooks
 * into the application that the compiler/simulator can access.
 */
typedef struct t_vpi_systf_data {
      PLI_INT32 type;
      PLI_INT32 sysfunctype;
      const char *tfname;
      PLI_INT32 (*calltf)(char*);
      PLI_INT32 (*compiletf)(char*);
      PLI_INT32 (*sizetf)(char*);
      char      *user_data;
} s_vpi_systf_data, *p_vpi_systf_data;

/* The type in the above structure can have one of the following
   values: */
#define vpiSysTask  1
#define vpiSysFunc  2

typedef struct t_vpi_vlog_info
{
      PLI_INT32 argc;
      char      **argv;
      char      *product;
      char      *version;
} s_vpi_vlog_info, *p_vpi_vlog_info;


typedef struct t_vpi_time {
      PLI_INT32 type;
      PLI_UINT32 high;
      PLI_UINT32 low;
      double real;
} s_vpi_time, *p_vpi_time;

#define vpiScaledRealTime 1
#define vpiSimTime        2
#define vpiSuppressTime   3

typedef struct t_vpi_vecval {
      PLI_INT32 aval, bval; /* ab encoding: 00=0, 10=1, 11=X, 01=Z */
} s_vpi_vecval, *p_vpi_vecval;

typedef struct t_vpi_strengthval {
      PLI_INT32 logic;
      PLI_INT32 s0, s1;
} s_vpi_strengthval, *p_vpi_strengthval;

/*
 * This structure holds values that are passed back and forth between
 * the simulator and the application.
 */
typedef struct t_vpi_value {
      PLI_INT32 format;
      union {
	    char      *str;
	    PLI_INT32 scalar;
	    PLI_INT32 integer;
	    double real;
	    struct t_vpi_time *time;
	    struct t_vpi_vecval *vector;
	    struct t_vpi_strengthval *strength;
	    char      *misc;
      } value;
} s_vpi_value, *p_vpi_value;

/* These are valid codes for the format of the t_vpi_value structure. */
#define vpiBinStrVal    1
#define vpiOctStrVal    2
#define vpiDecStrVal    3
#define vpiHexStrVal    4
#define vpiScalarVal    5
#define vpiIntVal       6
#define vpiRealVal      7
#define vpiStringVal    8
#define vpiVectorVal    9
#define vpiStrengthVal 10
#define vpiTimeVal     11
#define vpiObjTypeVal  12
#define vpiSuppressVal 13


/* SCALAR VALUES */
#define vpi0 0
#define vpi1 1
#define vpiZ 2
#define vpiX 3
#define vpiH 4
#define vpiL 5
#define vpiDontCare 6

/* STRENGTH VALUES */
#define vpiSupplyDrive  0x80
#define vpiStrongDrive  0x40
#define vpiPullDrive    0x20
#define vpiLargeCharge  0x10
#define vpiWeakDrive    0x08
#define vpiMediumCharge 0x04
#define vpiSmallCharge  0x02
#define vpiHiZ          0x01

/* OBJECT CODES */
#define vpiConstant     7
#define vpiFunction    20
#define vpiIntegerVar  25
#define vpiIterator    27
#define vpiMemory      29
#define vpiMemoryWord  30
#define vpiModule      32
#define vpiNamedBegin  33
#define vpiNamedEvent  34
#define vpiNamedFork   35
#define vpiNet         36
#define vpiParameter   41
#define vpiRealVar     47
#define vpiReg         48
#define vpiSysFuncCall 56
#define vpiSysTaskCall 57
#define vpiTask        59
#define vpiTimeVar     63
#define vpiIndex       78
#define vpiLeftRange   79
#define vpiRightRange  83
#define vpiScope       84
#define vpiSysTfCall   85
#define vpiArgument    89
#define vpiInternalScope 92
#define vpiVariables   100

#define vpiCallback  1000

/* PROPERTIES */
#define vpiUndefined  (-1)
#define vpiType       1
#define vpiName       2
#define vpiFullName   3
#define vpiSize       4
#define vpiDefName    9
#define vpiTimeUnit      11
#define vpiTimePrecision 12
#define vpiNetType    22
#   define vpiWire        1
#define vpiConstType 40
#   define vpiDecConst    1
#   define vpiRealConst   2
#   define vpiBinaryConst 3
#   define vpiOctConst    4
#   define vpiHexConst    5
#   define vpiStringConst 6
#define vpiFuncType  44
#   define vpiIntFunc     1
#   define vpiRealFunc    2
#   define vpiTimeFunc    3
#   define vpiSizedFunc   4
#   define vpiSizedSignedFunc 5
#define vpiSysFuncType     vpiFuncType
#   define vpiSysFuncInt   vpiIntFunc
#   define vpiSysFuncReal  vpiRealFunc
#   define vpiSysFuncTime  vpiTimeFunc
#   define vpiSysFuncSized vpiSizedFunc
#define vpiSigned    65
/* IVL private properties */
#define _vpiNexusId 0x1000000

/* DELAY MODES */
#define vpiNoDelay            1
#define vpiInertialDelay      2
#define vpiTransportDelay     3
#define vpiPureTransportDelay 4

#define vpiForceFlag   5
#define vpiReleaseFlag 6


/* VPI FUNCTIONS */
extern void vpi_register_systf(const struct t_vpi_systf_data*ss);

/* I/O routines */
extern PLI_UINT32 vpi_mcd_open(char *name);
extern PLI_UINT32 vpi_mcd_close(PLI_UINT32 mcd);
extern char      *vpi_mcd_name(PLI_UINT32 mcd);
extern PLI_INT32  vpi_mcd_printf(PLI_UINT32 mcd, const char*fmt, ...)
      __attribute__((format (printf,2,3)));

extern PLI_INT32  vpi_printf(const char*fmt, ...)
      __attribute__((format (printf,1,2)));

extern PLI_INT32  vpi_vprintf(const char*fmt, va_list ap);
extern PLI_INT32  vpi_mcd_vprintf(PLI_UINT32 mcd, const char*fmt, va_list ap);

extern PLI_INT32  vpi_flush(void);
extern PLI_INT32  vpi_mcd_flush(PLI_UINT32 mcd);

/* proposed extensions */
/*
 * These functions are proposed extensions to Verilog, and
 * are described by the Verilog PLI task force as issue#347.
 *
 * The vpi_fopen() function is exactly the same as the $fopen system
 * function. That is, it takes a path string and a mode string, and
 * opens the file. The result is a 32bit value with bit 31 set, the
 * remaining bits made up a small integer to represent the file.
 *
 * The vpi_get_file(fd) function takes as input a descriptor as
 * returned by vpi_fopen or $fopen. Bit 31 must be set. The result
 * is the C FILE* that represents the file.
 */
extern PLI_INT32 vpi_fopen(const char*name, const char*mode);
extern FILE *vpi_get_file(PLI_INT32 fd);

/*
 * support for VPI callback functions.
 */
typedef struct t_cb_data {
      PLI_INT32 reason;
      PLI_INT32 (*cb_rtn)(struct t_cb_data*cb);
      vpiHandle obj;
      p_vpi_time time;
      p_vpi_value value;
      PLI_INT32 index;
      char      *user_data;
} s_cb_data, *p_cb_data;

#define cbValueChange        1
#define cbStmt               2
#define cbForce              3
#define cbRelease            4
#define cbAtStartOfSimTime   5
#define cbReadWriteSynch     6
#define cbReadOnlySynch      7
#define cbNextSimTime        8
#define cbAfterDelay         9
#define cbEndOfCompile      10
#define cbStartOfSimulation 11
#define cbEndOfSimulation   12
#define cbError             13
#define cbTchkViolation     14
#define cbStartOfSave       15
#define cbEndOfSave         16
#define cbStartOfRestart    17
#define cbEndOfRestart      18
#define cbStartOfReset      19
#define cbEndOfReset        20
#define cbEnterInteractive  21
#define cbExitInteractive   22
#define cbInteractiveScopeChange 23
#define cbUnresolvedSystf   24

extern vpiHandle vpi_register_cb(p_cb_data data);
extern PLI_INT32 vpi_remove_cb(vpiHandle ref);

/*
 * This function allows a vpi application to control the simulation
 * engine. The operation parameter specifies the function to
 * perform. The remaining parameters (if any) are interpreted by the
 * operation. The vpi_sim_control definition was added to P1364-2000
 * 14 July 1999. See PLI Task Force ID: PTF-161
 *
 * vpiFinish - perform the $finish operation, as soon as the user
 *             function returns. This operation takes a single
 *             parameter, a diagnostic exit code.
 *
 * vpiStop -
 * vpiReset -
 * vpiSetInteractiveScope -
 */
extern void vpi_control(PLI_INT32 operation, ...);
#define vpiStop 1
#define vpiFinish 2
#define vpiReset  3
#define vpiSetInteractiveScope 4

/* vpi_sim_control is the incorrect name for vpi_control. */
extern void vpi_sim_control(PLI_INT32 operation, ...);

extern vpiHandle  vpi_handle(PLI_INT32 type, vpiHandle ref);
extern vpiHandle  vpi_iterate(PLI_INT32 type, vpiHandle ref);
extern vpiHandle  vpi_scan(vpiHandle iter);
extern vpiHandle  vpi_handle_by_index(vpiHandle ref, PLI_INT32 index);
extern vpiHandle  vpi_handle_by_name(const char*name, vpiHandle scope);

extern void  vpi_get_time(vpiHandle obj, s_vpi_time*t);
extern PLI_INT32 vpi_get(int property, vpiHandle ref);
extern char      *vpi_get_str(PLI_INT32 property, vpiHandle ref);
extern void  vpi_get_value(vpiHandle expr, p_vpi_value value);

/*
 * This function puts a value into the object referenced by the
 * handle. This assumes that the value supports having its value
 * written to. The time parameter specifies when the assignment is to
 * take place. This allows you to schedule an assignment to happen in
 * the future.
 *
 * The flags value specifies the delay model to use in assigning the
 * value. This specifies how the time value is to be used.
 *
 *  vpiNoDelay -- Set the value immediately. The p_vpi_time parameter
 *      may be NULL, in this case. This is like a blocking assignment
 *      in behavioral code.
 *
 *  vpiInertialDelay -- Set the value using the transport delay. The
 *      p_vpi_time parameter is required and specifies when the
 *      assignment is to take place. This is like a non-blocking
 *      assignment in behavioral code.
 */
extern vpiHandle vpi_put_value(vpiHandle obj, p_vpi_value value,
			       p_vpi_time when, PLI_INT32 flags);

extern PLI_INT32 vpi_free_object(vpiHandle ref);
extern PLI_INT32 vpi_get_vlog_info(p_vpi_vlog_info vlog_info_p);

/*
 * These functions support attaching user data to an instance of a
 * system task or function. These functions only apply to
 * vpiSysTaskCall or vpiSysFuncCall handles.
 */
extern PLI_INT32 vpi_put_userdata(vpiHandle obj, void*data);
extern void*vpi_get_userdata(vpiHandle obj);

/*
 * Support for handling errors.
 */
typedef struct t_vpi_error_info {
      PLI_INT32 state;
      PLI_INT32 level;
      char      *message;
      char      *product;
      char      *code;
      char      *file;
      PLI_INT32 line;
} s_vpi_error_info, *p_vpi_error_info;

/* error_info states */
# define  vpiCompile  1
# define  vpiPLI      2
# define  vpiRun      3

/* error_info levels */
# define  vpiNotice    1
# define  vpiWarning   2
# define  vpiError     3
# define  vpiSystem    4
# define  vpiInternal  5

extern PLI_INT32 vpi_chk_error(p_vpi_error_info info);


/* This is the table of startup routines included in each module. */
extern DLLEXPORT void (*vlog_startup_routines[])();


/*
 * ICARUS VERILOG EXTENSIONS
 *
 * The vpip_* functions are Icarus Verilog extensions. They are not
 * standard VPI functions, so use these at your own risk.
 *
 * The vpip_format_* functions format values in string format in the
 * manner of the $display system task.
 */

  /* Format a scalar a la %v. The str points to a 4byte character
     buffer. The value must be a vpiStrengthVal. */
extern void vpip_format_strength(char*str, s_vpi_value*value);

EXTERN_C_END

/*
 * $Log: vpi_user.h,v $
 * Revision 1.36  2004/10/04 01:10:56  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.35  2004/09/05 21:30:16  steve
 *  Better type safety.
 *
 * Revision 1.34  2004/03/09 04:29:26  steve
 *  Define function types.
 *
 * Revision 1.33  2004/02/19 21:32:46  steve
 *  Add sysfunctype to calltf structure.
 *
 * Revision 1.32  2004/01/13 02:55:50  steve
 *  Get value for vpoiConstType correct.
 *
 * Revision 1.31  2003/12/07 20:06:24  steve
 *  tfname can be constant.
 *
 * Revision 1.30  2003/10/30 03:42:51  steve
 *  Details on the vpi_get_file function.
 *
 * Revision 1.29  2003/07/15 03:49:22  steve
 *  Spelling fixes.
 *
 * Revision 1.28  2003/06/04 01:56:20  steve
 * 1) Adds configure logic to clean up compiler warnings
 * 2) adds acc_compare_handle, acc_fetch_range, acc_next_scope and
 *    tf_isetrealdelay, acc_handle_scope
 * 3) makes acc_next reentrant
 * 4) adds basic vpiWire type support
 * 5) fills in some acc_object_of_type() and acc_fetch_{full}type()
 * 6) add vpiLeftRange/RigthRange to signals
 *
 * Revision 1.27  2003/05/29 02:21:45  steve
 *  Implement acc_fetch_defname and its infrastructure in vvp.
 *
 * Revision 1.26  2003/05/23 04:04:02  steve
 *  Add vpi_fopen and vpi_get_file.
 *
 * Revision 1.25  2003/05/15 16:51:08  steve
 *  Arrange for mcd id=00_00_00_01 to go to stdout
 *  as well as a user specified log file, set log
 *  file to buffer lines.
 *
 *  Add vpi_flush function, and clear up some cunfused
 *  return codes from other vpi functions.
 *
 *  Adjust $display and vcd/lxt messages to use the
 *  standard output/log file.
 *
 * Revision 1.24  2003/04/20 02:49:07  steve
 *  acc_fetch_value support for %v format.
 *
 * Revision 1.23  2003/03/13 18:26:12  steve
 *  Verilog 2001 standart types.
 *
 * Revision 1.22  2003/03/13 04:34:35  steve
 *  Add VPI_TRACE tracing of VPI calls.
 *  vpi_handle_by_name takes a const char*.
 *
 * Revision 1.21  2003/03/12 02:49:38  steve
 *  Move _vpiNexisId safely out of the way.
 *
 * Revision 1.20  2003/03/10 23:40:54  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.19  2003/02/17 06:39:47  steve
 *  Add at least minimal implementations for several
 *  acc_ functions. Add support for standard ACC
 *  string handling.
 *
 *  Add the _pli_types.h header file to carry the
 *  IEEE1364-2001 standard PLI type declarations.
 *
 * Revision 1.18  2003/02/01 05:50:27  steve
 *  Make $time and $realtime available to $display uniquely.
 *
 * Revision 1.17  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.16  2003/01/09 04:10:17  steve
 *  Add vpi_put_userdata
 */
#endif
