#ifndef PLI_TYPES
#define PLI_TYPES
/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: _pli_types.h,v 1.2 2003/05/26 04:39:16 steve Exp $"
#endif

typedef signed int     PLI_INT32;
typedef unsigned int   PLI_UINT32;
typedef signed short   PLI_INT16;
typedef unsigned short PLI_UINT16;
typedef signed char    PLI_BYTE8;
typedef unsigned char  PLI_UBYTE8;

/*
 * $Log: _pli_types.h,v $
 * Revision 1.2  2003/05/26 04:39:16  steve
 *  Typo type name.
 *
 * Revision 1.1  2003/02/17 06:39:47  steve
 *  Add at least minimal implementations for several
 *  acc_ functions. Add support for standard ACC
 *  string handling.
 *
 *  Add the _pli_types.h header file to carry the
 *  IEEE1364-2001 standard PLI type declarations.
 *
 */
#endif
