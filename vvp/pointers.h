#ifndef __pointers_H
#define __pointers_H
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
#if !defined(WINNT)
#ident "$Id: pointers.h,v 1.1 2001/03/11 00:29:39 steve Exp $"
#endif

/*
 * This header file describes the various "pointer" integral types
 * that are used in vvp, along with functions that can be used to
 * manipulate those pointers.
 */

# include  "config.h"

/*
 * The vvp_ipoint_t is a 32bit integer that encodes the address of a
 * functor input port.
 *
 * The typedef below gets the type properly declared as an integral
 * type that is big enough. The config.h header file has the defines
 * needed for the following pre-processor magic to work.
 */
# if SIZEOF_UNSIGNED >= 4
typedef unsigned vvp_ipoint_t;
#elif SIZEOF_UNSIGNED_LONG >= 4
typedef unsigned long vvp_ipoint_t;
#else
#error "I need an unsigned type that is 32 bits!"
#endif

/*
 * Given a functor generic address and a desired port, this function
 * makes a complete vvp_ipoint_t that points to the port of the given
 * functor. The The result points to the same functor as the input
 * pointer, but addresses the supplied port instead.
 */
inline vvp_ipoint_t ipoint_make(vvp_ipoint_t func, unsigned port)
{
      return (func & ~3) | (port & 3);
}

/*
 * This function returns the port index of a functor given a complete
 * vvp_ipoint_t pointer.
 */
inline unsigned ipoint_port(vvp_ipoint_t func)
{
      return func & 3;
}

typedef unsigned vvp_cpoint_t;

/*
 * $Log: pointers.h,v $
 * Revision 1.1  2001/03/11 00:29:39  steve
 *  Add the vvp engine to cvs.
 *
 */
#endif
