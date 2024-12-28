/*
 * Copyright (c) 2019-2024 Martin Whitaker (icarus@martin-whitaker.me.uk)
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

#include "config.h"
#include "compiler.h"
#include "vpi_user.h"
#include "sv_vpi_user.h"
#include "vvp/ivl_dlfcn.h"

using namespace std;

/* The only VPI routines that can be legally called when the functions in
   the vlog_startup_routines[] array are executed are vpi_register_systf()
   and vpi_register_cb(), so we can simply provide stubs for the rest. We
   aren't going to execute any callbacks, so we can just provide a stub for
   vpi_register_cb() too.

   Note that the Icarus system module illegally calls vpi_get_vlog_info()
   during startup, so take care to fill in the data structure for that.
*/

// callback related

vpiHandle   vpi_register_cb(p_cb_data) { return 0; }
PLI_INT32   vpi_remove_cb(vpiHandle) { return 0; }

void        vpi_get_systf_info(vpiHandle, p_vpi_systf_data) { }

// for obtaining handles

vpiHandle   vpi_handle_by_name(const char*, vpiHandle) { return 0; }
vpiHandle   vpi_handle_by_index(vpiHandle, PLI_INT32) { return 0; }
vpiHandle   vpi_handle_multi(PLI_INT32, vpiHandle, vpiHandle) { return 0; }

// for traversing relationships

vpiHandle   vpi_handle(PLI_INT32, vpiHandle) { return 0; }
vpiHandle   vpi_iterate(PLI_INT32, vpiHandle) { return 0; }
vpiHandle   vpi_scan(vpiHandle) { return 0; }

// for processing properties

PLI_INT32   vpi_get(int, vpiHandle) { return 0; }
char*       vpi_get_str(PLI_INT32, vpiHandle) { return 0; }

// delay processing

void        vpi_get_delays(vpiHandle, p_vpi_delay) { }
void        vpi_put_delays(vpiHandle, p_vpi_delay) { }

// value processing

void        vpi_get_value(vpiHandle, p_vpi_value) { }
vpiHandle   vpi_put_value(vpiHandle, p_vpi_value, p_vpi_time, PLI_INT32) { return 0; }

// time processing

void        vpi_get_time(vpiHandle, s_vpi_time*) { }

// data processing

void*       vpi_get_userdata(vpiHandle) { return 0; }
PLI_INT32   vpi_put_userdata(vpiHandle, void*) { return 0; }

// I/O routines

PLI_UINT32  vpi_mcd_open(char *) { return 0; }
PLI_UINT32  vpi_mcd_close(PLI_UINT32) { return 0; }
PLI_INT32   vpi_mcd_flush(PLI_UINT32) { return 0; }
char*       vpi_mcd_name(PLI_UINT32) { return 0; }
PLI_INT32   vpi_mcd_printf(PLI_UINT32, const char*, ...) { return 0; }
PLI_INT32   vpi_mcd_vprintf(PLI_UINT32, const char*, va_list) { return 0; }

PLI_INT32   vpi_flush(void) { return 0; }
PLI_INT32   vpi_printf(const char*, ...) { return 0; }
PLI_INT32   vpi_vprintf(const char*, va_list) { return 0; }

// utility routines

PLI_INT32   vpi_chk_error(p_vpi_error_info) { return 0; }
PLI_INT32   vpi_compare_objects(vpiHandle, vpiHandle) { return 0; }
PLI_INT32   vpi_free_object(vpiHandle) { return 0; }
PLI_INT32   vpi_release_handle(vpiHandle) { return 0; }
PLI_INT32   vpi_get_vlog_info(p_vpi_vlog_info info)
{
    info->argc = 0;
    info->argv = 0;
    info->product = 0;
    info->version = 0;
    return 0;
}

// control routines

void        vpi_control(PLI_INT32, ...) { }
void        vpi_sim_control(PLI_INT32, ...) { }

// proposed standard extensions

PLI_INT32   vpi_fopen(const char*, const char*) { return 0; }
FILE*       vpi_get_file(PLI_INT32) { return 0; }

// Icarus extensions

s_vpi_vecval vpip_calc_clog2(vpiHandle)
{
    s_vpi_vecval val = { 0, 0 };
    return val;
}
void        vpip_count_drivers(vpiHandle, unsigned, unsigned [4]) { }
void        vpip_format_strength(char*, s_vpi_value*, unsigned) { }
void        vpip_make_systf_system_defined(vpiHandle) { }
void        vpip_mcd_rawwrite(PLI_UINT32, const char*, size_t) { }
void        vpip_set_return_value(int) { }
void        vpi_vcontrol(PLI_INT32, va_list) { }


/* When a module registers a system function, extract and save the return
   type for use during elaboration. */
vpiHandle vpi_register_systf(const struct t_vpi_systf_data*ss)
{
    if (ss->type != vpiSysFunc)
        return 0;

    struct sfunc_return_type ret_type;
    ret_type.name = ss->tfname;
    switch (ss->sysfunctype) {
      case vpiIntFunc:
        ret_type.type = IVL_VT_LOGIC;
        ret_type.wid  = 32;
        ret_type.signed_flag = true;
        break;
      case vpiRealFunc:
        ret_type.type = IVL_VT_REAL;
        ret_type.wid  = 1;
        ret_type.signed_flag = true;
        break;
      case vpiTimeFunc:
        ret_type.type = IVL_VT_LOGIC;
        ret_type.wid  = 64;
        ret_type.signed_flag = false;
        break;
      case vpiSizedFunc:
        ret_type.type = IVL_VT_LOGIC;
        ret_type.wid  = ss->sizetf ? ss->sizetf(ss->user_data) : 32;
        ret_type.signed_flag = false;
        break;
      case vpiSizedSignedFunc:
        ret_type.type = IVL_VT_LOGIC;
        ret_type.wid  = ss->sizetf ? ss->sizetf(ss->user_data) : 32;
        ret_type.signed_flag = true;
        break;
      case vpiStringFunc:
        ret_type.type = IVL_VT_STRING;
        ret_type.wid  = 0;
        ret_type.signed_flag = false;
        break;
      case vpiOtherFunc:
        ret_type.type = IVL_VT_NO_TYPE;
        ret_type.wid  = 0;
        ret_type.signed_flag = false;
        break;
      default:
        cerr << "warning: " << ss->tfname << " has an unknown return type. "
                "Assuming 32 bit unsigned." << endl;
        ret_type.type = IVL_VT_LOGIC;
        ret_type.wid  = 32;
        ret_type.signed_flag = false;
        break;
    }
    ret_type.override_flag = false;
    add_sys_func(ret_type);
    return 0;
}

#if defined(__MINGW32__) || defined (__CYGWIN__)
vpip_routines_s vpi_routines = {
    .register_cb                = vpi_register_cb,
    .remove_cb                  = vpi_remove_cb,
    .register_systf             = vpi_register_systf,
    .get_systf_info             = vpi_get_systf_info,
    .handle_by_name             = vpi_handle_by_name,
    .handle_by_index            = vpi_handle_by_index,
    .handle_multi               = vpi_handle_multi,
    .handle                     = vpi_handle,
    .iterate                    = vpi_iterate,
    .scan                       = vpi_scan,
    .get                        = vpi_get,
    .get_str                    = vpi_get_str,
    .get_delays                 = vpi_get_delays,
    .put_delays                 = vpi_put_delays,
    .get_value                  = vpi_get_value,
    .put_value                  = vpi_put_value,
    .get_time                   = vpi_get_time,
    .get_userdata               = vpi_get_userdata,
    .put_userdata               = vpi_put_userdata,
    .mcd_open                   = vpi_mcd_open,
    .mcd_close                  = vpi_mcd_close,
    .mcd_flush                  = vpi_mcd_flush,
    .mcd_name                   = vpi_mcd_name,
    .mcd_vprintf                = vpi_mcd_vprintf,
    .flush                      = vpi_flush,
    .vprintf                    = vpi_vprintf,
    .chk_error                  = vpi_chk_error,
    .compare_objects            = vpi_compare_objects,
    .free_object                = vpi_free_object,
    .release_handle             = vpi_release_handle,
    .get_vlog_info              = vpi_get_vlog_info,
    .vcontrol                   = vpi_vcontrol,
    .fopen                      = vpi_fopen,
    .get_file                   = vpi_get_file,
    .calc_clog2                 = vpip_calc_clog2,
    .count_drivers              = vpip_count_drivers,
    .format_strength            = vpip_format_strength,
    .make_systf_system_defined  = vpip_make_systf_system_defined,
    .mcd_rawwrite               = vpip_mcd_rawwrite,
    .set_return_value           = vpip_set_return_value,
};

typedef PLI_UINT32 (*vpip_set_callback_t)(vpip_routines_s*, PLI_UINT32);
#endif
typedef void (*vlog_startup_routines_t)(void);

bool load_vpi_module(const char*path)
{
    ivl_dll_t dll = ivl_dlopen(path, false);
    if (dll == 0) {
	cerr << "error: Failed to open '" << path << "' because:" << endl;
        cerr << "     : " << dlerror() << endl;
	return false;
    }

#if defined(__MINGW32__) || defined (__CYGWIN__)
    void*function = ivl_dlsym(dll, "vpip_set_callback");
    if (function == 0) {
        cerr << "warning: '" << path << "' has no vpip_set_callback()" << endl;
        ivl_dlclose(dll);
        return true;
    }
    vpip_set_callback_t set_callback = (vpip_set_callback_t)function;
    if (!set_callback(&vpi_routines, vpip_routines_version)) {
        cerr << "error: Failed to link '" << path << "'. "
                "Try rebuilding it with iverilog-vpi." << endl;
        ivl_dlclose(dll);
        return true;
    }
#endif

    void*table = ivl_dlsym(dll, LU "vlog_startup_routines" TU);
    if (table == 0) {
        cerr << "warning: '" << path << "' has no vlog_startup_routines" << endl;
        ivl_dlclose(dll);
        return true;
    }

    vlog_startup_routines_t*routines = (vlog_startup_routines_t*)table;
    for (unsigned idx = 0; routines[idx]; idx += 1) {
        (routines[idx])();
    }

    ivl_dlclose(dll);
    return true;
}
