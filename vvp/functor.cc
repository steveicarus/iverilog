/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: functor.cc,v 1.48 2005/04/28 04:59:53 steve Exp $"
#endif

# include  "functor.h"
# include  "statistics.h"
# include  <assert.h>
# include  <string.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

# include  <stdio.h>

functor_s::~functor_s()
{
}

/*
 * $Log: functor.cc,v $
 * Revision 1.48  2005/04/28 04:59:53  steve
 *  Remove dead functor code.
 *
 * Revision 1.47  2005/04/03 06:13:34  steve
 *  Remove dead fvectors class.
 */

