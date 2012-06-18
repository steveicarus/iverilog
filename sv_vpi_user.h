#ifndef __sv_vpi_user_H
#define __sv_vpi_user_H
/*
 * Copyright (c) 2010-2011 Stephen Williams (steve@icarus.com)
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

# include  "vpi_user.h"

#if defined(__MINGW32__) || defined (__CYGWIN32__)
#  define DLLEXPORT __declspec(dllexport)
#else
#  define DLLEXPORT
#endif

#ifdef __cplusplus
# define EXTERN_C_START extern "C" {
# define EXTERN_C_END }
#else
# define EXTERN_C_START
# define EXTERN_C_END
#endif

#ifndef __GNUC__
# undef  __attribute__
# define __attribute__(x)
#endif

EXTERN_C_START

/********* OBJECT TYPES ***********/
#define vpiLongIntVar       610
#define vpiShortIntVar      611
#define vpiIntVar           612
#define vpiByteVar          614
#define vpiLogicVar         vpiReg
#define vpiStringVar        616
#define vpiBitVar           620

/********* TYPESPECS *************/
#define vpiEnumTypespec     633
#define vpiEnumConst        634

/********* One-to-One ***********/
#define vpiBaseTypespec     703

/********* Many-to-One ***********/
#define vpiMember           742

EXTERN_C_END

#endif
