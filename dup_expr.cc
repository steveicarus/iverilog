/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: dup_expr.cc,v 1.3 2000/05/04 03:37:58 steve Exp $"
#endif

# include  "netlist.h"
# include  <cassert>

NetEScope* NetEScope::dup_expr() const
{
      assert(0);
      return 0;
}

NetESFunc* NetESFunc::dup_expr() const
{
      return new NetESFunc(name_, expr_width());
}

/*
 * $Log: dup_expr.cc,v $
 * Revision 1.3  2000/05/04 03:37:58  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.2  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1999/11/27 19:07:57  steve
 *  Support the creation of scopes.
 *
 */

