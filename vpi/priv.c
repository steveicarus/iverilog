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
#ident "$Id: priv.c,v 1.2 2003/10/06 21:26:27 steve Exp $"
#endif

# include  "sys_priv.h"

PLI_UINT64 timerec_to_time64(const struct t_vpi_time*time)
{
      PLI_UINT64 tmp;

      tmp = time->high;
      tmp <<= 32;
      tmp |= (PLI_UINT64) time->low;
      return tmp;
}

/*
 * $Log: priv.c,v $
 * Revision 1.2  2003/10/06 21:26:27  steve
 *  Include sys_priv.h instead of priv.h
 *
 * Revision 1.1  2003/10/02 21:16:11  steve
 *  Include timerec_to_time64 implementation.
 *
 */

