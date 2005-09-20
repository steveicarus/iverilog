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
#ident "$Id: getinstance.c,v 1.5 2005/09/20 18:34:01 steve Exp $"
#endif

#include  <veriuser.h>
#include  <vpi_user.h>

/*
 * tf_getinstance implemented using VPI interface
 */
PLI_BYTE8* tf_getinstance(void)
{
      return (PLI_BYTE8 *)vpi_handle(vpiSysTfCall, 0 /* NULL */);
}

/*
 * $Log: getinstance.c,v $
 * Revision 1.5  2005/09/20 18:34:01  steve
 *  Clean up compiler warnings.
 *
 * Revision 1.4  2003/05/18 00:16:35  steve
 *  Add PLI_TRACE tracing of PLI1 modules.
 *
 *  Add tf_isetdelay and friends, and add
 *  callback return values for acc_vcl support.
 *
 * Revision 1.3  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2002/06/03 21:52:59  steve
 *  Fix return type of tf_getinstance.
 *
 * Revision 1.1  2002/06/02 18:54:59  steve
 *  Add tf_getinstance function.
 *
 */
