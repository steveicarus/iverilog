#ifndef __named_H
#define __named_H
/*
 * Copyright (c) 2000-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: named.h,v 1.4 2004/02/20 06:22:56 steve Exp $"
#endif

# include  "StringHeap.h"

/*
 * There are lots of places where names are attached to objects. This
 * simple template expresses the lot.
 */

template <class T> struct named {
      perm_string name;
      T parm;
};

/*
 * $Log: named.h,v $
 * Revision 1.4  2004/02/20 06:22:56  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.3  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  2000/01/09 05:50:49  steve
 *  Support named parameter override lists.
 *
 */
#endif
