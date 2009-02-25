#ifndef __sys_readmem_lex_H
#define __sys_readmem_lex_H
/*
 * Copyright (c) 1999-2009 Stephen Williams (steve@icarus.com)
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

# include  <stdio.h>
# include  "vpi_user.h"

# define MEM_ADDRESS 257
# define MEM_WORD    258
# define MEM_ERROR   259

extern char *readmem_error_token;

extern void sys_readmem_start_file(FILE*in, int bin_flag,
				   unsigned width, struct t_vpi_vecval*val);
extern int readmemlex();

extern void destroy_readmem_lexor();

#endif
