/*
 * Copyright (c) 2001-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_modules.cc,v 1.17 2003/10/08 23:09:09 steve Exp $"
#endif

# include  "config.h"
# include  "vpi_priv.h"
# include  "ivl_dlfcn.h"
# include  <stdio.h>
# include  <string.h>
# include  <sys/types.h>
# include  <sys/stat.h>

typedef void (*vlog_startup_routines_t)(void);


const char* vpip_module_path[64] = {
#ifdef MODULE_DIR1
      MODULE_DIR1,
#endif
#ifdef MODULE_DIR2
      MODULE_DIR2,
#endif
      0
};

unsigned vpip_module_path_cnt = 0
#ifdef MODULE_DIR1
         + 1
#endif
#ifdef MODULE_DIR2
         + 1
#endif
;

void vpip_load_module(const char*name)
{
      struct stat sb;
      int rc;
      bool export_flag = false;
      char buf[4096];

#ifdef __MINGW32__
      const char sep = '\\';
#else
      const char sep = '/';
#endif

      ivl_dll_t dll = 0;
      buf[0] = 0;                     /* terminate the string */
      if (strchr(name, sep)) {
	      /* If the name has at least one directory character in
		 it, then assume it is a complete name, maybe including any
		 possible .vpi suffix. */
	    export_flag = false;
	    rc = stat(name, &sb);

	    if (rc != 0) {            /* did we find a file? */
	          /* no, try with a .vpi suffix too */
		  export_flag = false;
		  sprintf(buf, "%s.vpi", name);
		  rc = stat(buf, &sb);

		    /* Tray alwo with the .vpl suffix. */
		  if (rc != 0) {
			export_flag = true;
			sprintf(buf, "%s.vpl", name);
			rc = stat(buf, &sb);
		  }

		  if (rc != 0) {
			fprintf(stderr, "%s: Unable to find module file `%s' "
				"or `%s.vpi'.\n", name,name,buf);
			return;
		  }
	    } else {
	      strcpy(buf,name);   /* yes copy the name into the buffer */
	    }

      } else {
	    rc = -1;
	    for (unsigned idx = 0
		       ; (rc != 0) && (idx < vpip_module_path_cnt)
		       ;  idx += 1) {
		  export_flag = false;
		  sprintf(buf, "%s%c%s.vpi", vpip_module_path[idx], sep, name);
		  rc = stat(buf,&sb);

		  if (rc != 0) {
			export_flag = true;
			sprintf(buf, "%s%c%s.vpl",
				vpip_module_path[idx], sep, name);
			rc = stat(buf,&sb);
		  }
	    }

	    if (rc != 0) {
		  fprintf(stderr, "%s: Unable to find a "
			  "`%s.vpi' module on the search path.\n",
			  name, name); 
		  return;
	    }

      }

      /* must have found some file that could possibly be a vpi module 
       * try to open it as a shared object.
       */
      dll = ivl_dlopen(buf, export_flag);
      if(dll==0) {
	/* hmm, this failed, let the user know what has really gone wrong */
	fprintf(stderr,"%s:`%s' failed to open using dlopen() because:\n"
		"    %s.\n",name,buf,dlerror());

	return;
      }


      void*table = ivl_dlsym(dll, LU "vlog_startup_routines" TU);
      if (table == 0) {
	    fprintf(stderr, "%s: no vlog_startup_routines\n", name);
	    ivl_dlclose(dll);
	    return;
      }

      vpi_mode_flag = VPI_MODE_REGISTER;
      vlog_startup_routines_t*routines = (vlog_startup_routines_t*)table;
      for (unsigned tmp = 0 ;  routines[tmp] ;  tmp += 1)
	    (routines[tmp])();
      vpi_mode_flag = VPI_MODE_NONE;
}

/*
 * $Log: vpi_modules.cc,v $
 * Revision 1.17  2003/10/08 23:09:09  steve
 *  Completely support vvp32 when enabled.
 *
 * Revision 1.16  2003/10/02 21:30:40  steve
 *  Configure control for the vpi subdirectory.
 *
 * Revision 1.15  2003/02/16 02:21:20  steve
 *  Support .vpl files as loadable LIBRARIES.
 *
 * Revision 1.14  2003/02/09 23:33:26  steve
 *  Spelling fixes.
 *
 * Revision 1.13  2003/01/10 03:06:32  steve
 *  Remove vpithunk, and move libvpi to vvp directory.
 *
 * Revision 1.12  2002/08/12 01:35:09  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.11  2002/05/18 02:34:11  steve
 *  Add vpi support for named events.
 *
 *  Add vpi_mode_flag to track the mode of the
 *  vpi engine. This is for error checking.
 *
 * Revision 1.10  2002/03/05 05:31:52  steve
 *  Better linker error messages.
 *
 * Revision 1.9  2001/10/14 18:42:46  steve
 *  Try appending .vpi to module names with directories.
 *
 * Revision 1.8  2001/07/30 02:44:05  steve
 *  Cleanup defines and types for mingw compile.
 *
 * Revision 1.7  2001/07/28 03:29:42  steve
 *  If module name has a /, skip the path search.
 *
 * Revision 1.6  2001/07/26 03:13:51  steve
 *  Make the -M flag add module search paths.
 */

