/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_modules.cc,v 1.1 2001/03/16 01:44:34 steve Exp $"
#endif

# include  "config.h"
# include  "vpi_priv.h"
# include  "ivl_dlfcn.h"
# include  <stdio.h>

typedef void (*vlog_startup_routines_t)(void);

void vpip_load_modules(unsigned cnt, const char*mod[], const char*path)
{
      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
	    char buf[4096];
	    sprintf(buf, "%s/%s.vpi", path, mod[idx]);
	    printf("Load %s...\n", buf);

	    ivl_dll_t dll = ivl_dlopen(buf);
	    if (dll == 0) {
		  fprintf(stderr, "%s: %s\n", mod[idx], dlerror());
		  continue;
	    }

	    void*table = ivl_dlsym(dll, LU "vlog_startup_routines" TU);
	    if (table == 0) {
		  fprintf(stderr, "%s: no vlog_startup_routines\n", mod[idx]);
		  ivl_dlclose(dll);
		  continue;
	    }

	    vlog_startup_routines_t*routines = (vlog_startup_routines_t*)table;
	    for (unsigned tmp = 0 ;  routines[tmp] ;  tmp += 1)
		  (routines[tmp])();

      }
}

/*
 * $Log: vpi_modules.cc,v $
 * Revision 1.1  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 */

