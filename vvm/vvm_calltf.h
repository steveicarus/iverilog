#ifndef __vvm_vvm_calltf_H
#define __vvm_vvm_calltf_H
/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: vvm_calltf.h,v 1.2 1999/08/15 01:23:56 steve Exp $"
#endif

# include  "vvm.h"
# include  <string>

typedef struct __vpiHandle*vpiHandle;

/*
 * This function loads a vpi module by name.
 */
extern void vvm_load_vpi_module(const char*path);


/*
 * The vvm_init_vpi_handle functions initialize statis VPI handles
 * that may be used many places within the program.
 */
extern void vvm_init_vpi_handle(vpiHandle, vvm_bits_t*, vvm_monitor_t*);
extern void vvm_init_vpi_timevar(vpiHandle, const char*name);

/*
 * The vvm_make_vpi_parm functions initialize the vpiHandles to
 * represent objects suitable for use as vvm_calltask parameters.
 */
extern void vvm_make_vpi_parm(vpiHandle ref, const char*val);
extern void vvm_make_vpi_parm(vpiHandle ref, vvm_bits_t*val);
extern void vvm_make_vpi_parm(vpiHandle ref);

/*
 * The vvm environment supports external calls to C++ by
 * vvm_calltask. The code generator generates calls to vvm_calltask
 * that corresponds to the system call in the Verilog source. The
 * vvm_calltask in turn locates the vpi implementation (by name) and
 * calls the VPI that implements the task.
 */

extern void vvm_calltask(vvm_simulation*sim, const string&name,
			 unsigned nparms, vpiHandle*parms);

/*
 * $Log: vvm_calltf.h,v $
 * Revision 1.2  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */
#endif
