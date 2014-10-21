/*
 * Copyright (c) 2003-2010 Stephen Williams (steve@icarus.com)
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

# include  <vpi_user.h>
# include  <veriuser.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
# include  "config.h"
# include  "ivl_dlfcn.h"
# include  "ivl_alloc.h"

typedef void* (*funcvp)(void);

static void thunker_register(void)
{
      struct t_vpi_vlog_info vlog_info;
      void*mod;
      void*boot;
      struct t_tfcell*tf;
      int idx;

      vpi_get_vlog_info(&vlog_info);

      for (idx = 0 ;  idx < vlog_info.argc ;  idx += 1) {
	    char*module, *cp, *bp;
	    if (strncmp("-cadpli=", vlog_info.argv[idx], 8) != 0)
		  continue;

	    cp = vlog_info.argv[idx] + 8;
	    assert(cp);

	    bp = strchr(cp, ':');
	    assert(bp);

	    module = malloc(bp-cp+1);
	    strncpy(module, cp, bp-cp);
	    module[bp-cp] = 0;

	    mod = ivl_dlopen(module);
	    if (mod == 0) {
		  vpi_printf("%s link: %s\n", vlog_info.argv[idx], dlerror());
		  free(module);
		  continue;
	    }

	    bp += 1;
	    boot = ivl_dlsym(mod, bp);
	    if (boot == 0) {
		  vpi_printf("%s: Symbol %s not found.\n",
			     vlog_info.argv[idx], bp);
		  free(module);
		  continue;
	    }

	    free(module);
	    assert(boot);

	    tf = (*((funcvp)boot))();
	    assert(tf);

	    veriusertfs_register_table(tf);
      }
}

void (*vlog_startup_routines[])(void) = {
      thunker_register,
      0
};
