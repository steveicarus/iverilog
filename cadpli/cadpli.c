/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: cadpli.c,v 1.5 2003/08/26 16:26:02 steve Exp $"
#endif

# include  <vpi_user.h>
# include  <veriuser.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <string.h>
# include  <assert.h>
# include  "ivl_dlfcn.h"

typedef void* (*funcvp)(void);

static void thunker_register(void)
{
      struct t_vpi_vlog_info vlog_info;
      void*mod;
      void*boot;
      void*tf;
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

void (*vlog_startup_routines[])() = {
      thunker_register,
      0
};


/*
 * $Log: cadpli.c,v $
 * Revision 1.5  2003/08/26 16:26:02  steve
 *  ifdef idents correctly.
 *
 * Revision 1.4  2003/04/30 01:28:06  steve
 *  Remove veriusertfs stuf.
 *
 * Revision 1.3  2003/02/22 04:04:38  steve
 *  Only include malloc.h if it is present.
 *
 * Revision 1.2  2003/02/17 00:01:25  steve
 *  Use a variant of ivl_dlfcn to do dynamic loading
 *  from within the cadpli module.
 *
 *  Change the +cadpli flag to -cadpli, to keep the
 *  plusargs namespace clear.
 *
 * Revision 1.1  2003/02/16 02:23:54  steve
 *  Add the cadpli interface module.
 *
 */

