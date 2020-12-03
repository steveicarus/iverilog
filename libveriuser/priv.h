#ifndef IVL_priv_H
#define IVL_priv_H
/*
 * Copyright (c) 2003-2020 Stephen Williams (steve@icarus.com)
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

# include  <stdio.h>
# include  "vpi_user.h"

/*
 * The VPI handle for the current task/function instance. This is the
 * handle returned by vpi_handle(vpiSysTfCall, 0) when the task/function
 * is compiled or called, but we also need it when executing a callback.
 */
extern vpiHandle cur_instance;

/*
 * This function implements the acc_ string buffer, by adding the
 * input string to the buffer, and returning a pointer to the first
 * character of the new string.
 */
extern char* __acc_newstring(const char*txt);

/*
 * Trace file for logging ACC and TF calls.
 */
extern FILE* pli_trace;

#endif /* IVL_priv_H */
