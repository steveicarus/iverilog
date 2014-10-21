#ifndef IVL_ivl_dlfcn_H
#define IVL_ivl_dlfcn_H
/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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

#if defined(__MINGW32__)
# include <windows.h>
# include <cstdio>
typedef void * ivl_dll_t;
#elif defined(HAVE_DLFCN_H)
# include  <dlfcn.h>
typedef void* ivl_dll_t;
#elif defined(HAVE_DL_H)
# include  <dl.h>
typedef shl_t ivl_dll_t;
#endif

#if defined(__MINGW32__)
inline ivl_dll_t ivl_dlopen(const char *name, bool)
{
      static char full_name[4096];
      unsigned long length = GetFullPathName(name, sizeof(full_name),
                                             full_name, NULL);
      if ((length == 0) || (length > sizeof(full_name)))
            return 0;

      return (void *)LoadLibrary(full_name);
}

inline void *ivl_dlsym(ivl_dll_t dll, const char *nm)
{ return (void *)GetProcAddress((HINSTANCE)dll,nm);}

inline void ivl_dlclose(ivl_dll_t dll)
{ (void)FreeLibrary((HINSTANCE)dll);}

inline const char *dlerror(void)
{
  static char msg[256];
  unsigned long err = GetLastError();
  FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &msg,
		sizeof(msg) - 1,
		NULL
		);
  return msg;
}

#elif defined(HAVE_DLFCN_H)
inline ivl_dll_t ivl_dlopen(const char*name, bool global_flag)
{ return dlopen(name,RTLD_LAZY|(global_flag?RTLD_GLOBAL:0)); }

inline void* ivl_dlsym(ivl_dll_t dll, const char*nm)
{
      void*sym = dlsym(dll, nm);
	/* Not found? try without the leading _ */
      if (sym == 0 && nm[0] == '_')
	    sym = dlsym(dll, nm+1);
      return sym;
}

inline void ivl_dlclose(ivl_dll_t dll)
{ dlclose(dll); }

#elif defined(HAVE_DL_H)
inline ivl_dll_t ivl_dlopen(const char*name)
{ return shl_load(name, BIND_IMMEDIATE, 0); }

inline void* ivl_dlsym(ivl_dll_t dll, const char*nm)
{
      void*sym;
      int rc = shl_findsym(&dll, nm, TYPE_PROCEDURE, &sym);
      return (rc == 0) ? sym : 0;
}

inline void ivl_dlclose(ivl_dll_t dll)
{ shl_unload(dll); }

inline const char*dlerror(void)
{ return strerror( errno ); }
#endif

#endif /* IVL_ivl_dlfcn_H */
