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
#ident "$Id: a_fetch_time.c,v 1.1 2003/03/13 04:35:09 steve Exp $"
#endif


#include  <vpi_user.h>
#include  <acc_user.h>
#include  "priv.h"

void acc_fetch_timescale_info(handle obj, p_timescale_info info)
{
      info->precision = vpi_get(vpiTimePrecision, 0);
      info->unit = vpi_get(vpiTimeUnit, obj);
}

/*
 * $Log: a_fetch_time.c,v $
 * Revision 1.1  2003/03/13 04:35:09  steve
 *  Add a bunch of new acc_ and tf_ functions.
 *
 */

