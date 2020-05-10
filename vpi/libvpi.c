/*
 * Copyright (c) 2019 Martin Whitaker (icarus@martin-whitaker.me.uk)
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

#include "vpi_user.h"
#include <assert.h>

static vpip_routines_s*vpip_routines = 0;

// callback related

vpiHandle vpi_register_cb(p_cb_data data)
{
      assert(vpip_routines);
      return vpip_routines->register_cb(data);
}
PLI_INT32 vpi_remove_cb(vpiHandle ref)
{
      assert(vpip_routines);
      return vpip_routines->remove_cb(ref);
}

vpiHandle vpi_register_systf(const struct t_vpi_systf_data*ss)
{
      assert(vpip_routines);
      return vpip_routines->register_systf(ss);
}
void vpi_get_systf_info(vpiHandle obj, p_vpi_systf_data data)
{
      assert(vpip_routines);
      vpip_routines->get_systf_info(obj, data);
}

// for obtaining handles

vpiHandle vpi_handle_by_name(const char*name, vpiHandle scope)
{
      assert(vpip_routines);
      return vpip_routines->handle_by_name(name, scope);
}
vpiHandle vpi_handle_by_index(vpiHandle ref, PLI_INT32 idx)
{
      assert(vpip_routines);
      return vpip_routines->handle_by_index(ref, idx);
}

// for traversing relationships

vpiHandle vpi_handle(PLI_INT32 type, vpiHandle ref)
{
      assert(vpip_routines);
      return vpip_routines->handle(type, ref);
}
vpiHandle vpi_iterate(PLI_INT32 type, vpiHandle ref)
{
      assert(vpip_routines);
      return vpip_routines->iterate(type, ref);
}
vpiHandle vpi_scan(vpiHandle iter)
{
      assert(vpip_routines);
      return vpip_routines->scan(iter);
}

// for processing properties

PLI_INT32 vpi_get(int property, vpiHandle ref)
{
      assert(vpip_routines);
      return vpip_routines->get(property, ref);
}
char*vpi_get_str(PLI_INT32 property, vpiHandle ref)
{
      assert(vpip_routines);
      return vpip_routines->get_str(property, ref);
}

// delay processing

void vpi_get_delays(vpiHandle expr, p_vpi_delay delays)
{
      assert(vpip_routines);
      vpip_routines->get_delays(expr, delays);
}
void vpi_put_delays(vpiHandle expr, p_vpi_delay delays)
{
      assert(vpip_routines);
      vpip_routines->put_delays(expr, delays);
}

// value processing

void vpi_get_value(vpiHandle expr, p_vpi_value value)
{
      assert(vpip_routines);
      vpip_routines->get_value(expr, value);
}
vpiHandle vpi_put_value(vpiHandle obj, p_vpi_value value, p_vpi_time when, PLI_INT32 flags)
{
      assert(vpip_routines);
      return vpip_routines->put_value(obj, value, when, flags);
}

// time processing

void vpi_get_time(vpiHandle obj, s_vpi_time*t)
{
      assert(vpip_routines);
      vpip_routines->get_time(obj, t);
}

// data processing

void*vpi_get_userdata(vpiHandle obj)
{
      assert(vpip_routines);
      return vpip_routines->get_userdata(obj);
}
PLI_INT32 vpi_put_userdata(vpiHandle obj, void*data)
{
      assert(vpip_routines);
      return vpip_routines->put_userdata(obj, data);
}

// I/O routines

PLI_UINT32 vpi_mcd_open(char*name)
{
      assert(vpip_routines);
      return vpip_routines->mcd_open(name);
}
PLI_UINT32 vpi_mcd_close(PLI_UINT32 mcd)
{
      assert(vpip_routines);
      return vpip_routines->mcd_close(mcd);
}
PLI_INT32 vpi_mcd_flush(PLI_UINT32 mcd)
{
      assert(vpip_routines);
      return vpip_routines->mcd_flush(mcd);
}
char*vpi_mcd_name(PLI_UINT32 mcd)
{
      assert(vpip_routines);
      return vpip_routines->mcd_name(mcd);
}
PLI_INT32 vpi_mcd_printf(PLI_UINT32 mcd, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      assert(vpip_routines);
      PLI_INT32 rv = vpip_routines->mcd_vprintf(mcd, fmt, ap);
      va_end(ap);
      return rv;
}
PLI_INT32 vpi_mcd_vprintf(PLI_UINT32 mcd, const char*fmt, va_list ap)
{
      assert(vpip_routines);
      return vpip_routines->mcd_vprintf(mcd, fmt, ap);
}

PLI_INT32 vpi_flush(void)
{
      assert(vpip_routines);
      return vpip_routines->flush();
}
PLI_INT32 vpi_printf(const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      assert(vpip_routines);
      PLI_INT32 rv = vpip_routines->vprintf(fmt, ap);
      va_end(ap);
      return rv;
}
PLI_INT32 vpi_vprintf(const char*fmt, va_list ap)
{
      assert(vpip_routines);
      return vpip_routines->vprintf(fmt, ap);
}

// utility routines

PLI_INT32 vpi_chk_error(p_vpi_error_info info)
{
      assert(vpip_routines);
      return vpip_routines->chk_error(info);
}
PLI_INT32 vpi_compare_objects(vpiHandle obj1, vpiHandle obj2)
{
      assert(vpip_routines);
      return vpip_routines->compare_objects(obj1, obj2);
}
PLI_INT32 vpi_free_object(vpiHandle ref)
{
      assert(vpip_routines);
      return vpip_routines->free_object(ref);
}
PLI_INT32 vpi_get_vlog_info(p_vpi_vlog_info info)
{
      assert(vpip_routines);
      return vpip_routines->get_vlog_info(info);

}

// control routines

void vpi_control(PLI_INT32 operation, ...)
{
      va_list ap;
      va_start(ap, operation);
      assert(vpip_routines);
      vpip_routines->vcontrol(operation, ap);
      va_end(ap);
}
void vpi_sim_control(PLI_INT32 operation, ...)
{
      va_list ap;
      va_start(ap, operation);
      assert(vpip_routines);
      vpip_routines->vcontrol(operation, ap);
      va_end(ap);
}

// proposed standard extensions

PLI_INT32 vpi_fopen(const char*name, const char*mode)
{
      assert(vpip_routines);
      return vpip_routines->fopen(name, mode);
}
FILE*vpi_get_file(PLI_INT32 fd)
{
      assert(vpip_routines);
      return vpip_routines->get_file(fd);
}

// Icarus extensions

s_vpi_vecval vpip_calc_clog2(vpiHandle arg)
{
      assert(vpip_routines);
      return vpip_routines->calc_clog2(arg);
}
void vpip_count_drivers(vpiHandle ref, unsigned idx, unsigned counts[4])
{
      assert(vpip_routines);
      vpip_routines->count_drivers(ref, idx, counts);
}
void vpip_format_strength(char*str, s_vpi_value*value, unsigned bit)
{
      assert(vpip_routines);
      vpip_routines->format_strength(str, value, bit);
}
void vpip_make_systf_system_defined(vpiHandle ref)
{
      assert(vpip_routines);
      vpip_routines->make_systf_system_defined(ref);
}
void vpip_mcd_rawwrite(PLI_UINT32 mcd, const char*buf, size_t count)
{
      assert(vpip_routines);
      vpip_routines->mcd_rawwrite(mcd, buf, count);
}
void vpip_set_return_value(int value)
{
      assert(vpip_routines);
      vpip_routines->set_return_value(value);
}

DLLEXPORT PLI_UINT32 vpip_set_callback(vpip_routines_s*routines, PLI_UINT32 version)
{
      if (version != vpip_routines_version)
            return 0;

      vpip_routines = routines;
      return 1;
}

#else

void __libvpi_c_dummy_function(void)
{
}

#endif
