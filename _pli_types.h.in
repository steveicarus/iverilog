#ifndef PLI_TYPES_H
#define PLI_TYPES_H
/*
 * Copyright (c) 2003-2014 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# undef HAVE_INTTYPES_H

#ifdef HAVE_INTTYPES_H

/*
 * If the host environment has the stdint.h header file,
 * then use that to size our PLI types.
 */
#ifndef __STDC_FORMAT_MACROS
# define __STDC_FORMAT_MACROS
#endif

# include  <inttypes.h>
typedef uint64_t PLI_UINT64;
typedef int64_t  PLI_INT64;
typedef uint32_t PLI_UINT32;
typedef int32_t  PLI_INT32;

typedef signed short   PLI_INT16;
typedef unsigned short PLI_UINT16;
typedef char           PLI_BYTE8;
typedef unsigned char  PLI_UBYTE8;

# define PLI_UINT64_FMT PRIu64

#else

/*
 * If we do not have the c99 stdint.h header file, then use
 * configure detection to guess the pli types ourselves.
 */

# define SIZEOF_UNSIGNED_LONG_LONG 8
# define SIZEOF_UNSIGNED_LONG 8
# define SIZEOF_UNSIGNED 4

#if SIZEOF_UNSIGNED >= 8
typedef unsigned PLI_UINT64;
typedef int      PLI_INT64;
# define PLI_UINT64_FMT "u"
#else
# if SIZEOF_UNSIGNED_LONG >= 8
typedef unsigned long PLI_UINT64;
typedef          long PLI_INT64;
#  define PLI_UINT64_FMT "lu"
# else
#  if SIZEOF_UNSIGNED_LONG_LONG > SIZEOF_UNSIGNED_LONG
typedef unsigned long long PLI_UINT64;
typedef          long long PLI_INT64;
#   define PLI_UINT64_FMT "llu"
#  else
typedef unsigned long PLI_UINT64;
typedef          long PLI_INT64;
#   define PLI_UINT64_FMT "lu"
#  endif
# endif
#endif

typedef signed int     PLI_INT32;
typedef unsigned int   PLI_UINT32;
typedef signed short   PLI_INT16;
typedef unsigned short PLI_UINT16;
typedef char           PLI_BYTE8;
typedef unsigned char  PLI_UBYTE8;
#endif

#endif /* PLI_TYPES_H */
