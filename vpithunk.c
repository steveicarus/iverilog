/*
 * Copyright (c) 2001-2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpithunk.c,v 1.7 2002/12/11 23:55:22 steve Exp $"
#endif

#include <stdio.h>
#include <stdarg.h>
#include "vpi_user.h"
#include "vpithunk.h"

/*
 * This code is linked into the VPI module, not the simulator. The
 * module uses the symbols defined in here to call implementations
 * supplied by the simulator, which loaded this module with dlopen or
 * the equivilent.
 *
 * The vpi_thunk_p pointer points to a table of function pointers that
 * point to all the functions that a simulator is expected to provide.
 * The vlog_register_sim is a function that the simulator is expected
 * to call with a value for the vpi_thunk_p pointer. This is how the
 * run time linkage to all the functions in the make program are made.
 *
 * In Icarus Verilog, the VPI module is also supposed to export a
 * vlog_startup_routines table. This is something that the programmer
 * does, so the libvpi library, and the vlog_register_sim function,
 * are invisible to the user.
 *
 * The simulator is careful to call the vpi_register_sim function from
 * a loaded module before executing the startup routines.
 */
static p_vpi_thunk vpi_thunk_p = 0;

#define VPITV_CALL(fn,args) {						\
  if (vpi_thunk_p == 0) {						\
    no_sim(#fn); return;						\
  }									\
  if (vpi_thunk_p->fn == 0) {						\
    no_func(#fn); return;						\
  }									\
  vpi_thunk_p->fn args;							\
}

#define VPIT_CALL(fn,def,args) {					\
  if (vpi_thunk_p == 0) {						\
    no_sim(#fn); return def;						\
  }									\
  if (vpi_thunk_p->fn == 0) {						\
    no_func(#fn); return def;						\
  }									\
  return vpi_thunk_p->fn args;						\
}

static void no_sim(const char *fn)
{
	fprintf(stderr, "No Simulator registered, cannot handle vpi "
            "call to %s\n", fn);
}

static void no_func(const char *fn)
{
	fprintf(stderr, "Simulator doesn't have a %s function\n",fn);
}

DLLEXPORT int vpi_register_sim(p_vpi_thunk tp)
{
  vpi_thunk_p = 0;
  if (tp->magic != VPI_THUNK_MAGIC) {
    fprintf(stderr, "Thunk Magic Number mismatch. Got %08X, expected %08x\n",
	    tp->magic, VPI_THUNK_MAGIC);
    return 0;
  }
  vpi_thunk_p = tp;
  return 1;
}

void vpi_register_systf(const struct t_vpi_systf_data *ss)
{
  VPITV_CALL(vpi_register_systf,(ss));
}

void vpi_printf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  VPITV_CALL(vpi_vprintf,(fmt,ap));
  va_end(ap);
}

unsigned int vpi_mcd_close(unsigned int mcd)
{
  VPIT_CALL(vpi_mcd_close,-1,(mcd));
}


char *vpi_mcd_name(unsigned int mcd) 
{
  VPIT_CALL(vpi_mcd_name,0,(mcd));
}

unsigned int vpi_mcd_open(char *name)
{
  VPIT_CALL(vpi_mcd_open,-1,(name));
}

unsigned int vpi_mcd_open_x(char *name, char *mode)
{
  VPIT_CALL(vpi_mcd_open_x,-1, (name, mode));
}

int vpi_mcd_printf(unsigned int mcd, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  VPIT_CALL(vpi_mcd_vprintf,0,(mcd,fmt,ap));
  va_end(ap);			/* bad - never hit */
}

int vpi_mcd_fputc(unsigned int mcd, unsigned char x)
{
  VPIT_CALL(vpi_mcd_fputc, 0, (mcd, x));
}

int vpi_mcd_fgetc(unsigned int mcd)
{
  VPIT_CALL(vpi_mcd_fgetc, -1, (mcd));
}

extern vpiHandle vpi_register_cb(p_cb_data data)
{
  VPIT_CALL(vpi_register_cb,0,(data));
}

extern int vpi_remove_cb(vpiHandle ref)
{
  VPIT_CALL(vpi_remove_cb,0,(ref));
}

extern void vpi_sim_control(int operation, ...)
{
  va_list ap;
  va_start(ap, operation);
  VPITV_CALL(vpi_sim_vcontrol,(operation,ap));
  va_end(ap);			/* bad - never hit */
}

extern void vpi_control(int operation, ...)
{
  va_list ap;
  va_start(ap, operation);
  VPITV_CALL(vpi_sim_vcontrol,(operation,ap));
  va_end(ap);			/* bad - never hit */
}

extern vpiHandle  vpi_handle(int type, vpiHandle ref)
{
  VPIT_CALL(vpi_handle, 0, (type, ref));
}

extern vpiHandle  vpi_iterate(int type, vpiHandle ref)
{
  VPIT_CALL(vpi_iterate, 0, (type, ref));
}

extern vpiHandle  vpi_scan(vpiHandle iter)
{
  VPIT_CALL(vpi_scan, 0, (iter));
}

extern vpiHandle  vpi_handle_by_index(vpiHandle ref, int index)
{
  VPIT_CALL(vpi_handle_by_index,0,(ref, index));
}

extern vpiHandle  vpi_handle_by_name(char *name, vpiHandle scope)
{
      VPIT_CALL(vpi_handle_by_name,0,(name, scope));
}

extern void  vpi_get_time(vpiHandle obj, s_vpi_time*t)
{
  VPITV_CALL(vpi_get_time, (obj,t));
}

extern int   vpi_get(int property, vpiHandle ref)
{
  VPIT_CALL(vpi_get, 0, (property, ref));

}

extern char* vpi_get_str(int property, vpiHandle ref)
{
  VPIT_CALL(vpi_get_str, 0, (property,ref));
}

extern void  vpi_get_value(vpiHandle expr, p_vpi_value value)
{
  VPITV_CALL(vpi_get_value, (expr, value));
}

extern vpiHandle vpi_put_value(vpiHandle obj, p_vpi_value value,
			       p_vpi_time when, int flags)
{
  VPIT_CALL(vpi_put_value, 0, (obj, value, when, flags));
}

extern int vpi_free_object(vpiHandle ref)
{
  VPIT_CALL(vpi_free_object, -1, (ref));
}

extern int vpi_get_vlog_info(p_vpi_vlog_info vlog_info_p)
{
  VPIT_CALL(vpi_get_vlog_info, 0, (vlog_info_p));
}

extern int vpi_chk_error(p_vpi_error_info info)
{
  VPIT_CALL(vpi_chk_error, 0, (info));
}

/*
 * $Log: vpithunk.c,v $
 * Revision 1.7  2002/12/11 23:55:22  steve
 *  Add vpi_handle_by_name to the VPI interface,
 *  and bump the vpithunk magic number.
 *
 * Revision 1.6  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 */
