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
#ident "$Id: vpi_modules.cc,v 1.7 2001/07/28 03:29:42 steve Exp $"
#endif

# include  "config.h"
# include  "vpi_priv.h"
# include  "ivl_dlfcn.h"
# include  "vpithunk.h"
# include  <stdio.h>
# include  <string.h>

typedef void (*vlog_startup_routines_t)(void);
typedef int (*vpi_register_sim_t)(p_vpi_thunk tp);


char* vpip_module_path[64] = {
      MODULE_DIR,
      0
};

unsigned vpip_module_path_cnt = 1;


void vpip_load_module(const char*name)
{
#ifdef __MINGW32__
      const char sep = '\\';
#else
      const char sep = '/';
#endif

      ivl_dll_t dll = 0;

      if (strchr(name, sep)) {
	      /* If the name has at least one directory character in
		 it, then assume it is a complete name, including any
		 possble .vpi suffix. */
	    dll = ivl_dlopen(name);

	    if (dll == 0) {
		  fprintf(stderr, "%s: Unable to link this module\n", name);
		  return;
	    }

      } else {
	    for (unsigned idx = 0
		       ; (dll == 0) && (idx < vpip_module_path_cnt)
		       ;  idx += 1) {
		  char buf[4096];
		  sprintf(buf, "%s%c%s.vpi", vpip_module_path[idx], sep, name);

		  dll = ivl_dlopen(buf);
	    }

	    if (dll == 0) {
		  fprintf(stderr, "%s: Unable to find a "
			  "%s.vpi module\n", name, name); 
		  return;
	    }

      }

      void *regsub = ivl_dlsym(dll, LU "vpi_register_sim" TU);
      vpi_register_sim_t simreg = (vpi_register_sim_t)regsub;
      if (regsub == 0) {
	    fprintf(stderr, "%s: Unable to locate vpi_register_sim", name);
	    ivl_dlclose(dll);
	    return;
      }

      extern vpi_thunk vvpt;
      if (((simreg)(&vvpt)) == 0) {
	fprintf(stderr, "%s: vpi_register_sim returned zero", name);
	ivl_dlclose(dll);
	return;
      }


      void*table = ivl_dlsym(dll, LU "vlog_startup_routines" TU);
      if (table == 0) {
	    fprintf(stderr, "%s: no vlog_startup_routines\n", name);
	    ivl_dlclose(dll);
	    return;
      }

      vlog_startup_routines_t*routines = (vlog_startup_routines_t*)table;
      for (unsigned tmp = 0 ;  routines[tmp] ;  tmp += 1)
	    (routines[tmp])();

}

/*
 * $Log: vpi_modules.cc,v $
 * Revision 1.7  2001/07/28 03:29:42  steve
 *  If module name has a /, skip the path search.
 *
 * Revision 1.6  2001/07/26 03:13:51  steve
 *  Make the -M flag add module search paths.
 *
 * Revision 1.5  2001/06/12 03:53:11  steve
 *  Change the VPI call process so that loaded .vpi modules
 *  use a function table instead of implicit binding.
 *
 * Revision 1.4  2001/05/22 02:14:47  steve
 *  Update the mingw build to not require cygwin files.
 *
 * Revision 1.3  2001/03/23 02:40:22  steve
 *  Add the :module header statement.
 *
 * Revision 1.2  2001/03/22 05:39:34  steve
 *  Test print that interferes with output.
 *
 * Revision 1.1  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 */

