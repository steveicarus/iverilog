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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vpi_user.h,v 1.10 2002/05/23 03:34:46 steve Exp $"
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

EXTERN_C_START

# include  <stdarg.h>

typedef struct __vpiHandle *vpiHandle;

/*
 * This structure is created by the VPI application to provide hooks
 * into the application that the compiler/simulator can access.
 */
typedef struct t_vpi_systf_data {
      int type;
      int subtype;
      char *tfname;
      int (*calltf)(char*);
      int (*compiletf)(char*);
      int (*sizetf)();
      char *user_data;
} s_vpi_systf_data, *p_vpi_systf_data;

/* The type in the above structure can have one of the following
   values: */
#define vpiSysTask  1
#define vpiSysFunc  2

typedef struct t_vpi_vlog_info
{
      int argc;
      char **argv;
      char *product;
      char *version;
} s_vpi_vlog_info, *p_vpi_vlog_info;


typedef struct t_vpi_time {
      int type;
      unsigned int high;
      unsigned int low;
      double real;
} s_vpi_time, *p_vpi_time;

#define vpiScaledRealTime 1
#define vpiSimTime        2
#define vpiSuppressTime   3

typedef struct t_vpi_vecval {
      int aval, bval; /* ab encoding: 00=0, 10=1, 11=X, 01=Z */
} s_vpi_vecval, *p_vpi_vecval;

typedef struct t_vpi_strengthval {
      int logic;
      int s0, s1;
} s_vpi_strengthval, *p_vpi_strengthval;

/*
 * This structure holds values that are passed back and forth between
 * the simulator and the application.
 */
typedef struct t_vpi_value {
      int format;
      union {
	    char*str;
	    int scalar;
	    int integer;
	    double real;
	    struct t_vpi_time *time;
	    struct t_vpi_vecval *vector;
	    struct t_vpi_strengthval *strength;
	    char*misc;
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
#define vpiIterator    27
#define vpiMemory      29
#define vpiMemoryWord  30
#define vpiModule      32
#define vpiNamedBegin  33
#define vpiNamedEvent  34
#define vpiNamedFork   35
#define vpiNet         36
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

#define vpiCallback  1000

/* PROPERTIES */
#define vpiType       1
#define vpiName       2
#define vpiFullName   3
#define vpiSize       4
#define vpiTimeUnit      11
#define vpiTimePrecision 12
#define vpiConstType 43
#   define vpiDecConst    1
#   define vpiRealConst   2
#   define vpiBinaryConst 3
#   define vpiOctConst    4
#   define vpiHexConst    5
#   define vpiStringConst 6
#define vpiSigned    65
/* IVL private properties */
#define _vpiNexusId 128

/* DELAY MODES */
#define vpiNoDelay            1
#define vpiInertialDelay      2
#define vpiTransportDelay     3
#define vpiPureTransportDelay 4

#define vpiForceFlag   5
#define vpiReleaseFlag 6


/* VPI FUNCTIONS */
extern void vpi_register_systf(const struct t_vpi_systf_data*ss);
extern void vpi_printf(const char*fmt, ...);
  /* vpi_vprintf is non-standard. */
extern void vpi_vprintf(const char*fmt, va_list ap);

extern unsigned int vpi_mcd_close(unsigned int mcd);
extern char *vpi_mcd_name(unsigned int mcd);
extern unsigned int vpi_mcd_open(char *name);
extern unsigned int vpi_mcd_open_x(char *name, char *mode);
extern int vpi_mcd_printf(unsigned int mcd, const char*fmt, ...);
extern int vpi_mcd_fputc(unsigned int mcd, unsigned char x);
extern int vpi_mcd_fgetc(unsigned int mcd);

/*
 * support for VPI callback functions.
 */
typedef struct t_cb_data {
      int reason;
      int (*cb_rtn)(struct t_cb_data*cb);
      vpiHandle obj;
      p_vpi_time time;
      p_vpi_value value;
      int index;
      char*user_data;
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
extern int vpi_remove_cb(vpiHandle ref);

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
extern void vpi_sim_control(int operation, ...);
#define vpiStop 1
#define vpiFinish 2
#define vpiReset  3
#define vpiSetInteractiveScope 4

extern vpiHandle  vpi_handle(int type, vpiHandle ref);
extern vpiHandle  vpi_iterate(int type, vpiHandle ref);
extern vpiHandle  vpi_scan(vpiHandle iter);
extern vpiHandle  vpi_handle_by_index(vpiHandle ref, int index);

extern void  vpi_get_time(vpiHandle obj, s_vpi_time*t);
extern int   vpi_get(int property, vpiHandle ref);
extern char* vpi_get_str(int property, vpiHandle ref);
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
			       p_vpi_time when, int flags);

extern int vpi_free_object(vpiHandle ref);
extern int vpi_get_vlog_info(p_vpi_vlog_info vlog_info_p);


/* This is the table of startup routines included in each module. */
extern DLLEXPORT void (*vlog_startup_routines[])();

EXTERN_C_END

/*
 * $Log: vpi_user.h,v $
 * Revision 1.10  2002/05/23 03:34:46  steve
 *  Export the vpi_vprintf function.
 *
 * Revision 1.9  2002/05/18 02:34:11  steve
 *  Add vpi support for named events.
 *
 *  Add vpi_mode_flag to track the mode of the
 *  vpi engine. This is for error checking.
 *
 * Revision 1.8  2002/05/17 16:13:08  steve
 *  Add vpiIndex update.
 *
 * Revision 1.7  2002/01/24 04:19:39  steve
 *  Add the vpiLeft.. and vpiRightRange constants
 *
 * Revision 1.6  2001/09/30 05:18:46  steve
 *  Reduce VCD output by removing duplicates. (Stephan Boettcher)
 *
 * Revision 1.5  2001/05/20 15:09:40  steve
 *  Mingw32 support (Venkat Iyer)
 *
 * Revision 1.4  2001/05/10 00:16:00  steve
 *  Add the vpi_user strength definitions.
 *
 * Revision 1.3  2001/04/25 04:45:52  steve
 *  Implement vpi_put_value for signals.
 *
 * Revision 1.2  2001/03/22 02:23:17  steve
 *  fgetc patch from Peter Monta.
 *
 * Revision 1.1  2001/03/19 01:21:45  steve
 *  vpi_user header file is a root header.
 */
#endif
