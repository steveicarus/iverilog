/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: null.c,v 1.3 2001/05/20 15:09:40 steve Exp $"
#endif

/*
 * This is a null target module. It does nothing.
 */

# include  "ivl_target.h"


int target_design(ivl_design_t des)
{
      return 0;
}

#if defined(__MINGW32__) || defined (__CYGWIN32__)
#include <cygwin/cygwin_dll.h>
DECLARE_CYGWIN_DLL(DllMain);
#endif

/*
 * $Log: null.c,v $
 * Revision 1.3  2001/05/20 15:09:40  steve
 *  Mingw32 support (Venkat Iyer)
 *
 * Revision 1.2  2001/02/07 22:21:59  steve
 *  ivl_target header search path fixes.
 *
 * Revision 1.1  2000/12/02 04:50:32  steve
 *  Make the null target into a loadable target.
 *
 */

