/*
 * Copyright (c) 2001-2022 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "vpi_priv.h"
# include  "ivl_dlfcn.h"
# include  "vvp_cleanup.h"
# include  <cstdio>
# include  <cstring>
# include  <sys/types.h>
# include  <sys/stat.h>
# include  "ivl_alloc.h"

static ivl_dll_t*dll_list = 0;
static unsigned dll_list_cnt = 0;

#if defined(__MINGW32__) || defined (__CYGWIN__)
typedef PLI_UINT32 (*vpip_set_callback_t)(vpip_routines_s*, PLI_UINT32);
#endif
typedef void (*vlog_startup_routines_t)(void);

# define VPIP_MODULE_PATH_MAX 64

static const char* vpip_module_path[VPIP_MODULE_PATH_MAX] = {0};

static unsigned vpip_module_path_cnt = 0;

static bool disable_default_paths = false;

void vpip_clear_module_paths()
{
      vpip_module_path_cnt = 0;
      vpip_module_path[0] = 0;
      disable_default_paths = true;
}

void vpip_add_module_path(const char*path)
{
      if (vpip_module_path_cnt >= VPIP_MODULE_PATH_MAX) {
	    fprintf(stderr, "Too many module paths specified\n");
	    exit(1);
      }
      vpip_module_path[vpip_module_path_cnt++] = path;
}

void vpip_add_env_and_default_module_paths()
{
      if (disable_default_paths)
	    return;

#ifdef __MINGW32__
      const char path_sep = ';';
#else
      const char path_sep = ':';
#endif

      if (char *var = ::getenv("IVERILOG_VPI_MODULE_PATH")) {
            char *ptr = var;
            char *end = var+strlen(var);
            int len = 0;
            while (ptr <= end) {
                  if (*ptr == 0 || *ptr == path_sep) {
                        if (len > 0) {
                              vpip_add_module_path(strndup(var, len));
                        }
                        len = 0;
                        var = ptr+1;
                  } else {
                        len++;
                  }
                  ptr++;
            }
      }

#ifdef __MINGW32__
	/* Calculate the module path from the path to the command.
	   This is necessary because of the installation process on
	   Windows. Mostly, it is those darn drive letters, but oh
	   well. We know the command path is formed like this:

		D:\iverilog\bin\iverilog.exe

	   The IVL_ROOT in a Windows installation is the path:

		D:\iverilog\lib\ivl$(suffix)

	   so we chop the file name and the last directory by
	   turning the last two \ characters to null. Then we append
	   the lib\ivl$(suffix) to finish. */
      char *s;
      char basepath[4096], tmp[4096];
      GetModuleFileName(NULL, tmp, sizeof tmp);
	/* Convert to a short name to remove any embedded spaces. */
      GetShortPathName(tmp, basepath, sizeof basepath);
      s = strrchr(basepath, '\\');
      if (s) *s = 0;
      else {
	    fprintf(stderr, "%s: Missing first \\ in exe path!\n", tmp);
	    exit(1);
      }
      s = strrchr(basepath, '\\');
      if (s) *s = 0;
      else {
	    fprintf(stderr, "%s: Missing second \\ in exe path!\n", tmp);
	    exit(1);
      }
      strcat(s, "\\lib\\ivl" IVL_SUFFIX);
      vpip_add_module_path(strdup(basepath));
#else
#ifdef MODULE_DIR1
      vpip_add_module_path(MODULE_DIR1);
#endif
#endif
#ifdef MODULE_DIR2
      vpip_add_module_path(MODULE_DIR2);
#endif
}

void load_module_delete(void)
{
      for (unsigned idx = 0; idx < dll_list_cnt; idx += 1) {
	    ivl_dlclose(dll_list[idx]);
      }
      free(dll_list);
      dll_list = 0;
      dll_list_cnt = 0;
}

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
#ifdef __MINGW32__
      if (strchr(name, '\\') || strchr(name, '/')) {
#else
      if (strchr(name, sep)) {
#endif
	      /* If the name has at least one directory character in
		 it, then assume it is a complete name, maybe including any
		 possible .vpi suffix. */
	    export_flag = false;
	    rc = stat(name, &sb);

	    if (rc != 0) {            /* did we find a file? */
	          /* no, try with a .vpi suffix too */
		  export_flag = false;
		  snprintf(buf, sizeof(buf), "%s.vpi", name);
		  rc = stat(buf, &sb);

		    /* Try also with the .vpl suffix. */
		  if (rc != 0) {
			export_flag = true;
			snprintf(buf, sizeof(buf), "%s.vpl", name);
			rc = stat(buf, &sb);
		  }

		  if (rc != 0) {
			fprintf(stderr, "%s: Unable to find module file `%s' "
				"or `%s.vpi'.\n", name, name, name);
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
		  snprintf(buf, sizeof(buf), "%s%c%s.vpi",
			   vpip_module_path[idx], sep, name);
		  rc = stat(buf,&sb);

		  if (rc != 0) {
			export_flag = true;
			snprintf(buf, sizeof(buf), "%s%c%s.vpl",
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

#if defined(__MINGW32__) || defined (__CYGWIN__)
      void*function = ivl_dlsym(dll, "vpip_set_callback");
      if (function) {
            vpip_set_callback_t set_callback = (vpip_set_callback_t)function;
            if (!set_callback(&vpi_routines, vpip_routines_version)) {
	          fprintf(stderr, "Failed to link VPI module %s. Try rebuilding it with iverilog-vpi.\n", name);
	          ivl_dlclose(dll);
	          return;
            }
      }
#endif

      void*table = ivl_dlsym(dll, LU "vlog_startup_routines" TU);
      if (table == 0) {
	    fprintf(stderr, "%s: no vlog_startup_routines\n", name);
	    ivl_dlclose(dll);
	    return;
      }

	/* Add the dll to the list so it can be closed when we are done. */
      dll_list_cnt += 1;
      dll_list = (ivl_dll_t*)realloc(dll_list, dll_list_cnt*sizeof(ivl_dll_t));
      dll_list[dll_list_cnt-1] = dll;

      vpi_mode_flag = VPI_MODE_REGISTER;
      vlog_startup_routines_t*routines = (vlog_startup_routines_t*)table;
      for (unsigned tmp = 0 ;  routines[tmp] ;  tmp += 1)
	    (routines[tmp])();
      vpi_mode_flag = VPI_MODE_NONE;
}
