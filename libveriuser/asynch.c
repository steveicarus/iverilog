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
#ident "$Id: asynch.c,v 1.3 2003/04/23 15:01:29 steve Exp $"
#endif

# include  <veriuser.h>

/* Enables async misctf callbacks */
int async_misctf_enable = 0;

/*
 * Implement misctf async enable
 */
int tf_asynchon(void)
{
      async_misctf_enable = 1;
      return 0;
}

int tf_asynchoff(void)
{
      async_misctf_enable = 0;
      return 0;
}

/*
 * $Log: asynch.c,v $
 * Revision 1.3  2003/04/23 15:01:29  steve
 *  Add tf_synchronize and tf_multiply_long.
 *
 * Revision 1.2  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/06/04 01:40:03  steve
 *  Add asynchon and asynchoff
 *
 */
