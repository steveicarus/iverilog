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
#ident "$Id: vpi_user.h,v 1.15 2000/05/04 03:37:59 steve Exp $"
#endif

#ifdef __cplusplus
extern "C" {
#endif

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
#define vpiScalerVal    5
#define vpiIntVal       6
#define vpiReadVal      7
#define vpiStringVal    8
#define vpiVectorVal    9
#define vpiStrengthVal 10
#define vpiTimeVal     11
#define vpiObjTypeVal  12
#define vpiSuppressVal 13


/* OBJECT CODES */
#define vpiConstant     7
#define vpiFunction    20
#define vpiIterator    27
#define vpiMemory      29
#define vpiMemoryWord  30
#define vpiModule      32
#define vpiNamedBegin  33
#define vpiNamedFork   35
#define vpiNet         36
#define vpiReg         48
#define vpiSysFuncCall 56
#define vpiSysTaskCall 57
#define vpiTask        59
#define vpiTimeVar     63
#define vpiSysTfCall   85
#define vpiArgument    89
#define vpiInternalScope 92

#define vpiCallback  1000

/* PROPERTIES */
#define vpiType       1
#define vpiName       2
#define vpiFullName   3
#define vpiSize       4
#define vpiConstType 43
#   define vpiDecConst    1
#   define vpiRealConst   2
#   define vpiBinaryConst 3
#   define vpiOctConst    4
#   define vpiHexConst    5
#   define vpiStringConst 6

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
extern vpiHandle vpi_put_value(vpiHandle obj, p_vpi_value value,
			       p_vpi_time when, int flags);

extern int vpi_free_object(vpiHandle ref);


/* This is the table of startup routines included in each module. */
extern void (*vlog_startup_routines[])();

#ifdef __cplusplus
}
#endif

/*
 * $Log: vpi_user.h,v $
 * Revision 1.15  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.14  2000/03/08 04:36:54  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.13  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.12  2000/02/13 19:18:28  steve
 *  Accept memory words as parameter to $display.
 *
 * Revision 1.11  2000/01/20 06:04:55  steve
 *  $dumpall checkpointing in VCD dump.
 *
 * Revision 1.10  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 * Revision 1.9  1999/11/28 00:56:08  steve
 *  Build up the lists in the scope of a module,
 *  and get $dumpvars to scan the scope for items.
 *
 * Revision 1.8  1999/11/27 19:07:58  steve
 *  Support the creation of scopes.
 *
 * Revision 1.7  1999/11/10 02:52:24  steve
 *  Create the vpiMemory handle type.
 *
 * Revision 1.6  1999/11/07 20:33:30  steve
 *  Add VCD output and related system tasks.
 *
 * Revision 1.5  1999/11/07 02:25:08  steve
 *  Add the $monitor implementation.
 *
 * Revision 1.4  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 * Revision 1.3  1999/08/19 02:51:03  steve
 *  Add vpi_sim_control
 *
 * Revision 1.2  1999/08/18 03:44:49  steve
 *  declare vou_sim_control
 *
 * Revision 1.1  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 */
#endif
