#ifndef __veriuser_H
#define __veriuser_H
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
#if !defined(WINNT)
#ident "$Id: veriuser.h,v 1.1 1999/08/15 01:23:56 steve Exp $"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This header file only exists as a stub, and to carry the function
 * declaration for the tf_dofinish() function. I plan on removing both
 * as soon as a VPI way of doing this appears.
 */

extern void tf_dofinish();

#ifdef __cplusplus
}
#endif

/*
 * $Log: veriuser.h,v $
 * Revision 1.1  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 */
#endif
