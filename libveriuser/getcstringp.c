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
#if !defined(WINNT)
#ident "$Id: getcstringp.c,v 1.1 2002/06/07 02:58:59 steve Exp $"
#endif

#include  <veriuser.h>
#include  <acc_user.h>

/*
 * tf_getinstance implemented using equvalent acc_ routing
 */
char *tf_getcstringp(int n)
{
      return acc_fetch_tfarg_str(n);
}

/*
 * $Log: getcstringp.c,v $
 * Revision 1.1  2002/06/07 02:58:59  steve
 *  Add a bunch of acc/tf functions. (mruff)
 *
 */
