#ifndef IVL_ivl_alloc_H
#define IVL_ivl_alloc_H
/*
 *  Copyright (C) 2010-2014  Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef __cplusplus
#  include <cstdlib>
#  include <cstdio>
#else
#  include <stdlib.h>
#  include <stdio.h>
#endif

#if defined(__GNUC__)
/*
 * Define a safer version of malloc().
 */

#define malloc(__ivl_size) \
({ \
	/* To be safe we only evaluate the argument once. */ \
      size_t __ivl_lsize = __ivl_size; \
      void *__ivl_rtn = malloc(__ivl_lsize); \
	/* If we run out of memory then exit with a message. */ \
      if ((__ivl_rtn == NULL) && (__ivl_lsize != 0)) { \
	    fprintf(stderr, "%s:%d: Error: malloc() ran out of memory.\n", \
	                    __FILE__, __LINE__); \
	    exit(1); \
      } \
      __ivl_rtn; \
})

/*
 * Define a safer version of realloc().
 */

#define realloc(__ivl_ptr, __ivl_size) \
({ \
	/* To be safe we only evaluate the arguments once. */ \
      void *__ivl_lptr = __ivl_ptr; \
      size_t __ivl_lsize = __ivl_size; \
      void *__ivl_rtn = realloc(__ivl_lptr, __ivl_lsize); \
	/* If we run out of memory then exit with a message. */ \
      if ((__ivl_rtn == NULL) && (__ivl_lsize != 0)) { \
	    fprintf(stderr, "%s:%d: Error: realloc() ran out of memory.\n", \
	                    __FILE__, __LINE__); \
	    free(__ivl_lptr); \
	    exit(1); \
      } \
      __ivl_rtn; \
})

/*
 * Define a safer version of calloc().
 */

#define calloc(__ivl_count, __ivl_size) \
({ \
	/* To be safe we only evaluate the arguments once. */ \
      size_t __ivl_lcount = __ivl_count; \
      size_t __ivl_lsize = __ivl_size; \
      void *__ivl_rtn = calloc(__ivl_lcount, __ivl_lsize); \
	/* If we run out of memory then exit with a message. */ \
      if ((__ivl_rtn == NULL) && (__ivl_lcount != 0) && (__ivl_lsize != 0)) { \
	    fprintf(stderr, "%s:%d: Error: calloc() ran out of memory.\n", \
	                    __FILE__, __LINE__); \
	    exit(1); \
      } \
      __ivl_rtn; \
})
#endif

#endif /* IVL_ivl_alloc_H */
