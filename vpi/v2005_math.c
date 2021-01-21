/*
 *  Verilog-2005 math library for Icarus Verilog
 *  http://www.icarus.com/eda/verilog/
 *
 *  Copyright (C) 2007-2021  Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "vpi_config.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "vpi_user.h"
#include "ivl_alloc.h"

/* Single argument functions. */
typedef struct s_single_data {
    const char *name;
    double (*func)(double);
} t_single_data;

static t_single_data va_single_data[]= {
    {"$sqrt",  sqrt},
    {"$ln",    log},
    {"$log10", log10},
    {"$exp",   exp},
    {"$ceil",  ceil},
    {"$floor", floor},
    {"$sin",   sin},
    {"$cos",   cos},
    {"$tan",   tan},
    {"$asin",  asin},
    {"$acos",  acos},
    {"$atan",  atan},
    {"$sinh",  sinh},
    {"$cosh",  cosh},
    {"$tanh",  tanh},
    {"$asinh", asinh},
    {"$acosh", acosh},
    {"$atanh", atanh},
    {0, 0}  /* Must be NULL terminated! */
};


/* Double argument functions. */
typedef struct s_double_data {
    const char *name;
    double (*func)(double, double);
} t_double_data;

static t_double_data va_double_data[]= {
    {"$pow",   pow},
    {"$atan2", atan2},
    {"$hypot", hypot},
    {0, 0}  /* Must be NULL terminated! */
};


/*
 * This structure holds the single argument information.
 */
typedef struct {
    vpiHandle arg;
    double (*func)(double);
} va_single_t;


/*
 * This structure holds the double argument information.
 */
typedef struct {
    vpiHandle arg1;
    vpiHandle arg2;
    double (*func)(double, double);
} va_double_t;


/*
 * Cleanup the allocated memory at the end of simulation.
 */
static va_single_t** single_funcs = 0;
static unsigned single_funcs_count = 0;
static va_double_t** double_funcs = 0;
static unsigned double_funcs_count = 0;

static PLI_INT32 sys_end_of_simulation(p_cb_data cb_data)
{
    unsigned idx;

    (void)cb_data; /* Parameter is not used. */

    for (idx = 0; idx < single_funcs_count; idx += 1) {
        free(single_funcs[idx]);
    }
    free(single_funcs);
    single_funcs = 0;
    single_funcs_count = 0;

    for (idx = 0; idx < double_funcs_count; idx += 1) {
        free(double_funcs[idx]);
    }
    free(double_funcs);
    double_funcs = 0;
    double_funcs_count = 0;

    return 0;
}


/*
 * Standard error message routine. The format string must take one
 * string argument (the name of the function).
 */
static void va_error_message(vpiHandle callh, const char *format,
                             const char *name) {
    vpi_printf("%s:%d: error: ", vpi_get_str(vpiFile, callh),
               (int)vpi_get(vpiLineNo, callh));
    vpi_printf(format, name);
    vpip_set_return_value(1);
    vpi_control(vpiFinish, 1);
}


/*
 * Process an argument.
 */
static vpiHandle va_process_argument(vpiHandle callh, const char *name,
                              vpiHandle arg, const char *post) {
    PLI_INT32 type;

    if (arg == NULL) return 0;
    type = vpi_get(vpiType, arg);
    /* Math function cannot do anything with a string. */
    if ((type == vpiConstant || type == vpiParameter) &&
        (vpi_get(vpiConstType, arg) == vpiStringConst)) {
        const char* basemsg = "%s cannot process strings";
        char* msg = malloc(strlen(basemsg)+strlen(post)+3);
        strcpy(msg, basemsg);
        strcat(msg, post);
        strcat(msg, ".\n");
        va_error_message(callh, msg, name);
        free(msg);
        return 0;
    }
    return arg;
}


/*
 * Routine to check all the single argument math functions.
 */
static PLI_INT32 va_single_argument_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *ud)
{
    vpiHandle callh, argv, arg;
    const t_single_data *data;
    const char *name;
    va_single_t* fun_data;

    assert(ud != 0);
    callh = vpi_handle(vpiSysTfCall, 0);
    assert(callh != 0);
    argv = vpi_iterate(vpiArgument, callh);
    data = (const t_single_data *) ud;
    name = data->name;

    fun_data = malloc(sizeof(va_single_t));

    /* Check that there are arguments. */
    if (argv == 0) {
        va_error_message(callh, "%s requires one argument.\n", name);
        free(fun_data);
        return 0;
    }

    /* In Icarus if we have an argv we have at least one argument. */
    arg = vpi_scan(argv);
    fun_data->arg = va_process_argument(callh, name, arg, "");

    /* These functions only take one argument. */
    arg = vpi_scan(argv);
    if (arg != 0) {
        va_error_message(callh, "%s takes only one argument.\n", name);
        vpi_free_object(argv);
    }

    /* Get the function that is to be used by the calltf routine. */
    fun_data->func = data->func;

    vpi_put_userdata(callh, fun_data);
    single_funcs_count += 1;
    single_funcs = (va_single_t **)realloc(single_funcs,
                   single_funcs_count*sizeof(va_single_t **));
    single_funcs[single_funcs_count-1] = fun_data;

    /* vpi_scan() returning 0 (NULL) has already freed argv. */
    return 0;
}


/*
 * Routine to implement the single argument math functions.
 */
static PLI_INT32 va_single_argument_calltf(ICARUS_VPI_CONST PLI_BYTE8 *ud)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    s_vpi_value val;
    va_single_t* fun_data;

    (void)ud; /* Parameter is not used. */

    /* Retrieve the function and argument data. */
    fun_data = vpi_get_userdata(callh);

    /* Calculate the result */
    val.format = vpiRealVal;
    vpi_get_value(fun_data->arg, &val);
    val.value.real = (fun_data->func)(val.value.real);

    /* Return the result */
    vpi_put_value(callh, &val, 0, vpiNoDelay);

    return 0;
}


/*
 * Routine to check all the double argument math functions.
 */
static PLI_INT32 va_double_argument_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *ud)
{
    vpiHandle callh, argv, arg;
    const t_double_data *data;
    const char *name;
    va_double_t* fun_data;

    assert(ud != 0);
    callh = vpi_handle(vpiSysTfCall, 0);
    assert(callh != 0);
    argv = vpi_iterate(vpiArgument, callh);
    data = (const t_double_data *) ud;
    name = data->name;

    fun_data = malloc(sizeof(va_double_t));

    /* Check that there are arguments. */
    if (argv == 0) {
        va_error_message(callh, "%s requires two arguments.\n", name);
        free(fun_data);
        return 0;
    }

    /* In Icarus if we have an argv we have at least one argument. */
    arg = vpi_scan(argv);
    fun_data->arg1 = va_process_argument(callh, name, arg, " (arg1)");

    /* Check that there are at least two arguments. */
    arg = vpi_scan(argv);
    if (arg == 0) {
        va_error_message(callh, "%s requires two arguments.\n", name);
    }
    fun_data->arg2 = va_process_argument(callh, name, arg, " (arg2)");

    /* These functions only take two arguments. */
    arg = vpi_scan(argv);
    if (arg != 0) {
        va_error_message(callh, "%s takes only two arguments.\n", name);
        vpi_free_object(argv);
    }

    /* Get the function that is to be used by the calltf routine. */
    fun_data->func = data->func;

    vpi_put_userdata(callh, fun_data);
    double_funcs_count += 1;
    double_funcs = (va_double_t **)realloc(double_funcs,
                   double_funcs_count*sizeof(va_double_t **));
    double_funcs[double_funcs_count-1] = fun_data;

    /* vpi_scan() returning 0 (NULL) has already freed argv. */
    return 0;
}


/*
 * Routine to implement the double argument math functions.
 */
static PLI_INT32 va_double_argument_calltf(ICARUS_VPI_CONST PLI_BYTE8 *ud)
{
    vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
    s_vpi_value val;
    double first_arg;
    va_double_t* fun_data;

    (void)ud; /* Parameter is not used. */

    /* Retrieve the function and argument data. */
    fun_data = vpi_get_userdata(callh);

    /* Calculate the result */
    val.format = vpiRealVal;
    vpi_get_value(fun_data->arg1, &val);
    first_arg = val.value.real;
    vpi_get_value(fun_data->arg2, &val);
    val.value.real = (fun_data->func)(first_arg, val.value.real);

    /* Return the result */
    vpi_put_value(callh, &val, 0, vpiNoDelay);

    return 0;
}


/*
 * Register all the functions with Verilog.
 */
static void sys_v2005_math_register(void)
{
    s_cb_data cb_data;
    s_vpi_systf_data tf_data;
    vpiHandle res;
    unsigned idx;

    /* Register the single argument functions. */
    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiRealFunc;
    tf_data.calltf      = va_single_argument_calltf;
    tf_data.compiletf   = va_single_argument_compiletf;
    tf_data.sizetf      = 0;

    for (idx=0; va_single_data[idx].name != 0; idx++) {
        tf_data.tfname    = va_single_data[idx].name;
        tf_data.user_data = (PLI_BYTE8 *) &va_single_data[idx];
        res = vpi_register_systf(&tf_data);
        vpip_make_systf_system_defined(res);
    }

    /* Register the double argument functions. */
    tf_data.type        = vpiSysFunc;
    tf_data.sysfunctype = vpiRealFunc;
    tf_data.calltf      = va_double_argument_calltf;
    tf_data.compiletf   = va_double_argument_compiletf;
    tf_data.sizetf      = 0;

    for (idx=0; va_double_data[idx].name != 0; idx++) {
        tf_data.tfname    = va_double_data[idx].name;
        tf_data.user_data = (PLI_BYTE8 *) &va_double_data[idx];
        res = vpi_register_systf(&tf_data);
        vpip_make_systf_system_defined(res);
    }

    /* We need to clean up the userdata. */
    cb_data.reason = cbEndOfSimulation;
    cb_data.time = 0;
    cb_data.cb_rtn = sys_end_of_simulation;
    cb_data.user_data = "system";
    vpi_register_cb(&cb_data);
}


/*
 * Hook to get Icarus Verilog to find the registration function.
 */
extern void sys_clog2_register(void);

void (*vlog_startup_routines[])(void) = {
    sys_v2005_math_register,
    sys_clog2_register,
    0
};
