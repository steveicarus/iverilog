#ifndef __vvm_machine_H
#define __vvm_machine_H
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: machine.h,v 1.2 2000/02/23 02:56:56 steve Exp $"
#endif

#ifdef NEED_LU
#define LU "_"
#else
#define LU ""
#endif

#ifdef NEED_TU
#define TU "_"
#else
#define TU ""
#endif

/*
 * $Log: machine.h,v $
 * Revision 1.2  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  2000/01/24 00:18:20  steve
 *  Handle systems that need underscores in symbols.
 *
 */
#endif
