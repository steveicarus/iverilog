/* vi:sw=6
 * Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
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
#ident "$Id: a_fetch_fullname.c,v 1.5 2003/05/29 02:21:45 steve Exp $"
#endif

#include  <vpi_user.h>
#include  <acc_user.h>
#include  "priv.h"

/*
 * acc_fetch_fullname implemented using VPI interface
 */
char *acc_fetch_fullname(handle object)
{
      return __acc_newstring(vpi_get_str(vpiFullName, object));
}

char* acc_fetch_name(handle object)
{
      return __acc_newstring(vpi_get_str(vpiName, object));
}

char* acc_fetch_defname(handle object)
{
      return __acc_newstring(vpi_get_str(vpiDefName, object));
}

/*
 * $Log: a_fetch_fullname.c,v $
 * Revision 1.5  2003/05/29 02:21:45  steve
 *  Implement acc_fetch_defname and its infrastructure in vvp.
 *
 * Revision 1.4  2003/03/13 05:07:46  steve
 *  Declaration warnings.
 *
 * Revision 1.3  2003/02/17 06:39:47  steve
 *  Add at least minimal implementations for several
 *  acc_ functions. Add support for standard ACC
 *  string handling.
 *
 *  Add the _pli_types.h header file to carry the
 *  IEEE1364-2001 standard PLI type declarations.
 *
 * Revision 1.2  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/06/07 02:58:58  steve
 *  Add a bunch of acc/tf functions. (mruff)
 *
 */
