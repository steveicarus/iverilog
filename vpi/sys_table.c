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
#ident "$Id: sys_table.c,v 1.14 2002/03/09 21:54:49 steve Exp $"
#endif

# include "config.h"
#include "vpi_user.h"

extern void sys_finish_register();
extern void sys_deposit_register();
extern void sys_display_register();
extern void sys_random_register();
extern void sys_readmem_register();
extern void sys_time_register();
extern void sys_vcd_register();
extern void sys_lxt_register();

void (*vlog_startup_routines[])() = {
      sys_finish_register,
      sys_deposit_register,
      sys_display_register,
      sys_random_register,
      sys_readmem_register,
      sys_time_register,
      sys_vcd_register,
      sys_lxt_register,
      0
};


/*
 * $Log: sys_table.c,v $
 * Revision 1.14  2002/03/09 21:54:49  steve
 *  Add LXT dumper support. (Anthony Bybell)
 *
 * Revision 1.13  2001/09/30 16:45:10  steve
 *  Fix some Cygwin DLL handling. (Venkat Iyer)
 *
 * Revision 1.12  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.11  2001/05/22 02:14:47  steve
 *  Update the mingw build to not require cygwin files.
 *
 * Revision 1.10  2001/05/20 15:09:40  steve
 *  Mingw32 support (Venkat Iyer)
 *
 * Revision 1.9  2001/04/26 00:01:33  steve
 *  Support $deposit to a wire or reg.
 *
 * Revision 1.8  2000/12/14 23:36:34  steve
 *  include vpi_user.h.
 *
 * Revision 1.7  2000/11/01 03:19:36  steve
 *  Add the general $time system function.
 *
 * Revision 1.6  2000/09/30 03:20:47  steve
 *  Cygwin port changes from Venkat
 *
 * Revision 1.5  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.4  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.3  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 * Revision 1.2  1999/11/07 20:33:30  steve
 *  Add VCD output and related system tasks.
 *
 * Revision 1.1  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 */

