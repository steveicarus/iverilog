#ifndef VPI_USER_H
#define VPI_USER_H
/*
 * Copyright (c) 1999-2020 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#if defined(__MINGW32__) || defined (__CYGWIN__)
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
# include  <stdarg.h>
# include  "_pli_types.h"

#ifndef ICARUS_VPI_CONST
#define ICARUS_VPI_CONST
#endif
#ifdef __cplusplus
typedef class __vpiHandle *vpiHandle;
#else
typedef struct __vpiHandle *vpiHandle;
#endif

/*
 * This structure is created by the VPI application to provide hooks
 * into the application that the compiler/simulator can access.
 */
typedef struct t_vpi_systf_data {
      PLI_INT32 type;
      PLI_INT32 sysfunctype;
      const char *tfname;
      PLI_INT32 (*calltf)   (ICARUS_VPI_CONST PLI_BYTE8*);
      PLI_INT32 (*compiletf)(ICARUS_VPI_CONST PLI_BYTE8*);
      PLI_INT32 (*sizetf)   (ICARUS_VPI_CONST PLI_BYTE8*);
      ICARUS_VPI_CONST PLI_BYTE8 *user_data;
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

      /*
	Type can be :

	vpiScaledRealTime == 1
	vpiSimTime        == 2
	vpiSuppressTime   == 3
      */

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


/*

  Conform the IEEE 1364, We add the
  Standard vpi_delay structure to
  enable the modpath delay values


  Conform IEEE 1364, Pg 670 :

  The "da" field of the s_vpi_delay
  structure shall be a user allocated
  array of "s_vpi_time" structure

  The array shall store delay values returned
  by vpi_get_delay(). The number of elements in
  the array shall be determined by

  (1) The number of delays to be retrieved
      ( normally this is used in vpi_get_delays (..) )
  {
    (1.1) Set by "no_of_delays" field

    (1.2) For the primitive_object, the no_of_delays
        shall be 2 or 3

    (1.3) For path_delay object the no_of_delays shall
        be 1,2,3,6, 12

    (1.4) For timing_check_object, the no_of_delays shall
        be match the number of limits existing in the
	Time Check

    (1.5) For intermodule_path object, the no_of_delays shall
        be 2 or 3
  }




  (2) The "mtm_flag" && "pulsere_flag"


  Normally, if you set mtm = X, pulsere = Y
  then, you will need allocate (num * no_of_delay)
  s_vpi_time elements for 'da' array before calling
  the vpi_get/put_delays (..)

  ---------------------------------------------------------------------------
  |                |                         |                              |
  | mtm_flag       | No of s_vpi_time array  |   order in which delay       |
  | pulsere_flag   | element required by the |   elements shall be filed    |
  |                | s_vpi_delay->da         |                              |
  |                |                         |                              |
  |----------------|-------------------------|------------------------------|
  |                |                         | 1o delay  da[0]--> 1o delay  |
  | mtm = false    | no_of_delay             | 2o delay  da[1]--> 2o delay  |
  | pulere = false |                         |                              |
  |                |                         |                              |
  |----------------|-------------------------|------------------------------|
  |                |                         | 1o delay  da[0]--> min delay |
  | mtm = true     |                         |           da[1]--> typ delay |
  | pulere = false | 3*no_of_delay           |           da[2]--> max delay |
  |                |                         | 2o delay  da[3]--> min delay |
  |                |                         |           da[4]--> typ delay |
  |                |                         |           ....               |
  |----------------|-------------------------|------------------------------|
  |                |                         | 1o delay  da[0]--> delay     |
  | mtm = false    |                         |           da[1]--> rej limit |
  | pulere = true  | 3*no_of_delay           |           da[2]--> err limit |
  |                |                         | 2o delay  da[3]--> delay     |
  |                |                         |           da[4]--> rej limit |
  |                |                         |           ....               |
  |----------------|-------------------------|------------------------------|
  |                |                         | 1o delay da[0]--> min delay  |
  | mtm = true     |                         |          da[1]--> typ delay  |
  | pulere = true  | 9*no_of_delay           |          da[2]--> max delay  |
  |                |                         |          da[3]--> min delay  |
  |                |                         |          da[4]--> typ delay  |
  |                |                         |          da[5]--> max delay  |
  |                |                         |          da[6]--> min delay  |
  |                |                         |          da[7]--> typ delay  |
  |                |                         |          da[8]--> max delay  |
  |                |                         | 2o delay da[9]--> min delay  |
  |                |                         |          ....                |
   -------------------------------------------------------------------------

   IMPORTANT :

   The delay Structure has to be allocated before passing a pointer to
   "vpi_get_delays ( )".

*/


typedef struct t_vpi_delay  {
      struct t_vpi_time  *da; /* Array of delay data */
      PLI_INT32  no_of_delays ;
      PLI_INT32  time_type; /* vpiScaledRealTime, vpiSimTime */
      PLI_INT32  mtm_flag;
      PLI_INT32  append_flag;
      PLI_INT32  pulsere_flag;
} s_vpi_delay, *p_vpi_delay;





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
#define vpiModPath     31
#define vpiModule      32
#define vpiNamedBegin  33
#define vpiNamedEvent  34
#define vpiNamedFork   35
#define vpiNet         36
#define vpiNetBit      37
#define vpiParameter   41
#define vpiPartSelect  42
#define vpiPathTerm    43
#define vpiPort        44
#define vpiRealVar     47
#define vpiReg         48
#define vpiRegBit      49
#define vpiSysFuncCall 56
#define vpiSysTaskCall 57
#define vpiTask        59
#define vpiTimeVar     63
#define vpiUdpDefn     66
#define vpiUserSystf   67
#define vpiNetArray   114
#define vpiIndex       78
#define vpiLeftRange   79
#define vpiParent      81
#define vpiRightRange  83
#define vpiScope       84
#define vpiSysTfCall   85
#define vpiArgument    89
#define vpiInternalScope 92
#define vpiModPathIn     95
#define vpiModPathOut    96
#define vpiVariables   100
#define vpiExpr        102

/********************** object types added with 1364-2001 *********************/

#define vpiCallback   107
#define vpiRegArray   116

/********************** object types added with 1364-2005 *********************/

#define vpiGenScope   134

/* PROPERTIES */
#define vpiUndefined   (-1)
#define vpiType           1
#define vpiName           2
#define vpiFullName       3
#define vpiSize           4
#define vpiFile           5
#define vpiLineNo         6
#define vpiTopModule      7
#define vpiCellInstance   8
#define vpiDefName        9
#define vpiTimeUnit      11
#define vpiTimePrecision 12
#define vpiDefFile       15
#define vpiDefLineNo     16
#define vpiScalar        17
#define vpiVector        18

#define vpiDirection 20 /* direction of port: */
#   define vpiInput 1
#   define vpiOutput 2
#   define vpiInout 3
#   define vpiMixedIO 4 /* Not currently output */
#   define vpiNoDirection 5

#define vpiNetType       22
#   define vpiWire         1
#   define vpiWand         2
#   define vpiWor          3
#   define vpiTri          4
#   define vpiTri0         5
#   define vpiTri1         6
#   define vpiTriReg       7
#   define vpiTriAnd       8
#   define vpiTriOr        9
#   define vpiSupply1     10
#   define vpiSupply0     11
#define vpiArray         28
#define vpiPortIndex     29
#define vpiEdge          36
#   define vpiNoEdge       0x00 /* No edge */
#   define vpiEdge01       0x01 /* 0 --> 1 */
#   define vpiEdge10       0x02 /* 1 --> 0 */
#   define vpiEdge0x       0x04 /* 0 --> x */
#   define vpiEdgex1       0x08 /* x --> 1 */
#   define vpiEdge1x       0x10 /* 1 --> x */
#   define vpiEdgex0       0x20 /* x --> 0 */
#   define vpiPosedge      (vpiEdgex1|vpiEdge01|vpiEdge0x)
#   define vpiNegedge      (vpiEdgex0|vpiEdge10|vpiEdge1x)
#   define vpiAnyEdge      (vpiPosedge|vpiNegedge)
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
#define vpiUserDefn       45
#define vpiAutomatic      50
#define vpiConstantSelect 53
#define vpiSigned         65
#define vpiLocalParam     70
/* IVL private properties, also see vvp/vpi_priv.h for other properties */
#define _vpiNexusId        0x1000000
/* used in vvp/vpi_priv.h  0x1000001 */
#define _vpiDelaySelection 0x1000002
#  define _vpiDelaySelMinimum 1
#  define _vpiDelaySelTypical 2
#  define _vpiDelaySelMaximum 3
/* used in vvp/vpi_priv.h  0x1000003 */
/* used in vvp/vpi_priv.h  0x1000004 */

/* DELAY MODES */
#define vpiNoDelay            1
#define vpiInertialDelay      2
#define vpiTransportDelay     3
#define vpiPureTransportDelay 4

#define vpiForceFlag   5
#define vpiReleaseFlag 6
#define vpiReturnEvent 0x1000

/* VPI FUNCTIONS */
extern vpiHandle vpi_register_systf(const struct t_vpi_systf_data*ss);
extern void vpi_get_systf_info(vpiHandle obj, p_vpi_systf_data data);

/* I/O routines */
extern PLI_UINT32 vpi_mcd_open(char *name);
extern PLI_UINT32 vpi_mcd_close(PLI_UINT32 mcd);
extern char      *vpi_mcd_name(PLI_UINT32 mcd);
extern PLI_INT32  vpi_mcd_printf(PLI_UINT32 mcd, const char*fmt, ...)
#ifdef __MINGW32__
      __attribute__((format (gnu_printf,2,3)));
#else
      __attribute__((format (printf,2,3)));
#endif
extern PLI_INT32  vpi_printf(const char*fmt, ...)
#ifdef __MINGW32__
      __attribute__((format (gnu_printf,1,2)));
#else
      __attribute__((format (printf,1,2)));
#endif

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
      ICARUS_VPI_CONST PLI_BYTE8 *user_data;
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
#define cbAtEndOfSimTime    31

extern vpiHandle vpi_register_cb(p_cb_data data);
extern PLI_INT32 vpi_remove_cb(vpiHandle ref);

/*
 * This function allows a vpi application to control the simulation
 * engine. The operation parameter specifies the function to
 * perform. The remaining parameters (if any) are interpreted by the
 * operation. The vpi_sim_control definition (now named vpi_control)
 * was added to P1364-2000 14 July 1999. See PLI Task Force ID: PTF-161
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
/************* vpi_control() constants (added with 1364-2000) *************/
#define vpiStop                66  /* execute simulator's $stop */
#define vpiFinish              67  /* execute simulator's $finish */
#define vpiReset               68  /* execute simulator's $reset */
#define vpiSetInteractiveScope 69  /* set simulator's interactive scope */
#define __ivl_legacy_vpiStop 1
#define __ivl_legacy_vpiFinish 2

/* vpi_sim_control is the incorrect name for vpi_control. */
extern void vpi_sim_control(PLI_INT32 operation, ...);

extern vpiHandle  vpi_handle(PLI_INT32 type, vpiHandle ref);
extern vpiHandle  vpi_iterate(PLI_INT32 type, vpiHandle ref);
extern vpiHandle  vpi_scan(vpiHandle iter);
extern vpiHandle  vpi_handle_by_index(vpiHandle ref, PLI_INT32 idx);
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
  These Routines will enable the modpath vpiHandle
  to read/write delay values
*/
extern void vpi_get_delays(vpiHandle expr, p_vpi_delay delays);

extern void vpi_put_delays(vpiHandle expr, p_vpi_delay delays);


/*
 * Check to see if two handles point to the same object.
 */
extern PLI_INT32 vpi_compare_objects(vpiHandle obj1, vpiHandle obj2);


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
extern DLLEXPORT void (*vlog_startup_routines[])(void);


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
extern void vpip_format_strength(char*str, s_vpi_value*value, unsigned bit);
  /* Set the return value to return from the vvp run time. This is
     usually 0 or 1. This is the exit code that the vvp process
     returns, and in distinct from the finish_number that is an
     argument to $fatal and other severity tasks. The $fatal and
     $finish system tasks bundled with iverilog use this function to
     tell vvp to exit SUCCESS or FAILURE. */
extern void vpip_set_return_value(int value);

extern s_vpi_vecval vpip_calc_clog2(vpiHandle arg);
extern void vpip_make_systf_system_defined(vpiHandle ref);

  /* Perform fwrite to mcd files. This is used to write raw data,
     which may include nulls. */
extern void vpip_mcd_rawwrite(PLI_UINT32 mcd, const char*buf, size_t count);

  /* Return driver information for a net bit. The information is returned
     in the 'counts' array as follows:
       counts[0] - number of drivers driving '0' onto the net
       counts[1] - number of drivers driving '1' onto the net
       counts[2] - number of drivers driving 'X' onto the net
       counts[3] - set to 1 if the net is forced, 0 otherwise
     The 'ref' argument should reference a net. The 'idx' argument selects
     which bit of the net is examined. */
extern void vpip_count_drivers(vpiHandle ref, unsigned idx,
                               unsigned counts[4]);

/*
 * Stopgap fix for br916. We need to reject any attempt to pass a thread
 * variable to $strobe or $monitor. To do this, we use some private VPI
 * properties that are normally only used by the VVP thread cleanup code.
 * Normally the following definitions are provided by vvp/vpi_priv.h, but
 * for the stopgap fix we need to make them more widely available.
 */
#define BR916_STOPGAP_FIX
#ifdef BR916_STOPGAP_FIX
#define _vpiFromThr 0x1000001
#   define _vpiNoThr   0
#   define _vpiString  1
#   define _vpiVThr    2
#   define _vpiWord    3
#   define _vpi_at_PV  4
#   define _vpi_at_A   5
#   define _vpi_at_APV 6
#endif

#if defined(__MINGW32__) || defined (__CYGWIN__)
/*
 * In Linux, when loaded, a shared library can automatically bind to functions
 * provided by its client. In Windows, a DLL can only do this statically at
 * link time, and is then tied to a specific client. So to enable VPI modules
 * to be used by both the compiler and the simulator, we construct a jump table
 * for the VPI routines that we can pass down to the VPI modules.
 */

// Increment the version number any time vpip_routines_s is changed.
static const PLI_UINT32 vpip_routines_version = 1;

typedef struct {
    vpiHandle   (*register_cb)(p_cb_data);
    PLI_INT32   (*remove_cb)(vpiHandle);
    vpiHandle   (*register_systf)(const struct t_vpi_systf_data*ss);
    void        (*get_systf_info)(vpiHandle, p_vpi_systf_data);
    vpiHandle   (*handle_by_name)(const char*, vpiHandle);
    vpiHandle   (*handle_by_index)(vpiHandle, PLI_INT32);
    vpiHandle   (*handle)(PLI_INT32, vpiHandle);
    vpiHandle   (*iterate)(PLI_INT32, vpiHandle);
    vpiHandle   (*scan)(vpiHandle);
    PLI_INT32   (*get)(int, vpiHandle);
    char*       (*get_str)(PLI_INT32, vpiHandle);
    void        (*get_delays)(vpiHandle, p_vpi_delay);
    void        (*put_delays)(vpiHandle, p_vpi_delay);
    void        (*get_value)(vpiHandle, p_vpi_value);
    vpiHandle   (*put_value)(vpiHandle, p_vpi_value, p_vpi_time, PLI_INT32);
    void        (*get_time)(vpiHandle, s_vpi_time*);
    void*       (*get_userdata)(vpiHandle);
    PLI_INT32   (*put_userdata)(vpiHandle, void*);
    PLI_UINT32  (*mcd_open)(char *);
    PLI_UINT32  (*mcd_close)(PLI_UINT32);
    PLI_INT32   (*mcd_flush)(PLI_UINT32);
    char*       (*mcd_name)(PLI_UINT32);
    PLI_INT32   (*mcd_vprintf)(PLI_UINT32, const char*, va_list);
    PLI_INT32   (*flush)(void);
    PLI_INT32   (*vprintf)(const char*, va_list);
    PLI_INT32   (*chk_error)(p_vpi_error_info);
    PLI_INT32   (*compare_objects)(vpiHandle, vpiHandle);
    PLI_INT32   (*free_object)(vpiHandle);
    PLI_INT32   (*get_vlog_info)(p_vpi_vlog_info info) ;
    void        (*vcontrol)(PLI_INT32, va_list);
    PLI_INT32   (*fopen)(const char*, const char*);
    FILE*       (*get_file)(PLI_INT32);
    s_vpi_vecval(*calc_clog2)(vpiHandle);
    void        (*count_drivers)(vpiHandle, unsigned, unsigned [4]);
    void        (*format_strength)(char*, s_vpi_value*, unsigned);
    void        (*make_systf_system_defined)(vpiHandle);
    void        (*mcd_rawwrite)(PLI_UINT32, const char*, size_t);
    void        (*set_return_value)(int);
} vpip_routines_s;

extern DLLEXPORT PLI_UINT32 vpip_set_callback(vpip_routines_s*routines, PLI_UINT32 version);

#endif // defined(__MINGW32__) || defined (__CYGWIN__)

EXTERN_C_END

#endif /* VPI_USER_H */
