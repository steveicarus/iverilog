/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_calltf.cc,v 1.10 2000/01/24 00:18:20 steve Exp $"
#endif

# include  "machine.h"
# include  "vvm_calltf.h"
# include  <vpi_user.h>
# include  "vpi_priv.h"
# include  <new>
# include  <iostream>
# include  <assert.h>
# include  <stdlib.h>
# include  <stdarg.h>
# include  <malloc.h>
# include  <stdio.h>
# include  <dlfcn.h>

# define MAX_PATHLEN 1024

static char*module_path = 0;

void vvm_set_module_path(const char*path)
{
      if (module_path) free(module_path);
      module_path = strdup(path);
}

/*
 * The load_vpi_module function attempts to locate and load the named
 * vpi module and call the included startup routines. This is invoked
 * by the generated C++ code to load all the modules that the
 * simulation requires.
 *
 * If there is a '/' character in the name, or there is no
 * VPI_MODULE_PATH, the the name is usd as is. No path is searched for
 * the module.
 *
 * If there is a VPI_MODULE_PATH and there is no '/' in the name, the
 * VPI_MODULE_PATH is taken as a ':' separated list of directory
 * names. Each directory is searched for a module with the right name
 * that will link in. The current working directory is not implicitly
 * tried. If you wish '.' be in th search path, include it.
 */
typedef void (*vlog_startup_routines_t)(void);

void vvm_load_vpi_module(const char*name)
{
      void*mod = 0;
      const char*path = getenv("VPI_MODULE_PATH");
      if (path == 0) path = module_path;

      if ((path == 0) || (strchr(name, '/'))) {
	  mod = dlopen(name, RTLD_NOW);
	  if (mod == 0) {
		cerr << name << ": " << dlerror() << endl;
		return;
	  }

      } else {
	    const char*cur = path;
	    const char*ep;
	    for (cur = path ; cur  ; cur = ep? ep+1 : 0) {
		  char dest[MAX_PATHLEN+1];

		  ep = strchr(cur, ':');
		  size_t n = ep? ep-cur : strlen(cur);
		  if ((n + strlen(name) + 2) > sizeof dest)
			continue;

		  strncpy(dest, cur, n);
		  dest[n] = '/';
		  dest[n+1] = 0;
		  strcat(dest, name);

		  mod = dlopen(dest, RTLD_NOW);
		  if (mod) break;
	    }
      }

      if (mod == 0) {
	    cerr << dlerror() << endl;
	    return;
      }

      void*table = dlsym(mod, LU "vlog_startup_routines" TU);
      vlog_startup_routines_t*routines = (vlog_startup_routines_t*)table;
      if (routines == 0) {
	    cerr << name << ": Unable to locate the vlog_startup_routines"
		 " table." << endl;
	    dlclose(mod);
	    return;
      }

      for (unsigned idx = 0 ;  routines[idx] ;  idx += 1)
	    (routines[idx])();
}



/*
 * $Log: vvm_calltf.cc,v $
 * Revision 1.10  2000/01/24 00:18:20  steve
 *  Handle systems that need underscores in symbols.
 *
 * Revision 1.9  1999/11/28 18:05:37  steve
 *  Set VPI_MODULE_PATH in the target code, if desired.
 *
 * Revision 1.8  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 * Revision 1.7  1999/10/10 14:57:38  steve
 *  Handle display of odd octal/hex widths (Eric Ardoom)
 *
 * Revision 1.6  1999/09/29 01:41:18  steve
 *  Support the $write system task, and have the
 *  vpi_scan function free iterators as needed.
 *
 * Revision 1.5  1999/09/13 03:08:10  steve
 *  fix vpiHexStrVal dumping of digits to strings.
 *
 * Revision 1.4  1999/08/19 02:51:11  steve
 *  Add vpi_sim_control
 *
 * Revision 1.3  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 * Revision 1.2  1999/05/31 15:46:36  steve
 *  Handle time in more places.
 *
 * Revision 1.1  1998/11/09 23:44:10  steve
 *  Add vvm library.
 *
 */

