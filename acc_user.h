#ifndef __acc_user_H
#define __acc_user_H
/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: acc_user.h,v 1.2 2002/05/30 02:06:05 steve Exp $"
#endif

/*
 * This header file contains the definitions and declarations needed
 * by an Icarus Verilog user using acc_ routines.
 *
 * NOTE: Icarus Verilog does not support acc_ routines. This is just a
 * stub. The functions that are implemented here are actually
 * implemented using VPI routines.
 */

#ifdef __cplusplus
# define EXTERN_C_START extern "C" {
# define EXTERN_C_END }
#else
# define EXTERN_C_START
# define EXTERN_C_END
# define bool int
#endif

EXTERN_C_START

extern int acc_error_flag;

extern int acc_initialize(void);
extern void acc_close(void);
extern char*acc_product_version(void);

EXTERN_C_END

/*
 * $Log: acc_user.h,v $
 * Revision 1.2  2002/05/30 02:06:05  steve
 *  Implement acc_product_version.
 *
 * Revision 1.1  2002/05/23 03:46:42  steve
 *  Add the acc_user.h header file.
 *
 */

#endif
