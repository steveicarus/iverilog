#ifndef _VPI_THUNK_H_
#define _VPI_THUNK_H_ 1

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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vpithunk.h,v 1.3 2002/08/11 23:47:04 steve Exp $"
#endif

/* These functions are actually defined in lieu of the vpi functions
   by the simulator.  These prototypes should'nt go into vpi_user.h, 
   because we don't want the users to be seeing this stuff.  They
   are non-standard.  We have to put them here, so that 
   including vpi_user.h doesn't require including stdarg.h */

EXTERN_C_START

# include  <stdarg.h>
extern void vpi_vprintf(const char*fmt, va_list ap);
extern int vpi_mcd_vprintf(unsigned int mcd, const char*fmt, va_list ap);
extern void vpi_sim_vcontrol(int operation, va_list ap);

EXTERN_C_END

#define VPI_THUNK_MAGIC  (0x87836BA4)

typedef struct {
  int magic;
  void (*vpi_register_systf)(const struct t_vpi_systf_data*ss);
  void (*vpi_vprintf)(const char*fmt, va_list ap);
  unsigned int (*vpi_mcd_close)(unsigned int mcd);
  char *(*vpi_mcd_name)(unsigned int mcd);
  unsigned int (*vpi_mcd_open)(char *name);
  unsigned int (*vpi_mcd_open_x)(char *name, char *mode);
  int (*vpi_mcd_vprintf)(unsigned int mcd, const char*fmt, va_list ap);
  int (*vpi_mcd_fputc)(unsigned int mcd, unsigned char x);
  int (*vpi_mcd_fgetc)(unsigned int mcd);
  vpiHandle (*vpi_register_cb)(p_cb_data data);
  int (*vpi_remove_cb)(vpiHandle ref);
  void (*vpi_sim_vcontrol)(int operation, va_list ap);
  vpiHandle (*vpi_handle)(int type, vpiHandle ref);
  vpiHandle (*vpi_iterate)(int type, vpiHandle ref);
  vpiHandle (*vpi_scan)(vpiHandle iter);
  vpiHandle (*vpi_handle_by_index)(vpiHandle ref, int index);
  void (*vpi_get_time)(vpiHandle obj, s_vpi_time*t);
  int (*vpi_get)(int property, vpiHandle ref);
  char* (*vpi_get_str)(int property, vpiHandle ref);
  void (*vpi_get_value)(vpiHandle expr, p_vpi_value value);
  vpiHandle (*vpi_put_value)(vpiHandle obj, p_vpi_value value,
				     p_vpi_time when, int flags);
  int (*vpi_free_object)(vpiHandle ref);
  int (*vpi_get_vlog_info)(p_vpi_vlog_info vlog_info_p);
  int (*vpi_chk_error)(p_vpi_error_info info);
} vpi_thunk, *p_vpi_thunk;

DLLEXPORT int vpi_register_sim(p_vpi_thunk tp);

#endif

/*
 * $Log: vpithunk.h,v $
 * Revision 1.3  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 */
